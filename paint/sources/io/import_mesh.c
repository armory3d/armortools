
#include "../global.h"

bool import_mesh_clear_layers  = true;
bool import_mesh_needs_unwrap  = false;
bool import_mesh_no_reset      = false;
bool import_mesh_no_scale      = false;
bool import_mesh_keep_timeline = false;
bool import_mesh_append        = false;

static mesh_object_t *import_mesh_appended = NULL;
static i32            import_mesh_material_offset    = 0;
static i32            import_mesh_previous_stride    = 1;
static i32            import_mesh_previous_count     = 0;
static bool           import_mesh_previous_materials = false;

// Axis indices: +X, -X, +Y, -Y, +Z, -Z. Both -1 means automatic.
// Manual axes describe the source file; ArmorPaint uses +Z up and -Y facing forward.
void import_mesh_run_with_axes(char *path, bool clear_layers, bool replace_existing, bool keep_camera, i32 up, i32 forward) {
	bool automatic = up == -1 && forward == -1;
	if (!automatic && (up < 0 || up > 5 || forward < 0 || forward > 5 || up / 2 == forward / 2)) {
		console_error(tr("Up and forward axes must be perpendicular"));
		return;
	}
	i32          first    = replace_existing ? 0 : g_project->_->paint_objects->length;
	mesh_data_t *old_data = context_main_object()->data;
#ifdef WITH_PLUGINS
	extern bool io_fbx_auto_axes;
	bool        previous_auto = io_fbx_auto_axes;
	io_fbx_auto_axes          = automatic;
#endif
	import_mesh_run(path, clear_layers, replace_existing, keep_camera);
#ifdef WITH_PLUGINS
	io_fbx_auto_axes = previous_auto;
#endif
	if (automatic || (replace_existing && context_main_object()->data == old_data)) {
		return;
	}

	// Undo the built-in Y-up conversion before interpreting the file's source axes.
	// Blender's OBJ bridge already returns Blender coordinates; .skin is native Z-up.
	char *lower  = to_lower_case(path);
	bool  y_to_z = ends_with(lower, ".obj") || ends_with(lower, ".fbx") || ends_with(lower, ".gltf") || ends_with(lower, ".glb");
	free(lower);
	i32 u[3] = {0}, f[3] = {0};
	u[up / 2]      = up % 2 == 0 ? 1 : -1;
	f[forward / 2] = forward % 2 == 0 ? 1 : -1;
	i32 rows[3][3] = {
	    {u[1] * f[2] - u[2] * f[1], u[2] * f[0] - u[0] * f[2], u[0] * f[1] - u[1] * f[0]},
	    {-f[0], -f[1], -f[2]},
	    {u[0], u[1], u[2]},
	};
	i32       src[3], sign[3];
	const i32 undo_src[3] = {0, 2, 1}, undo_sign[3] = {1, 1, -1};
	for (i32 r = 0; r < 3; ++r) {
		for (i32 c = 0; c < 3; ++c) {
			if (rows[r][c] != 0) {
				src[r]  = y_to_z ? undo_src[c] : c;
				sign[r] = rows[r][c] * (y_to_z ? undo_sign[c] : 1);
			}
		}
	}
	for (i32 i = first; i < g_project->_->paint_objects->length; ++i) {
		mesh_data_t *md             = g_project->_->paint_objects->buffer[i]->data;
		bool         already_mapped = false;
		for (i32 j = first; j < i; ++j) {
			if (g_project->_->paint_objects->buffer[j]->data == md)
				already_mapped = true;
		}
		if (already_mapped)
			continue;
		i16_array_t *pa = mesh_data_get_vertex_array(md, "pos")->values;
		i16_array_t *na = mesh_data_get_vertex_array(md, "nor")->values;
		for (i32 v = 0; v < pa->length / 4; ++v) {
			i32 p[3] = {pa->buffer[v * 4], pa->buffer[v * 4 + 1], pa->buffer[v * 4 + 2]};
			i32 n[3] = {na->buffer[v * 2], na->buffer[v * 2 + 1], pa->buffer[v * 4 + 3]};
			for (i32 r = 0; r < 3; ++r) {
				i32 pv = sign[r] * p[src[r]], nv = sign[r] * n[src[r]];
				pa->buffer[v * 4 + r] = pv > 32767 ? 32767 : pv;
				if (r == 2)
					pa->buffer[v * 4 + 3] = nv > 32767 ? 32767 : nv;
				else
					na->buffer[v * 2 + r] = nv > 32767 ? 32767 : nv;
			}
		}
		util_skin_remap_axes(md->_->skin_blob, src, sign);
		mesh_data_build_vertices(md->_->vertex_buffer, md->vertex_arrays);
	}
}

void import_mesh_run(char *path, bool _clear_layers, bool replace_existing, bool keep_camera) {
	if (!path_is_mesh(path)) {
		if (!context_enable_import_plugin(path)) {
			console_error(strings_unknown_asset_format());
			return;
		}
	}

	import_mesh_clear_layers = _clear_layers;
	import_mesh_no_reset     = keep_camera;
	import_mesh_append       = !replace_existing;
	import_mesh_appended     = NULL;
	import_mesh_previous_stride    = util_mesh_atlas_stride();
	import_mesh_previous_count     = replace_existing ? 0 : g_project->_->paint_objects->length;
	import_mesh_previous_materials = !replace_existing && util_mesh_material_uvs();
	import_mesh_material_offset    = replace_existing ? 0 : import_mesh_previous_stride * import_mesh_previous_stride;
	g_context->layer_filter  = 0;

	char *p        = to_lower_case(path);
	bool  is_obj   = ends_with(p, ".obj");
	bool  is_blend = ends_with(p, ".blend");
	free(p);

	if (is_obj) {
		import_obj_run(path, replace_existing);
	}
	else if (is_blend) {
		import_blend_mesh_run(path, replace_existing);
	}
	else {
		// File recognition is case-insensitive; importer lookup must use the same convention.
		char *ext                           = to_lower_case(path + string_last_index_of(path, ".") + 1);
		raw_mesh_t *(*importer)(char *path) = any_map_get(import_mesh_importers, ext);
		free(ext);
		if (importer == NULL) {
			console_error(string("No mesh importer available for '%s'", path));
			return;
		}

		raw_mesh_t *mesh = importer(path);
		if (mesh == NULL) {
			console_error(string("Failed to import '%s'", path));
			return;
		}
		if (string_equals(mesh->name, "")) {
			mesh->name = string_copy(path_base_name(path));
		}

		replace_existing ? import_mesh_make_mesh(mesh) : import_mesh_add_mesh(mesh);

		bool has_next = mesh->has_next;
		while (has_next) {
			raw_mesh_t *mesh = importer(path);
			if (mesh == NULL) {
				console_error(string("Failed to import '%s'", path));
				break;
			}
			if (string_equals(mesh->name, "")) {
				mesh->name = string_copy(path_base_name(path));
			}
			has_next = mesh->has_next;
			import_mesh_add_mesh(mesh);
		}
	}

	g_project->mesh_assets = any_array_create_from_raw(
	    (void *[]){
	        path,
	    },
	    1);

#if defined(IRON_ANDROID) || defined(IRON_IOS)
	sys_title_set(substring(path, string_last_index_of(path, PATH_SEP) + 1, string_last_index_of(path, ".")));
#endif
}

static mesh_object_t *import_mesh_make_root(char *source) {
	char          *name = string_copy(_import_mesh_unique_name(source));
	mesh_data_t   *md   = util_shape_make_data("empty");
	md->name            = name;
	mesh_object_t *mo   = scene_add_mesh_object(md, g_context->paint_object->material, NULL);
	mo->base->name      = name;
	tab_stages_add_object(name);
	return mo;
}

i32 import_mesh_finish_import_sort(void **pa, void **pb) {
	mesh_object_t *a = *(pa);
	mesh_object_t *b = *(pb);
	return strcmp(a->base->name, b->base->name);
}

static i32 import_mesh_lod(char *name, i32 *prefix) {
	if (name == NULL)
		return -1;
	i32 n = strlen(name), p = n;
	while (p > 0 && name[p - 1] >= '0' && name[p - 1] <= '9')
		--p;
	if (p == n || p < 4 || name[p - 4] != '_' || (name[p - 3] != 'L' && name[p - 3] != 'l') || (name[p - 2] != 'O' && name[p - 2] != 'o') ||
	    (name[p - 1] != 'D' && name[p - 1] != 'd'))
		return -1;
	*prefix = p - 4;
	return atoi(name + p);
}

static void import_mesh_group_materials() {
	if (import_mesh_append && import_mesh_appended == NULL)
		return;
	i32       count = g_project->_->paint_objects->length;
	object_t *root  = import_mesh_append ? import_mesh_appended->base->parent : context_main_object()->base;
	for (i32 i = import_mesh_previous_count; i < count; ++i) {
		mesh_object_t *o      = g_project->_->paint_objects->buffer[i];
		char          *source = o->data->paint_uvs->source_object;
		if (source == NULL || (o->base->parent != root && o->base->parent != NULL))
			continue;
		mesh_object_t *group = import_mesh_make_root(source);
		object_set_parent(group->base, root);
		any_array_push(g_project->_->paint_objects, group);
		bool visible = true;
		i32  prefix = 0, lod = import_mesh_lod(source, &prefix);
		if (lod >= 0) {
			for (i32 j = 0; j < count; ++j) {
				char *other        = g_project->_->paint_objects->buffer[j]->data->paint_uvs->source_object;
				i32   other_prefix = 0, other_lod = import_mesh_lod(other, &other_prefix);
				if (other_lod >= 0 && other_lod < lod && prefix == other_prefix && strncmp(source, other, prefix) == 0)
					visible = false;
			}
		}
		group->base->visible = visible;
		stage_t *stage       = tab_stages_get_stage();
		if (stage != NULL)
			tab_stages_set_hidden(stage, group->base->name, !visible);
		for (i32 j = i; j < count; ++j) {
			mesh_object_t *part = g_project->_->paint_objects->buffer[j];
			if (part->data->paint_uvs->source_object != NULL && string_equals(part->data->paint_uvs->source_object, source) && part->base->parent == root) {
				object_set_parent(part->base, group->base);
				part->base->visible = visible;
				if (stage != NULL)
					tab_stages_set_hidden(stage, part->base->name, !visible);
			}
		}
		transform_build_matrix(group->base->transform);
	}
}

void import_mesh_finish_import(void *_) {
	// Keep existing slots stable when the atlas grows or imported parts are removed.
	i32 material_count = 0;
	for (i32 i = 0; i < g_project->_->paint_objects->length; ++i)
		material_count = math_max(material_count, g_project->_->paint_objects->buffer[i]->data->paint_uvs->count);
	if (material_count > 0) {
		for (i32 i = 0; i < g_project->_->paint_objects->length; ++i) {
			mesh_object_t *o = g_project->_->paint_objects->buffer[i];
			if (i < import_mesh_previous_count && o->data->paint_uvs->tile == 0 && o->data->index_array->length > 0) {
				i32 udim                 = util_mesh_udim_tile(o->base->name);
				o->data->paint_uvs->tile = udim < 0 ? 1 : util_mesh_udim_slot(udim) + 1;
			}
			o->data->paint_uvs->count = material_count;
		}
	}
	if (g_context->merged_object != NULL) {
		mesh_data_delete(g_context->merged_object->data);
		mesh_object_remove(g_context->merged_object);
		g_context->merged_object = NULL;
	}

	context_select_paint_object(context_main_object());

	if (!import_mesh_append) {
		// No mask by default
		for (i32 i = 0; i < g_project->_->paint_objects->length; ++i) {
			mesh_object_t *p = g_project->_->paint_objects->buffer[i];
			p->base->visible = true;
		}
	}

	// Keep appended objects at scene root
	if (import_mesh_append) {
		g_project->mesh_parents = i32_array_create(0);
	}

	if (g_project->_->paint_objects->length > 1) {
		if (!import_mesh_append) {
			// Sort by name
			array_sort(g_project->_->paint_objects, &import_mesh_finish_import_sort);

			// Parent all meshes to an empty root
			mesh_object_t *root = import_mesh_make_root("Root");
			for (i32 i = 0; i < g_project->_->paint_objects->length; ++i) {
				mesh_object_t *p = g_project->_->paint_objects->buffer[i];
				object_set_parent(p->base, root->base);
				transform_reset(p->base->transform);
				transform_build_matrix(p->base->transform);
			}
			array_insert(g_project->_->paint_objects, 0, root);
			g_project->mesh_parents = i32_array_create(0);
		}
		context_select_paint_object(import_mesh_append ? context_main_object() : g_project->_->paint_objects->buffer[1]);

		if (g_context->merged_object == NULL) {
			util_mesh_merge(NULL);
		}
		if (g_context->merged_object != NULL) {
			g_context->merged_object->base->visible = true;
		}
	}

	import_mesh_group_materials();
	util_mesh_visibility_changed();
	if (import_mesh_append && util_mesh_atlas_stride() != import_mesh_previous_stride) {
		layers_resize_atlas(import_mesh_previous_stride, util_mesh_atlas_stride());
	}

	if (import_mesh_append && import_mesh_appended != NULL && array_index_of(g_project->_->paint_objects, import_mesh_appended) >= 0) {
		context_select_paint_object(import_mesh_appended);
	}
	import_mesh_appended = NULL;

	if (!import_mesh_no_scale && !import_mesh_append) {
		viewport_scale_to_bounds(2.0);
	}
	import_mesh_no_scale = false;

	if (string_equals(g_context->paint_object->base->name, "")) {
		g_context->paint_object->base->name = "Object";
	}

	make_material_parse_paint_material(true);
	make_material_parse_mesh_material();
	ui_view2d_hwnd->redraws    = 2;
	render_path_raytrace_ready = false;
	g_context->paint_body      = NULL;
	tab_meshes_reset_preview_map();
	base_update_tools();

	if (import_mesh_needs_unwrap) {
		import_mesh_needs_unwrap = false;
		project_unwrap_mesh_box();
	}

	import_mesh_append = false;
}

void _import_mesh_make_mesh_clear_layers(void *_) {
	layers_init();
}

bool _import_mesh_is_unique_name(char *s) {
	for (i32 i = 0; i < g_project->_->paint_objects->length; ++i) {
		mesh_object_t *p = g_project->_->paint_objects->buffer[i];
		if (string_equals(p->base->name, s)) {
			return false;
		}
	}
	return true;
}

char *_import_mesh_unique_name(char *name) {
	// Returns the name or the next free .00X variant
	char *base;
	i32   i   = strings_split_number_ext(name, &base);
	char *res = i == 0 ? base : string_tmp("%s%s", base, strings_number_ext(i));
	if (_import_mesh_is_unique_name(res)) {
		return res;
	}

	// Mark the numbers after i taken by base.00X names in one pass, instead of a pass per number tried,
	// one of the next length + 1 numbers is free
	mesh_object_t_array_t *objects  = g_project->_->paint_objects;
	i32                    count    = objects->length + 1;
	bool                  *taken    = calloc(count, sizeof(bool));
	i32                    base_len = string_length(base);
	for (i32 j = 0; j < objects->length; ++j) {
		char *s = objects->buffer[j]->base->name;
		if (strncmp(s, base, base_len) != 0 || s[base_len] != '.') {
			continue;
		}
		char *digits = s + base_len + 1;
		i32   len    = 0;
		while (digits[len] >= '0' && digits[len] <= '9') {
			++len;
		}
		if (len < 3 || len > 9 || digits[len] != '\0') {
			continue;
		}
		i32 k = parse_int(digits) - i - 1;
		if (k >= 0 && k < count) {
			taken[k] = true;
		}
	}
	i32 k = 0;
	while (taken[k]) {
		++k;
	}
	free(taken);
	return string_tmp("%s%s", base, strings_number_ext(i + 1 + k));
}

void import_mesh_make_mesh(raw_mesh_t *mesh) {
	if (mesh == NULL || mesh->posa == NULL || mesh->nora == NULL || mesh->inda == NULL || mesh->posa->length == 0) {
		console_error(strings_failed_to_read_mesh_data());
		return;
	}

	import_mesh_needs_unwrap = mesh->texa == NULL;
	if (mesh->texa == NULL) {
		i32 verts  = mesh->posa->length / 4;
		mesh->texa = i16_array_create(verts * 2);
	}

	mesh_data_t *raw = import_mesh_raw_mesh(mesh);

	mesh_data_t *md    = mesh_data_create(raw);
	md->_->skin_blob   = mesh->blob;
	md->_->skin_frame  = -1;
	md->_->owns_arrays = true;

	g_context->paint_object = context_main_object();

	context_select_paint_object(context_main_object());
	if (!import_mesh_no_reset) {
		viewport_reset();
	}

	nodes_mesh_reset();
	util_mesh_remove_objects(g_project->_->paint_objects, g_context->paint_object);

	char *handle = g_context->paint_object->data->_->handle;
	if (!string_equals(handle, "SceneSphere") && !string_equals(handle, "ScenePlane")) {
		sys_notify_on_next_frame(&util_mesh_delete_data_uncache, g_context->paint_object->data);
	}

	mesh_object_set_data(g_context->paint_object, md);
	transform_build_matrix(g_context->paint_object->base->transform);
	g_context->paint_object->base->name = mesh->name;

	mesh_object_t_array_t *old_paint_objects = g_project->_->paint_objects;
	g_project->_->paint_objects              = any_array_create_from_raw(
        (void *[]){
            g_context->paint_object,
        },
        1);
	array_delete(old_paint_objects);

	if (!import_mesh_keep_timeline) {
		tab_timeline_reset();
	}

	g_context->ddirty = 4;

	ui_dock_redraw(TAB_AREA_SIDEBAR0);
	ui_dock_redraw(TAB_AREA_SIDEBAR1);
	util_uv_uvmap_cached                              = false;
	util_uv_trianglemap_cached                        = false;
	util_uv_dilatemap_cached                          = false;

	if (import_mesh_clear_layers) {
		while (g_project->_->layers->length > 0) {
			slot_layer_t *l = array_pop(g_project->_->layers);
			slot_layer_unload(l);
		}
		layers_new_layer(false, -1, NULL);
		sys_notify_on_next_frame(&_import_mesh_make_mesh_clear_layers, NULL);
		history_reset();
	}

	g_project->stages                      = NULL;
	g_context->paint_object->base->visible = true;
	tab_stages_init();

	// Wait for add_mesh calls to finish
	sys_notify_on_next_frame(&import_mesh_finish_import, NULL);
}

void import_mesh_add_mesh(raw_mesh_t *mesh) {
	if (import_mesh_previous_materials && mesh->paint_uv_tile == 0 && util_mesh_udim_tile(mesh->name) < 0) {
		mesh->paint_uv_tile  = 1;
		mesh->paint_uv_count = 1;
	}
	if (mesh->texa == NULL) {
		i32 verts  = mesh->posa->length / 4;
		mesh->texa = i16_array_create(verts * 2);
	}

	mesh_data_t *raw = import_mesh_raw_mesh(mesh);

	mesh_data_t *md    = mesh_data_create(raw);
	md->_->skin_blob   = mesh->blob;
	md->_->skin_frame  = -1;
	md->_->owns_arrays = true;

	object_t      *parent = import_mesh_append ? NULL : g_context->paint_object->base;
	mesh_object_t *object = scene_add_mesh_object(md, g_context->paint_object->material, parent);
	object->base->name    = mesh->name;

	// Ensure unique names
	char *uname = _import_mesh_unique_name(object->base->name);
	if (!string_equals(uname, object->base->name)) {
		object->base->name = string_copy(uname);
		md->name           = string_copy(uname);
	}

	any_array_push(g_project->_->paint_objects, object);
	tab_stages_add_object(object->base->name);
	if (import_mesh_append && import_mesh_appended == NULL) {
		import_mesh_appended = object;
	}

	g_context->ddirty = 4;

	ui_dock_redraw(TAB_AREA_SIDEBAR0);
	util_uv_uvmap_cached                              = false;
	util_uv_trianglemap_cached                        = false;
	util_uv_dilatemap_cached                          = false;
}

mesh_data_t *import_mesh_raw_mesh(raw_mesh_t *mesh) {
	mesh_data_t *raw =
	    ALLOC_INIT(mesh_data_t,
	               {.name           = mesh->name,
	                .paint_uvs_data = any_array_create_from_raw(
	                    (void *[]){ALLOC_INIT(mesh_paint_uvs_t, {.source_object = mesh->source_object,
	                                                             .material      = mesh->paint_material,
	                                                             .tile  = mesh->paint_uv_tile > 0 ? mesh->paint_uv_tile + import_mesh_material_offset : 0,
	                                                             .count = mesh->paint_uv_count > 0 ? mesh->paint_uv_count + import_mesh_material_offset : 0})},
	                    1),
	                .vertex_arrays = any_array_create_from_raw(
	                    (void *[]){
	                        ALLOC_INIT(vertex_array_t, {.values = mesh->posa, .attrib = "pos", .data = "short4norm"}),
	                        ALLOC_INIT(vertex_array_t, {.values = mesh->nora, .attrib = "nor", .data = "short2norm"}),
	                        ALLOC_INIT(vertex_array_t, {.values = mesh->texa, .attrib = "tex", .data = "short2norm"}),
	                    },
	                    3),
	                .index_array = mesh->inda,
	                .scale_pos   = mesh->scale_pos,
	                .scale_tex   = mesh->scale_tex});
	if (mesh->texa1 != NULL) {
		vertex_array_t *va = ALLOC_INIT(vertex_array_t, {.values = mesh->texa1, .attrib = "tex1", .data = "short2norm"});
		any_array_push(raw->vertex_arrays, va);
	}
	if (mesh->cola != NULL) {
		vertex_array_t *va = ALLOC_INIT(vertex_array_t, {.values = mesh->cola, .attrib = "col", .data = "short4norm"});
		any_array_push(raw->vertex_arrays, va);
	}
	return raw;
}
