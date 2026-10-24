#include "../global.h"
#include "bake_normal_core.h"
#include <iron_thread.h>
#ifdef WITH_PLUGINS
#include "../../plugins/io_fbx/ufbx/ufbx.h"
#endif

typedef struct normal_bake_output {
	bn_result result;
	bool      applied;
} normal_bake_output;

typedef struct normal_bake_session {
	int    target_uid, high_uid, uv, status, high_count, low_count, imported_count;
	int    phase, source_count, source_index;
	int    map_mask, active_type, map_count, map_index, current_map;
	bool   building;
	int    result_version, analysis_version;
	char **source_names;
	int   *source_nodes;
#ifdef WITH_PLUGINS
	ufbx_scene *source_scene;
#endif
	bool               inspecting, analyzing, result_stale;
	uint8_t           *analysis_risk;
	bn_face           *high, *imported, *work_high;
	bn_low            *low;
	bn_options         options[4];
	bn_options         settings[4];
	bn_options         job_options;
	normal_bake_output outputs[4];
	char              *path;
	char              *blender, *blender_object, *blender_script;
	bool               blender_selected;
	char               error[256];
	mat4_t             import_transform, work_transform;
	vec4_t             offset, rotation;
	float              scale;
	iron_thread_t      thread;
	int                cancel, done, progress;
	iron_mutex_t       mutex;
	bool               running, loading, closing, preview_dirty;
	gpu_texture_t     *image, *diagnostic;
} normal_bake_session;

static normal_bake_session normal_bake = {.target_uid = -1, .high_uid = -1, .scale = 1.0f, .source_index = -1, .map_mask = 1};
static mat4_t              normal_bake_adjustment();
static normal_bake_output *normal_bake_current() {
	return &normal_bake.outputs[normal_bake.active_type];
}

static void normal_bake_clear_source() {
	for (int i = 0; i < normal_bake.source_count; i++)
		free(normal_bake.source_names[i]);
	free(normal_bake.source_names);
	free(normal_bake.source_nodes);
	normal_bake.source_names = NULL;
	normal_bake.source_nodes = NULL;
	normal_bake.source_count = 0;
	normal_bake.source_index = -1;
#ifdef WITH_PLUGINS
	if (normal_bake.source_scene)
		ufbx_free_scene(normal_bake.source_scene);
	normal_bake.source_scene = NULL;
#endif
}

static int normal_bake_read(int *value) {
	iron_mutex_lock(&normal_bake.mutex);
	int result = *value;
	iron_mutex_unlock(&normal_bake.mutex);
	return result;
}
static void normal_bake_write(int *value, int next) {
	iron_mutex_lock(&normal_bake.mutex);
	*value = next;
	iron_mutex_unlock(&normal_bake.mutex);
}

static mesh_object_t *normal_bake_object(int uid) {
	for (int i = 0; i < g_project->_->paint_objects->length; i++) {
		mesh_object_t *o = g_project->_->paint_objects->buffer[i];
		if (o->base->uid == uid && o->data->index_array->length > 0)
			return o;
	}
	return NULL;
}

static bn_v3 normal_bake_point(bn_v3 p, mat4_t m, float w) {
	vec4_t v = vec4_apply_mat4((vec4_t){p.x, p.y, p.z, w}, m);
	return (bn_v3){v.x, v.y, v.z};
}

static bn_low *normal_bake_snapshot(mesh_object_t *o, int uv, int *count, bool require_uv) {
	*count = 0;
	if (!o || !o->data || o->data->index_array->length == 0)
		return NULL;
	mesh_data_t    *m   = o->data;
	vertex_array_t *pos = mesh_data_get_vertex_array(m, "pos"), *nor = mesh_data_get_vertex_array(m, "nor");
	vertex_array_t *tex = mesh_data_get_vertex_array(m, uv == 1 ? "tex1" : "tex");
	if (!pos || !nor || (require_uv && !tex) || m->index_array->length % 3)
		return NULL;
	int length = m->index_array->length / 3;
	if (length > 10000000)
		return NULL;
	bn_low *out = calloc(length, sizeof(bn_low));
	if (!out)
		return NULL;
	transform_build_matrix(o->base->transform);
	mat4_t world   = o->base->transform->world;
	mat4_t normals = mat4_transpose(mat4_inv(world));
	for (int t = 0; t < length; t++)
		for (int k = 0; k < 3; k++) {
			int v = m->index_array->buffer[t * 3 + k];
			if (v < 0 || v >= pos->values->length / 4 || v >= nor->values->length / 2 || (tex && v >= tex->values->length / 2)) {
				free(out);
				return NULL;
			}
			i16  *p = pos->values->buffer + v * 4, *n = nor->values->buffer + v * 2;
			bn_v3 point      = {p[0] * m->scale_pos / 32767.0f, p[1] * m->scale_pos / 32767.0f, p[2] * m->scale_pos / 32767.0f};
			out[t].face.p[k] = normal_bake_point(point, world, 1);
			out[t].face.n[k] = bn_norm(normal_bake_point((bn_v3){n[0] / 32767.0f, n[1] / 32767.0f, p[3] / 32767.0f}, normals, 0));
			if (tex) {
				out[t].uv[k][0] = tex->values->buffer[v * 2] / 32767.0f * m->scale_tex;
				out[t].uv[k][1] = tex->values->buffer[v * 2 + 1] / 32767.0f * m->scale_tex;
			}
			if (!bn_finite(out[t].face.p[k]) || !bn_finite(out[t].face.n[k]) || !isfinite(out[t].uv[k][0]) || !isfinite(out[t].uv[k][1])) {
				free(out);
				return NULL;
			}
		}
	*count = length;
	return out;
}

static void normal_bake_clear_result() {
	for (int i = 0; i < 4; i++) {
		bn_result_free(&normal_bake.outputs[i].result);
		normal_bake.outputs[i].applied = false;
	}
	free(normal_bake.analysis_risk);
	normal_bake.analysis_risk = NULL;
	normal_bake.result_stale  = false;
	if (normal_bake.image)
		gpu_delete_texture(normal_bake.image);
	if (normal_bake.diagnostic)
		gpu_delete_texture(normal_bake.diagnostic);
	normal_bake.image = normal_bake.diagnostic = NULL;
	normal_bake_current()->applied             = false;
	if (!normal_bake.running)
		normal_bake.status = 0;
}

static void normal_bake_release() {
	normal_bake_clear_result();
	normal_bake_clear_source();
	free(normal_bake.high);
	free(normal_bake.imported);
	free(normal_bake.work_high);
	free(normal_bake.low);
	free(normal_bake.path);
	free(normal_bake.blender);
	free(normal_bake.blender_object);
	free(normal_bake.blender_script);
	normal_bake.blender = normal_bake.blender_object = normal_bake.blender_script = NULL;
	normal_bake.high = normal_bake.imported = normal_bake.work_high = NULL;
	normal_bake.low                                                 = NULL;
	normal_bake.path                                                = NULL;
	normal_bake.high_count = normal_bake.low_count = normal_bake.imported_count = 0;
	normal_bake.status                                                          = 0;
	normal_bake.target_uid = normal_bake.high_uid = -1;
	normal_bake.scale                             = 1;
	normal_bake.offset = normal_bake.rotation = (vec4_t){0};
	normal_bake.closing                       = false;
	normal_bake.preview_dirty                 = true;
	normal_bake.error[0]                      = 0;
	normal_bake.map_mask                      = 1;
	normal_bake.active_type                   = BN_NORMAL;
	memset(normal_bake.settings, 0, sizeof(normal_bake.settings));
	normal_bake.analyzing = false;
}

static bool normal_bake_progress(void *user, int progress) {
	normal_bake_session *s       = user;
	int                  overall = s->loading ? progress : 10 + progress * 90 / 100;
	if (!s->loading && !s->analyzing)
		overall = s->building ? 10 + progress * 10 / 15 : 20 + (s->map_index * 80 + (int)fmaxf(0, progress - 20)) / s->map_count;
	normal_bake_write(&s->progress, overall);
	if (!s->loading)
		normal_bake_write(&s->phase, progress < 20 ? 4 : s->analyzing ? 7 : progress < 90 ? 5 : 6);
	return !normal_bake_read(&s->cancel);
}

#include "bake_normal_blend.h"

#ifdef WITH_PLUGINS
static ufbx_progress_result normal_bake_import_progress(void *user, const ufbx_progress *p) {
	return normal_bake_progress(user, p->bytes_total ? (int)(p->bytes_read * 65 / p->bytes_total) : 0) ? UFBX_PROGRESS_CONTINUE : UFBX_PROGRESS_CANCEL;
}

static void normal_bake_import_worker(void *arg) {
	normal_bake_session *s               = arg;
	char                 temporary[4200] = {0};
	if (s->blender_script && s->inspecting) {
		normal_bake_write(&s->phase, 0);
		if (!normal_bake_export_blend(s, temporary, sizeof(temporary), true)) {
			normal_bake_write(&s->done, true);
			return;
		}
#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
		FILE    *file  = bn_open(temporary, "rb");
		uint32_t count = 0;
		bool     valid = file && fread(&count, 4, 1, file) == 1 && count > 0 && count <= 65536;
		if (valid) {
			s->source_names = calloc(count, sizeof(char *));
			valid           = s->source_names != NULL;
		}
		for (uint32_t i = 0; valid && i < count; i++) {
			uint32_t len;
			valid = fread(&len, 4, 1, file) == 1 && len > 0 && len < 4096;
			if (valid) {
				s->source_names[i] = calloc(len + 1, 1);
				s->source_count++;
				valid = s->source_names[i] && fread(s->source_names[i], 1, len, file) == len;
			}
		}
		if (file)
			fclose(file);
		bn_remove(temporary);
		temporary[0] = 0;
		if (!valid) {
			snprintf(s->error, sizeof(s->error), "Could not read Blender mesh list");
			normal_bake_write(&s->done, true);
			return;
		}
		s->source_index = count == 1 ? 0 : -1;
		if (s->blender_object && s->blender_object[0])
			for (int i = 0; i < s->source_count; i++)
				if (strcmp(s->source_names[i], s->blender_object) == 0)
					s->source_index = i;
		if (s->source_index < 0) {
			normal_bake_write(&s->done, true);
			return;
		}
		free(s->blender_object);
		s->blender_object = string_copy(s->source_names[s->source_index]);
#endif
	}
	if (s->blender_script && !normal_bake_export_blend(s, temporary, sizeof(temporary), false)) {
		normal_bake_write(&s->done, true);
		return;
	}
	normal_bake_write(&s->phase, 1);
	ufbx_load_opts opts = {.generate_missing_normals = true,
	                       .target_axes              = ufbx_axes_right_handed_y_up,
	                       .progress_cb              = {.fn = normal_bake_import_progress, .user = s},
	                       .ignore_animation         = true,
	                       .load_external_files      = false};
	ufbx_error     error;
	ufbx_scene    *scene = s->source_scene ? s->source_scene : ufbx_load_file(temporary[0] ? temporary : s->path, &opts, &error);
#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
	if (temporary[0])
		bn_remove(temporary);
#endif
	if (!scene) {
		snprintf(s->error, sizeof(s->error), "Could not load high-poly: %s", error.description.data);
		normal_bake_write(&s->done, true);
		return;
	}
	bool retained = !s->blender_script;
	if (retained && !s->source_scene) {
		s->source_scene = scene;
		for (size_t i = 0; i < scene->nodes.count; i++)
			if (scene->nodes.data[i]->mesh && scene->nodes.data[i]->mesh->num_triangles)
				s->source_count++;
		s->source_names = calloc(s->source_count, sizeof(char *));
		s->source_nodes = calloc(s->source_count, sizeof(int));
		if (!s->source_names || !s->source_nodes) {
			s->source_count = 0;
			snprintf(s->error, sizeof(s->error), "Not enough memory for object list");
			normal_bake_write(&s->done, true);
			return;
		}
		int at = 0;
		for (size_t i = 0; i < scene->nodes.count; i++)
			if (scene->nodes.data[i]->mesh && scene->nodes.data[i]->mesh->num_triangles) {
				s->source_names[at]   = string_copy((char *)scene->nodes.data[i]->name.data);
				s->source_nodes[at++] = (int)i;
			}
		s->source_index = s->source_count == 1 ? 0 : -1;
		if (s->source_count > 1) {
			normal_bake_write(&s->done, true);
			return;
		}
	}
	normal_bake_write(&s->phase, 2);
	size_t total = 0;
	for (size_t i = 0; i < scene->nodes.count; i++)
		if (scene->nodes.data[i]->mesh && (!retained || s->source_index < 0 || s->source_nodes[s->source_index] == (int)i))
			total += scene->nodes.data[i]->mesh->num_triangles;
	if (!total || total > 10000000) {
		snprintf(s->error, sizeof(s->error), "Source must contain 1 to 10 million visible triangles");
		if (!retained)
			ufbx_free_scene(scene);
		normal_bake_write(&s->done, true);
		return;
	}
	bn_face *faces = malloc(total * sizeof(bn_face));
	if (!faces) {
		snprintf(s->error, sizeof(s->error), "Not enough memory for high-poly");
		if (!retained)
			ufbx_free_scene(scene);
		normal_bake_write(&s->done, true);
		return;
	}
	size_t used = 0;
	bool   ok   = true;
	mat4_t nm   = mat4_transpose(mat4_inv(s->import_transform));
	for (size_t i = 0; i < scene->nodes.count && ok; i++) {
		ufbx_node *node = scene->nodes.data[i];
		ufbx_mesh *m    = node->mesh;
		if (!m || !m->num_triangles || (retained && s->source_index >= 0 && s->source_nodes[s->source_index] != (int)i))
			continue;
		uint32_t *indices = malloc(m->max_face_triangles * 3 * sizeof(uint32_t));
		if (!indices) {
			ok = false;
			break;
		}
		ufbx_matrix normals = ufbx_get_compatible_matrix_for_normals(node);
		for (size_t f = 0; f < m->faces.count; f++) {
			if ((f & 4095) == 0 && !normal_bake_progress(s, 65 + (int)(used * 34 / total))) {
				ok = false;
				break;
			}
			uint32_t triangles = ufbx_triangulate_face(indices, m->max_face_triangles * 3, m, m->faces.data[f]);
			for (uint32_t t = 0; t < triangles; t++) {
				bn_face *face = &faces[used++];
				for (int k = 0; k < 3; k++) {
					uint32_t  v = indices[t * 3 + k];
					ufbx_vec3 p = ufbx_transform_position(&node->geometry_to_world, ufbx_get_vertex_vec3(&m->vertex_position, v));
					ufbx_vec3 n = ufbx_transform_direction(&normals, ufbx_get_vertex_vec3(&m->vertex_normal, v));
					face->p[k]  = normal_bake_point((bn_v3){(float)p.x, (float)-p.z, (float)p.y}, s->import_transform, 1);
					face->n[k]  = bn_norm(normal_bake_point((bn_v3){(float)n.x, (float)-n.z, (float)n.y}, nm, 0));
					if (!isfinite(face->p[k].x) || !isfinite(face->p[k].y) || !isfinite(face->p[k].z))
						ok = false;
				}
			}
		}
		free(indices);
	}
	if (!retained)
		ufbx_free_scene(scene);
	if (ok) {
		s->imported       = faces;
		s->imported_count = (int)used;
	}
	else {
		free(faces);
		snprintf(s->error, sizeof(s->error), "Import cancelled or invalid geometry");
	}
	normal_bake_write(&s->done, true);
}
#endif

static void normal_bake_worker(void *arg) {
	normal_bake_session *s = arg;
	mat4_t               m = s->work_transform, nm = mat4_transpose(mat4_inv(m));
	for (int i = 0; i < s->high_count; i++) {
		if ((i & 4095) == 0) {
			normal_bake_write(&s->phase, 3);
			normal_bake_write(&s->progress, i * 10 / s->high_count);
		}
		if ((i & 4095) == 0 && normal_bake_read(&s->cancel)) {
			snprintf(s->error, sizeof(s->error), "Bake cancelled");
			normal_bake_write(&s->done, true);
			return;
		}
		for (int k = 0; k < 3; k++) {
			s->work_high[i].p[k] = normal_bake_point(s->high[i].p[k], m, 1);
			s->work_high[i].n[k] = bn_norm(normal_bake_point(s->high[i].n[k], nm, 0));
		}
	}
	bn_tree tree;
	s->building = true;
	bool ok     = bn_tree_init(&tree, s->work_high, s->high_count, normal_bake_progress, s);
	s->building = false;
	if (ok && s->analyzing)
		ok = bn_analyze(s->low, s->low_count, &tree, s->job_options, s->analysis_risk, normal_bake_progress, s);
	else if (ok) {
		int index = 0;
		for (int type = 0; type < 4 && ok; type++) {
			if (!(s->map_mask & (1 << type)))
				continue;
			normal_bake_write(&s->map_index, index++);
			normal_bake_write(&s->current_map, type);
			ok = bn_bake_tree(s->low, s->low_count, &tree, s->options[type], &s->outputs[type].result, normal_bake_progress, s);
		}
	}
	free(tree.order);
	free(tree.nodes);
	if (!ok) {
		if (s->analyzing) {
			free(s->analysis_risk);
			s->analysis_risk = NULL;
		}
		else
			for (int i = 0; i < 4; i++)
				bn_result_free(&s->outputs[i].result);
		snprintf(s->error, sizeof(s->error),
		         s->analyzing ? "Projection check cancelled or insufficient memory" : "Bake cancelled, insufficient memory or no valid target UVs");
	}
	normal_bake_write(&s->done, true);
}

static gpu_texture_t *normal_bake_texture(uint8_t *pixels, int size) {
	gpu_texture_t *t = calloc(1, sizeof(gpu_texture_t));
	if (t)
		gpu_texture_init_from_bytes(t, pixels, size, size, GPU_TEXTURE_FORMAT_RGBA32, false);
	return t;
}

static void normal_bake_preview_result() {
	if (normal_bake.image)
		gpu_delete_texture(normal_bake.image);
	if (normal_bake.diagnostic)
		gpu_delete_texture(normal_bake.diagnostic);
	normal_bake.image = normal_bake.diagnostic = NULL;
	normal_bake.image                          = normal_bake_texture(normal_bake_current()->result.rgba, normal_bake.options[normal_bake.active_type].size);
	size_t   count                             = (size_t)normal_bake.options[normal_bake.active_type].size * normal_bake.options[normal_bake.active_type].size;
	uint8_t *pixels                            = malloc(count * 4);
	if (pixels) {
		for (size_t i = 0; i < count; i++) {
			uint8_t m         = normal_bake_current()->result.mask[i];
			pixels[i * 4]     = m == 1 ? 255 : 30;
			pixels[i * 4 + 1] = m == 2 ? 180 : 30;
			pixels[i * 4 + 2] = 30;
			pixels[i * 4 + 3] = 255;
		}
		normal_bake.diagnostic = normal_bake_texture(pixels, normal_bake.options[normal_bake.active_type].size);
		free(pixels);
	}
}

bool bake_normal_select_result(i32 type) {
	if (normal_bake.running || type < 0 || type > 3 || !normal_bake.outputs[type].result.rgba)
		return false;
	normal_bake.active_type = type;
	normal_bake_preview_result();
	normal_bake.preview_dirty = true;
	if (ui_box_hwnd)
		ui_box_hwnd->redraws = 2;
	return true;
}

i32 bake_normal_result_mask() {
	if (normal_bake.running)
		return 0;
	int mask = 0;
	for (int i = 0; i < 4; i++)
		if (normal_bake.outputs[i].result.rgba)
			mask |= 1 << i;
	return mask;
}

static void normal_bake_poll(void *unused) {
	if (!normal_bake.running)
		return;
	if (ui_box_hwnd)
		ui_box_hwnd->redraws = 2;
	if (!normal_bake_read(&normal_bake.done))
		return;
	iron_thread_wait_and_destroy(&normal_bake.thread);
	bool cancelled = normal_bake_read(&normal_bake.cancel) != 0;
	iron_mutex_destroy(&normal_bake.mutex);
	normal_bake.running = false;
	sys_remove_update(normal_bake_poll);
	if (normal_bake.closing) {
		normal_bake_release();
		return;
	}
	if (cancelled) {
		free(normal_bake.imported);
		normal_bake.imported = NULL;
		if (!normal_bake.analyzing) {
			for (int i = 0; i < 4; i++)
				bn_result_free(&normal_bake.outputs[i].result);
		}
		else {
			free(normal_bake.analysis_risk);
			normal_bake.analysis_risk = NULL;
		}
		snprintf(normal_bake.error, sizeof(normal_bake.error), normal_bake.loading ? "Import cancelled" : "Bake cancelled");
	}
	if (normal_bake.loading) {
		if (normal_bake.imported) {
			free(normal_bake.high);
			normal_bake.high       = normal_bake.imported;
			normal_bake.high_count = normal_bake.imported_count;
			normal_bake.imported   = NULL;
			normal_bake.high_uid   = -1;
		}
		normal_bake.status = normal_bake.error[0] ? -1 : !normal_bake.high && normal_bake.source_count > 1 ? 5 : 0;
	}
	else {
		free(normal_bake.work_high);
		normal_bake.work_high = NULL;
		normal_bake.status    = normal_bake_current()->result.rgba ? 3 : normal_bake.analyzing && !normal_bake.error[0] ? 0 : -1;
		if (normal_bake.analyzing && normal_bake.analysis_risk)
			normal_bake.analysis_version++;
		if (normal_bake.status == 3 && !normal_bake.analyzing) {
			normal_bake.result_version++;
			normal_bake_preview_result();
		}
	}
	normal_bake.preview_dirty = true;
}

static void normal_bake_launch(bool loading) {
	iron_mutex_init(&normal_bake.mutex);
	normal_bake.loading  = loading;
	normal_bake.running  = true;
	normal_bake.closing  = false;
	normal_bake.error[0] = 0;
	normal_bake_write(&normal_bake.cancel, false);
	normal_bake_write(&normal_bake.done, false);
	normal_bake_write(&normal_bake.progress, 0);
	normal_bake_write(&normal_bake.phase, loading ? (normal_bake.blender_script ? 0 : 1) : 3);
	normal_bake.status = loading ? 1 : 2;
	sys_notify_on_update(normal_bake_poll, NULL);
}

bool bake_normal_select_low(mesh_object_t *object, i32 uv) {
	if (normal_bake.running || !object || normal_bake_object(object->base->uid) != object || (uv != 0 && uv != 1))
		return false;
	int     count;
	bn_low *low = normal_bake_snapshot(object, uv, &count, true);
	if (!low) {
		snprintf(normal_bake.error, sizeof(normal_bake.error), "Target has no selected UV set");
		return false;
	}
	for (int i = 0; i < count; i++)
		for (int k = 0; k < 3; k++)
			if (low[i].uv[k][0] < -0.0001f || low[i].uv[k][0] > 1.0001f || low[i].uv[k][1] < -0.0001f || low[i].uv[k][1] > 1.0001f) {
				free(low);
				snprintf(normal_bake.error, sizeof(normal_bake.error), "Target UVs must be inside 0-1; import UDIM tiles as separate targets");
				return false;
			}
	normal_bake_clear_result();
	free(normal_bake.low);
	if (normal_bake.target_uid != object->base->uid) {
		free(normal_bake.high);
		normal_bake.high       = NULL;
		normal_bake.high_count = 0;
		normal_bake.high_uid   = -1;
	}
	normal_bake.low              = low;
	normal_bake.low_count        = count;
	normal_bake.target_uid       = object->base->uid;
	normal_bake.uv               = uv;
	normal_bake.import_transform = object->base->transform->world;
	normal_bake.preview_dirty    = true;
	normal_bake.error[0]         = 0;
	return true;
}

bool bake_normal_select_high(mesh_object_t *object) {
	if (normal_bake.running)
		return false;
	if (!object || object->base->uid == normal_bake.target_uid || normal_bake_object(object->base->uid) != object) {
		snprintf(normal_bake.error, sizeof(normal_bake.error), "Choose a different project mesh as the high-poly source");
		return false;
	}
	int     count;
	bn_low *source = normal_bake_snapshot(object, 0, &count, false);
	if (!source)
		return false;
	bn_face *high = malloc((size_t)count * sizeof(bn_face));
	if (!high) {
		free(source);
		return false;
	}
	for (int i = 0; i < count; i++)
		high[i] = source[i].face;
	free(source);
	normal_bake_clear_result();
	free(normal_bake.high);
	normal_bake.high       = high;
	normal_bake.high_count = count;
	normal_bake.high_uid   = object->base->uid;
	normal_bake_clear_source();
	free(normal_bake.path);
	normal_bake.path   = NULL;
	normal_bake.scale  = 1;
	normal_bake.offset = normal_bake.rotation = (vec4_t){0};
	normal_bake.preview_dirty                 = true;
	normal_bake.error[0]                      = 0;
	return true;
}

bool bake_normal_configure_blender(char *executable, char *object_name, bool selected_only) {
	if (normal_bake.running)
		return false;
	free(normal_bake.blender);
	free(normal_bake.blender_object);
	normal_bake.blender          = string_copy(executable ? executable : "");
	normal_bake.blender_object   = string_copy(object_name ? object_name : "");
	normal_bake.blender_selected = selected_only;
	return true;
}

bool bake_normal_load(char *path) {
#ifdef WITH_PLUGINS
	if (normal_bake.running || !normal_bake.low || !path)
		return false;
	char *lower     = to_lower_case(path);
	bool  blend     = ends_with(lower, ".blend");
	bool  supported = ends_with(lower, ".fbx") || ends_with(lower, ".obj") || blend;
	free(lower);
	if (!supported) {
		snprintf(normal_bake.error, sizeof(normal_bake.error), "Choose an FBX, OBJ or Blender file");
		return false;
	}
	free(normal_bake.blender_script);
	normal_bake.blender_script = NULL;
	if (blend) {
		if (!normal_bake.blender) {
			char *detected = normal_bake_find_blender();
			bake_normal_configure_blender(g_config->blender && g_config->blender[0] ? g_config->blender : detected, "", true);
			free(detected);
		}
		if (!normal_bake.blender[0]) {
			snprintf(normal_bake.error, sizeof(normal_bake.error), "Set the Blender executable under Blender Import");
			return false;
		}
		buffer_t *script = data_get_blob("bake_normal.py");
		if (!script) {
			snprintf(normal_bake.error, sizeof(normal_bake.error), "Blender export script is missing");
			return false;
		}
		normal_bake.blender_script = calloc(script->length + 1, 1);
		if (normal_bake.blender_script)
			memcpy(normal_bake.blender_script, script->buffer, script->length);
		data_delete_blob("bake_normal.py");
		if (!normal_bake.blender_script)
			return false;
	}
	normal_bake_clear_result();
	normal_bake_clear_source();
	normal_bake.inspecting = true;
	normal_bake.analyzing  = false;
	free(normal_bake.path);
	normal_bake.path = string_copy(path);
	free(normal_bake.high);
	normal_bake.high       = NULL;
	normal_bake.high_count = 0;
	normal_bake.high_uid   = -1;
	normal_bake.scale      = 1;
	normal_bake.offset = normal_bake.rotation = (vec4_t){0};
	normal_bake_launch(true);
	iron_thread_init(&normal_bake.thread, normal_bake_import_worker, &normal_bake);
	return true;
#else
	snprintf(normal_bake.error, sizeof(normal_bake.error), "FBX/OBJ importer is unavailable in this build");
	return false;
#endif
}

i32 bake_normal_source_count() {
	return normal_bake.running ? 0 : normal_bake.source_count;
}
char *bake_normal_source_name(i32 index) {
	return !normal_bake.running && index >= 0 && index < normal_bake.source_count ? normal_bake.source_names[index] : "";
}
bool bake_normal_select_source(i32 index) {
#ifdef WITH_PLUGINS
	if (normal_bake.running || !normal_bake.path || index < -1 || index >= normal_bake.source_count || normal_bake.source_count == 0)
		return false;
	normal_bake_clear_result();
	free(normal_bake.high);
	normal_bake.high         = NULL;
	normal_bake.high_count   = 0;
	normal_bake.source_index = index;
	normal_bake.inspecting   = false;
	normal_bake.analyzing    = false;
	if (normal_bake.blender_script) {
		free(normal_bake.blender_object);
		normal_bake.blender_object   = string_copy(index < 0 ? "" : normal_bake.source_names[index]);
		normal_bake.blender_selected = false;
	}
	normal_bake_launch(true);
	iron_thread_init(&normal_bake.thread, normal_bake_import_worker, &normal_bake);
	return true;
#else
	return false;
#endif
}

bool bake_normal_set_type(i32 type, i32 rays, f32 radius, f32 strength) {
	if (normal_bake.running || type < BN_NORMAL || type > BN_HEIGHT || rays < 1 || rays > 128 || !isfinite(radius) || radius <= 0 || !isfinite(strength) ||
	    strength <= 0)
		return false;
	normal_bake.map_mask       = 1 << type;
	normal_bake.settings[type] = (bn_options){.rays = rays, .radius = radius, .strength = strength};
	return true;
}

bool bake_normal_set_maps(i32 mask) {
	if (normal_bake.running || mask <= 0 || (mask & ~15))
		return false;
	normal_bake.map_mask = mask;
	return true;
}

bool bake_normal_analyze(f32 outer, f32 inner, bool backfaces) {
	if (normal_bake.running || !normal_bake.high || !normal_bake.low || !isfinite(outer + inner) || outer < 0 || inner < 0 || outer + inner <= 0)
		return false;
	free(normal_bake.analysis_risk);
	normal_bake.analysis_risk = calloc(normal_bake.low_count, 1);
	normal_bake.work_high     = malloc((size_t)normal_bake.high_count * sizeof(bn_face));
	if (!normal_bake.analysis_risk || !normal_bake.work_high) {
		free(normal_bake.analysis_risk);
		free(normal_bake.work_high);
		normal_bake.analysis_risk = NULL;
		normal_bake.work_high     = NULL;
		return false;
	}
	normal_bake.job_options    = (bn_options){.outer = outer, .inner = inner, .backfaces = backfaces};
	normal_bake.work_transform = normal_bake_adjustment();
	normal_bake.analyzing      = true;
	normal_bake_launch(false);
	normal_bake.status = 4;
	iron_thread_init(&normal_bake.thread, normal_bake_worker, &normal_bake);
	return true;
}

static mat4_t normal_bake_adjustment() {
	vec4_t r = normal_bake.rotation;
	return mat4_compose(normal_bake.offset, quat_from_euler(r.x * math_pi() / 180, r.y * math_pi() / 180, r.z * math_pi() / 180),
	                    (vec4_t){normal_bake.scale, normal_bake.scale, normal_bake.scale, 1});
}

bool bake_normal_start(i32 size, f32 outer, f32 inner, i32 padding, bool directx, bool backfaces, i32 samples) {
	if (normal_bake.running || !normal_bake.high || !normal_bake_object(normal_bake.target_uid) || size < 16 || size > 4096 || padding < 0 || padding > 64 ||
	    !isfinite(outer) || !isfinite(inner) || !isfinite(outer + inner) || outer < 0 || inner < 0 || outer + inner <= 0 || (samples != 1 && samples != 4) ||
	    !isfinite(normal_bake.scale) || normal_bake.scale <= 0 || !isfinite(normal_bake.offset.x) || !isfinite(normal_bake.offset.y) ||
	    !isfinite(normal_bake.offset.z) || !isfinite(normal_bake.rotation.x) || !isfinite(normal_bake.rotation.y) || !isfinite(normal_bake.rotation.z))
		return false;
	normal_bake_clear_result();
	normal_bake.map_count   = 0;
	normal_bake.active_type = -1;
	for (int type = 0; type < 4; type++) {
		if (!(normal_bake.map_mask & (1 << type)))
			continue;
		if (normal_bake.active_type < 0)
			normal_bake.active_type = type;
		normal_bake.map_count++;
		bn_options settings                = normal_bake.settings[type];
		normal_bake.options[type]          = (bn_options){size, padding, samples, outer, inner, directx, backfaces};
		normal_bake.options[type].type     = type;
		normal_bake.options[type].rays     = settings.rays ? settings.rays : 16;
		normal_bake.options[type].radius   = settings.radius > 0 ? settings.radius : .1f;
		normal_bake.options[type].strength = settings.strength > 0 ? settings.strength : 2;
	}
	normal_bake.map_index   = 0;
	normal_bake.current_map = normal_bake.active_type;
	normal_bake.analyzing   = false;
	normal_bake.work_high   = malloc((size_t)normal_bake.high_count * sizeof(bn_face));
	if (!normal_bake.work_high)
		return false;
	normal_bake.work_transform = normal_bake_adjustment();
	normal_bake_launch(false);
	iron_thread_init(&normal_bake.thread, normal_bake_worker, &normal_bake);
	return true;
}

i32 bake_normal_status() {
	normal_bake_poll(NULL);
	return normal_bake.status;
}
i32 bake_normal_progress() {
	return !normal_bake.running ? 100 : normal_bake_read(&normal_bake.phase) == 0 ? -1 : normal_bake_read(&normal_bake.progress);
}
char *bake_normal_error() {
	return normal_bake.running ? "" : normal_bake.error;
}
i32 bake_normal_triangle_count(bool high) {
	return high ? normal_bake.high_count : normal_bake.low_count;
}
void bake_normal_cancel() {
	if (normal_bake.running)
		normal_bake_write(&normal_bake.cancel, true);
}
void bake_normal_close() {
	if (normal_bake.running) {
		normal_bake.closing = true;
		bake_normal_cancel();
	}
	else
		normal_bake_release();
}
void bake_normal_shutdown() {
	if (normal_bake.running) {
		bake_normal_cancel();
		iron_thread_wait_and_destroy(&normal_bake.thread);
		iron_mutex_destroy(&normal_bake.mutex);
		normal_bake.running = false;
		sys_remove_update(normal_bake_poll);
	}
	normal_bake_release();
}

bool bake_normal_apply(bool add_layer) {
	mesh_object_t *target = normal_bake_object(normal_bake.target_uid);
	if (normal_bake.running || normal_bake.status != 3 || normal_bake_current()->applied || normal_bake.result_stale || !target ||
	    !normal_bake_current()->result.rgba || normal_bake_current()->result.covered == normal_bake_current()->result.missed)
		return false;
	// Pack the generated image immediately: it has no external file dependency.
	int       size   = normal_bake.options[normal_bake.active_type].size;
	buffer_t  pixels = {.buffer = normal_bake_current()->result.rgba, .length = size * size * 4, .capacity = size * size * 4};
	buffer_t *png    = iron_encode_png(&pixels, size, size, 0);
	if (!png)
		return false;
	gpu_texture_t *image = normal_bake_texture(normal_bake_current()->result.rgba, size);
	if (!image) {
		array_free(png);
		free(png);
		return false;
	}
	char *kind = normal_bake.active_type == BN_AO          ? "AO"
	             : normal_bake.active_type == BN_CURVATURE ? "Curvature"
	             : normal_bake.active_type == BN_HEIGHT    ? "Height"
	                                                       : "Normal";
	char *name = string("Baked%s_%d.png", kind, g_project->_->next_asset_id);
	while (g_project->packed_assets && project_packed_asset_exists(g_project->packed_assets, name)) {
		free(name);
		name = string("Baked%s_%d.png", kind, ++g_project->_->next_asset_id);
	}
	asset_t *asset = ALLOC_INIT(asset_t, {.name = name, .file = string_copy(name), .id = g_project->_->next_asset_id++, .image = image});
	int      index = g_project->_->assets->length;
	any_array_push(g_project->_->assets, asset);
	any_map_set(data_cached_textures, asset->file, image);
	if (!g_project->packed_assets)
		g_project->packed_assets = any_array_create(0);
	any_array_push(g_project->packed_assets, ALLOC_INIT(packed_asset_t, {.name = string_copy(asset->file), .bytes = png}));
	slot_material_t *material = script_material_create(string_tmp("%s %s", target->base->name, kind));
	material->paint_base = material->paint_opac = material->paint_occ = material->paint_rough = material->paint_met = material->paint_height =
	    material->paint_emis = material->paint_subs = false;
	// Alpha is the coverage mask for every channel, including normals.
	material->paint_nor                                   = normal_bake.active_type == BN_NORMAL;
	material->paint_occ                                   = normal_bake.active_type == BN_AO;
	material->paint_height                                = normal_bake.active_type == BN_HEIGHT;
	material->paint_base                                  = normal_bake.active_type == BN_CURVATURE;
	material->paint_opac                                  = true;
	ui_node_t *texture                                    = script_material_create_node_at("TEX_IMAGE", 40, 160);
	texture->buttons->buffer[0]->default_value->buffer[0] = index;
	texture->buttons->buffer[1]->default_value->buffer[0] =
	    normal_bake.active_type == BN_NORMAL && normal_bake.options[normal_bake.active_type].directx ? COLOR_SPACE_DIRECTX_NORMAL_MAP : COLOR_SPACE_LINEAR;
	// Curvature is scalar mask data, including when previewed in the color channel.
	if (normal_bake.active_type == BN_CURVATURE)
		texture->buttons->buffer[1]->default_value->buffer[0] = COLOR_SPACE_AUTO;
	ui_node_t *output = script_material_get_node("OUTPUT_MATERIAL_PBR");
	int        socket = normal_bake.active_type == BN_NORMAL ? 5 : normal_bake.active_type == BN_AO ? 2 : normal_bake.active_type == BN_HEIGHT ? 7 : 0;
	script_material_connect(texture, 0, output, socket);
	script_material_update();
	if (add_layer) {
		slot_layer_t *l = script_layer_new();
		script_layer_set_object(l, target->base);
		l->uv_map     = normal_bake.uv;
		l->paint_base = l->paint_opac = l->paint_occ = l->paint_rough = l->paint_met = l->paint_height = l->paint_emis = l->paint_subs = false;
		l->paint_nor                                                                                                                   = material->paint_nor;
		l->paint_occ                                                                                                                   = material->paint_occ;
		l->paint_height                                                                                                                = material->paint_height;
		l->paint_base                                                                                                                  = material->paint_base;
		script_layer_set_name(l, material->canvas->name);
		history_to_fill_layer();
		slot_layer_to_fill_layer(l);
	}
	normal_bake_current()->applied = true;
	ui_dock_redraw(TAB_AREA_STATUS);
	ui_dock_redraw(TAB_AREA_SIDEBAR0);
	ui_view2d_hwnd->redraws = 2;
	g_context->ddirty       = 3;
	return true;
}

bool bake_normal_apply_all(bool add_layer) {
	if (normal_bake.running || normal_bake.status != 3 || normal_bake.result_stale)
		return false;
	int  active = normal_bake.active_type;
	bool any = false, ok = true;
	for (int type = 0; type < 4; type++) {
		if (!normal_bake.outputs[type].result.rgba || normal_bake.outputs[type].applied)
			continue;
		normal_bake.active_type = type;
		if (!bake_normal_apply(add_layer)) {
			ok = false;
			break;
		}
		any = true;
	}
	normal_bake.active_type = active;
	return ok && any;
}
