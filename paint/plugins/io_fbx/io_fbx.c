#include "iron_array.h"
#include "iron_obj.h"
#include "ufbx/ufbx.h"
#include <math.h>
#include <stdlib.h>

static bool        has_next         = false;
static int         current_node     = 0;
static float       scale_pos        = 1.0;
static ufbx_scene *active_base_scene = NULL;
static ufbx_scene *active_eval_scene = NULL;
static bool        active_tex1       = false;

void io_fbx_parse_mesh(raw_mesh_t *raw, ufbx_mesh *mesh, ufbx_matrix *to_world, ufbx_matrix *to_world_unscaled) {
	uint32_t  indices_size = mesh->max_face_triangles * 3;
	uint32_t *indices      = (uint32_t *)malloc(sizeof(uint32_t) * indices_size);

	bool has_tex  = mesh->vertex_uv.exists;
	bool has_tex1 = active_tex1 && mesh->uv_sets.count > 1;
	bool has_col  = mesh->vertex_color.exists;

	int    numtri  = mesh->num_triangles;
	float *posa32  = (float *)malloc(sizeof(float) * numtri * 3 * 3);
	float *nora32  = (float *)malloc(sizeof(float) * numtri * 3 * 3);
	float *texa32  = has_tex ? (float *)malloc(sizeof(float) * numtri * 3 * 2) : NULL;
	float *texa132 = has_tex1 ? (float *)malloc(sizeof(float) * numtri * 3 * 2) : NULL;
	float *cola32  = has_col ? (float *)malloc(sizeof(float) * numtri * 3 * 4) : NULL;
	int    pi      = 0;
	int    ni      = 0;
	int    ti      = 0;
	int    ti1     = 0;
	int    ci      = 0;

	for (int j = 0; j < mesh->faces.count; ++j) {
		ufbx_face face          = mesh->faces.data[j];
		uint32_t  num_triangles = ufbx_triangulate_face(indices, indices_size, mesh, face);
		for (uint32_t v_ix = 0; v_ix < num_triangles * 3; v_ix++) {
			uint32_t a = indices[v_ix];

			ufbx_vec3 v  = ufbx_transform_position(to_world, ufbx_get_vertex_vec3(&mesh->vertex_position, a));
			// posa32[pi++] = v.x;
			// posa32[pi++] = v.y;
			// posa32[pi++] = v.z;
			posa32[pi++] = v.x;
			posa32[pi++] = -v.z;
			posa32[pi++] = v.y;

			v            = ufbx_transform_direction(to_world_unscaled, ufbx_get_vertex_vec3(&mesh->vertex_normal, a));
			// nora32[ni++] = v.x;
			// nora32[ni++] = v.y;
			// nora32[ni++] = v.z;
			nora32[ni++] = v.x;
			nora32[ni++] = -v.z;
			nora32[ni++] = v.y;

			if (has_tex) {
				texa32[ti++] = ufbx_get_vertex_vec2(&mesh->vertex_uv, a).x;
				texa32[ti++] = ufbx_get_vertex_vec2(&mesh->vertex_uv, a).y;
			}

			if (has_tex1) {
				texa132[ti1++] = ufbx_get_vertex_vec2(&mesh->uv_sets.data[1].vertex_uv, a).x;
				texa132[ti1++] = ufbx_get_vertex_vec2(&mesh->uv_sets.data[1].vertex_uv, a).y;
			}

			if (has_col) {
				cola32[ci++] = ufbx_get_vertex_vec4(&mesh->vertex_color, a).x;
				cola32[ci++] = ufbx_get_vertex_vec4(&mesh->vertex_color, a).y;
				cola32[ci++] = ufbx_get_vertex_vec4(&mesh->vertex_color, a).z;
				cola32[ci++] = ufbx_get_vertex_vec4(&mesh->vertex_color, a).w;
			}
		}
	}

	free(indices);

	int vertex_count = pi / 3;
	int index_count  = vertex_count;

	uint32_t *inda = malloc(sizeof(uint32_t) * index_count);
	for (int i = 0; i < index_count; ++i) {
		inda[i] = i;
	}

	// Pack positions to (-1, 1) range
	float hx = 0.0;
	float hy = 0.0;
	float hz = 0.0;
	for (int i = 0; i < vertex_count; ++i) {
		float f = fabsf(posa32[i * 3]);
		if (hx < f)
			hx = f;
		f = fabsf(posa32[i * 3 + 1]);
		if (hy < f)
			hy = f;
		f = fabsf(posa32[i * 3 + 2]);
		if (hz < f)
			hz = f;
	}
	float _scale_pos = fmax(hx, fmax(hy, hz));
	if (_scale_pos > scale_pos)
		scale_pos = _scale_pos;
	float inv = 1 / scale_pos;

	// Pack into 16bit
	short *posa = malloc(sizeof(short) * vertex_count * 4);
	for (int i = 0; i < vertex_count; ++i) {
		posa[i * 4]     = posa32[i * 3] * 32767 * inv;
		posa[i * 4 + 1] = posa32[i * 3 + 1] * 32767 * inv;
		posa[i * 4 + 2] = posa32[i * 3 + 2] * 32767 * inv;
	}

	short *nora = malloc(sizeof(short) * vertex_count * 2);
	if (nora32 != NULL) {
		for (int i = 0; i < vertex_count; ++i) {
			nora[i * 2]     = nora32[i * 3] * 32767;
			nora[i * 2 + 1] = nora32[i * 3 + 1] * 32767;
			posa[i * 4 + 3] = nora32[i * 3 + 2] * 32767;
		}
		free(nora32);
	}

	free(posa32);

	short *texa = NULL;
	if (texa32 != NULL) {
		texa = malloc(sizeof(short) * vertex_count * 2);
		for (int i = 0; i < vertex_count; ++i) {
			texa[i * 2]     = texa32[i * 2] * 32767;
			texa[i * 2 + 1] = (1.0 - texa32[i * 2 + 1]) * 32767;
		}
		free(texa32);
	}

	short *texa1 = NULL;
	if (texa132 != NULL) {
		texa1 = malloc(sizeof(short) * vertex_count * 2);
		for (int i = 0; i < vertex_count; ++i) {
			texa1[i * 2]     = texa132[i * 2] * 32767;
			texa1[i * 2 + 1] = (1.0 - texa132[i * 2 + 1]) * 32767;
		}
		free(texa132);
	}

	short *cola = NULL;
	if (cola32 != NULL) {
		cola = malloc(sizeof(short) * vertex_count * 4);
		for (int i = 0; i < vertex_count; ++i) {
			cola[i * 4]     = cola32[i * 4] * 32767;
			cola[i * 4 + 1] = cola32[i * 4 + 1] * 32767;
			cola[i * 4 + 2] = cola32[i * 4 + 2] * 32767;
			cola[i * 4 + 3] = cola32[i * 4 + 3] * 32767;
		}
		free(cola32);
	}

	raw->posa         = (i16_array_t *)malloc(sizeof(i16_array_t));
	raw->posa->buffer = posa;
	raw->posa->length = raw->posa->capacity = vertex_count * 4;

	raw->nora         = (i16_array_t *)malloc(sizeof(i16_array_t));
	raw->nora->buffer = nora;
	raw->nora->length = raw->nora->capacity = vertex_count * 2;

	if (texa != NULL) {
		raw->texa         = (i16_array_t *)malloc(sizeof(i16_array_t));
		raw->texa->buffer = texa;
		raw->texa->length = raw->texa->capacity = vertex_count * 2;
	}

	if (texa1 != NULL) {
		raw->texa1         = (i16_array_t *)malloc(sizeof(i16_array_t));
		raw->texa1->buffer = texa1;
		raw->texa1->length = raw->texa1->capacity = vertex_count * 2;
	}

	if (cola != NULL) {
		raw->cola         = (i16_array_t *)malloc(sizeof(i16_array_t));
		raw->cola->buffer = cola;
		raw->cola->length = raw->cola->capacity = vertex_count * 4;
	}

	raw->inda         = (u32_array_t *)malloc(sizeof(u32_array_t));
	raw->inda->buffer = inda;
	raw->inda->length = raw->inda->capacity = index_count;

	raw->scale_pos = scale_pos;
	raw->scale_tex = 1.0;
}

void *io_fbx_parse(char *buf, size_t size) {
	if (active_base_scene == NULL) {
		ufbx_load_opts opts = {.generate_missing_normals = true};
		active_base_scene = active_eval_scene = ufbx_load_memory(buf, size, &opts, NULL);
		active_tex1 = true;
		for (size_t i = 0; i < active_eval_scene->nodes.count; ++i) {
			ufbx_node *n = active_eval_scene->nodes.data[i];
			if (n->mesh != NULL && n->mesh->uv_sets.count < 2) {
				active_tex1 = false;
				break;
			}
		}
	}

	raw_mesh_t *raw = (raw_mesh_t *)calloc(sizeof(raw_mesh_t), 1);

	for (; current_node < (int)active_eval_scene->nodes.count; ++current_node) {
		ufbx_node *n = active_eval_scene->nodes.data[current_node];
		if (n->mesh != NULL) {
			raw->name = malloc(strlen(n->name.data) + 1);
			strcpy(raw->name, n->name.data);
			ufbx_matrix normal_mat = ufbx_get_compatible_matrix_for_normals(n);
			io_fbx_parse_mesh(raw, n->mesh, &n->geometry_to_world, &normal_mat);
			break;
		}
	}
	current_node++;

	has_next = false;
	for (int i = current_node; i < (int)active_eval_scene->nodes.count; ++i) {
		ufbx_node *n = active_eval_scene->nodes.data[i];
		if (n->mesh != NULL) {
			has_next = true;
			break;
		}
	}

	if (!has_next) {
		ufbx_free_scene(active_base_scene);
		active_base_scene = NULL;
		active_eval_scene = NULL;
		current_node      = 0;
		active_tex1       = false;
	}

	raw->has_next = has_next;

	return raw;
}

void *io_fbx_parse_skinned(char *buf, size_t size, int frame) {
	if (active_base_scene == NULL) {
		ufbx_load_opts load_opts = {.generate_missing_normals = true};
		active_base_scene = ufbx_load_memory(buf, size, &load_opts, NULL);
		if (active_base_scene == NULL)
			return NULL;

		double fps  = active_base_scene->settings.frames_per_second > 0.0 ? active_base_scene->settings.frames_per_second : 30.0;
		double time = (double)frame / fps;
		if (active_base_scene->anim_stacks.count > 0) {
			ufbx_evaluate_opts eval_opts = {.evaluate_skinning = true};
			active_eval_scene = ufbx_evaluate_scene(active_base_scene, active_base_scene->anim_stacks.data[0]->anim, time, &eval_opts, NULL);
			if (active_eval_scene == NULL) {
				ufbx_free_scene(active_base_scene);
				active_base_scene = NULL;
				return NULL;
			}
		}
		else {
			active_eval_scene = active_base_scene;
		}

		// Pre-scan: disable tex1 if any mesh lacks a second uv set
		active_tex1 = true;
		for (size_t i = 0; i < active_eval_scene->nodes.count; ++i) {
			ufbx_node *n = active_eval_scene->nodes.data[i];
			if (n->mesh != NULL && n->mesh->uv_sets.count < 2) {
				active_tex1 = false;
				break;
			}
		}
	}

	// Find next mesh node
	ufbx_node *mesh_node = NULL;
	for (; current_node < (int)active_eval_scene->nodes.count; ++current_node) {
		ufbx_node *n = active_eval_scene->nodes.data[current_node];
		if (n->mesh != NULL) {
			mesh_node = n;
			break;
		}
	}
	current_node++;

	if (mesh_node == NULL) {
		if (active_eval_scene != active_base_scene)
			ufbx_free_scene(active_eval_scene);
		ufbx_free_scene(active_base_scene);
		active_base_scene = NULL;
		active_eval_scene = NULL;
		current_node      = 0;
		has_next          = false;
		active_tex1       = false;
		return calloc(sizeof(raw_mesh_t), 1);
	}

	ufbx_mesh  *mesh           = mesh_node->mesh;
	ufbx_matrix normal_to_world = ufbx_get_compatible_matrix_for_normals(mesh_node);

	uint32_t  indices_size = mesh->max_face_triangles * 3;
	uint32_t *indices      = (uint32_t *)malloc(sizeof(uint32_t) * indices_size);

	bool   has_tex  = mesh->vertex_uv.exists;
	bool   has_tex1 = active_tex1 && mesh->uv_sets.count > 1;
	int    numtri   = mesh->num_triangles;
	float *posa32   = (float *)malloc(sizeof(float) * numtri * 3 * 3);
	float *nora32   = (float *)malloc(sizeof(float) * numtri * 3 * 3);
	float *texa32   = has_tex  ? (float *)malloc(sizeof(float) * numtri * 3 * 2) : NULL;
	float *texa132  = has_tex1 ? (float *)malloc(sizeof(float) * numtri * 3 * 2) : NULL;
	int    pi = 0, ni = 0, ti = 0, ti1 = 0;

	for (int j = 0; j < (int)mesh->faces.count; ++j) {
		ufbx_face face          = mesh->faces.data[j];
		uint32_t  num_triangles = ufbx_triangulate_face(indices, indices_size, mesh, face);
		for (uint32_t v_ix = 0; v_ix < num_triangles * 3; v_ix++) {
			uint32_t a = indices[v_ix];

			// skinned_position/normal are world-space when skinned_is_local == false
			// (evaluated skinned mesh), or local-space when skinned_is_local == true
			ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh->skinned_position, a);
			if (mesh->skinned_is_local)
				v = ufbx_transform_position(&mesh_node->geometry_to_world, v);
			posa32[pi++] = v.x;
			posa32[pi++] = -v.z;
			posa32[pi++] = v.y;

			ufbx_vec3 n = ufbx_get_vertex_vec3(&mesh->skinned_normal, a);
			if (mesh->skinned_is_local)
				n = ufbx_transform_direction(&normal_to_world, n);
			float nlen = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
			if (nlen > 1e-6f) { n.x /= nlen; n.y /= nlen; n.z /= nlen; }
			nora32[ni++] = n.x;
			nora32[ni++] = -n.z;
			nora32[ni++] = n.y;

			if (has_tex) {
				texa32[ti++] = ufbx_get_vertex_vec2(&mesh->vertex_uv, a).x;
				texa32[ti++] = ufbx_get_vertex_vec2(&mesh->vertex_uv, a).y;
			}

			if (has_tex1) {
				texa132[ti1++] = ufbx_get_vertex_vec2(&mesh->uv_sets.data[1].vertex_uv, a).x;
				texa132[ti1++] = ufbx_get_vertex_vec2(&mesh->uv_sets.data[1].vertex_uv, a).y;
			}
		}
	}

	free(indices);

	int vertex_count = pi / 3;
	int index_count  = vertex_count;

	uint32_t *inda = malloc(sizeof(uint32_t) * index_count);
	for (int i = 0; i < index_count; ++i)
		inda[i] = i;

	// Pack positions to (-1, 1) range
	float hx = 0.0, hy = 0.0, hz = 0.0;
	for (int i = 0; i < vertex_count; ++i) {
		float f = fabsf(posa32[i * 3]);
		if (hx < f) hx = f;
		f = fabsf(posa32[i * 3 + 1]);
		if (hy < f) hy = f;
		f = fabsf(posa32[i * 3 + 2]);
		if (hz < f) hz = f;
	}
	float _scale_pos = fmax(hx, fmax(hy, hz));
	if (_scale_pos > scale_pos)
		scale_pos = _scale_pos;
	float inv = 1 / scale_pos;

	// Pack into 16bit
	short *posa = malloc(sizeof(short) * vertex_count * 4);
	for (int i = 0; i < vertex_count; ++i) {
		posa[i * 4]     = posa32[i * 3] * 32767 * inv;
		posa[i * 4 + 1] = posa32[i * 3 + 1] * 32767 * inv;
		posa[i * 4 + 2] = posa32[i * 3 + 2] * 32767 * inv;
	}
	free(posa32);

	short *nora = malloc(sizeof(short) * vertex_count * 2);
	for (int i = 0; i < vertex_count; ++i) {
		nora[i * 2]     = nora32[i * 3] * 32767;
		nora[i * 2 + 1] = nora32[i * 3 + 1] * 32767;
		posa[i * 4 + 3] = nora32[i * 3 + 2] * 32767;
	}
	free(nora32);

	short *texa = NULL;
	if (texa32 != NULL) {
		texa = malloc(sizeof(short) * vertex_count * 2);
		for (int i = 0; i < vertex_count; ++i) {
			texa[i * 2]     = texa32[i * 2] * 32767;
			texa[i * 2 + 1] = (1.0 - texa32[i * 2 + 1]) * 32767;
		}
		free(texa32);
	}

	short *texa1 = NULL;
	if (texa132 != NULL) {
		texa1 = malloc(sizeof(short) * vertex_count * 2);
		for (int i = 0; i < vertex_count; ++i) {
			texa1[i * 2]     = texa132[i * 2] * 32767;
			texa1[i * 2 + 1] = (1.0 - texa132[i * 2 + 1]) * 32767;
		}
		free(texa132);
	}

	raw_mesh_t *raw = (raw_mesh_t *)calloc(sizeof(raw_mesh_t), 1);
	raw->name       = malloc(strlen(mesh_node->name.data) + 1);
	strcpy(raw->name, mesh_node->name.data);

	raw->posa         = (i16_array_t *)malloc(sizeof(i16_array_t));
	raw->posa->buffer = posa;
	raw->posa->length = raw->posa->capacity = vertex_count * 4;

	raw->nora         = (i16_array_t *)malloc(sizeof(i16_array_t));
	raw->nora->buffer = nora;
	raw->nora->length = raw->nora->capacity = vertex_count * 2;

	if (texa != NULL) {
		raw->texa         = (i16_array_t *)malloc(sizeof(i16_array_t));
		raw->texa->buffer = texa;
		raw->texa->length = raw->texa->capacity = vertex_count * 2;
	}

	if (texa1 != NULL) {
		raw->texa1         = (i16_array_t *)malloc(sizeof(i16_array_t));
		raw->texa1->buffer = texa1;
		raw->texa1->length = raw->texa1->capacity = vertex_count * 2;
	}

	raw->inda         = (u32_array_t *)malloc(sizeof(u32_array_t));
	raw->inda->buffer = inda;
	raw->inda->length = raw->inda->capacity = index_count;

	raw->scale_pos = scale_pos;
	raw->scale_tex = 1.0;

	// Check for more mesh nodes
	has_next = false;
	for (int i = current_node; i < (int)active_eval_scene->nodes.count; ++i) {
		ufbx_node *n = active_eval_scene->nodes.data[i];
		if (n->mesh != NULL) {
			has_next = true;
			break;
		}
	}

	if (!has_next) {
		if (active_eval_scene != active_base_scene)
			ufbx_free_scene(active_eval_scene);
		ufbx_free_scene(active_base_scene);
		active_base_scene = NULL;
		active_eval_scene = NULL;
		current_node      = 0;
		active_tex1       = false;
		scale_pos         = 1.0;
	}

	raw->has_next = has_next;
	return raw;
}
