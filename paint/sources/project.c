
#include "global.h"

void project_open_on_file_picked(char *path) {
	if (!ends_with(path, ".arm")) {
		console_error(strings_arm_file_expected());
		return;
	}

	gpu_texture_t *current = _draw_current;
	bool           in_use  = gpu_in_use;
	if (in_use)
		draw_end();

	import_arm_run_project(path);
	if (in_use)
		draw_begin(current, false, 0);
}

void project_open() {
	ui_files_show("arm", false, false, &project_open_on_file_picked);
}

void project_save_on_next_frame(void *_) {
	export_arm_run_project();
	if (_project_save_and_quit) {
		iron_stop();
	}
}

void project_save(bool save_and_quit) {
	if (string_equals(project_filepath, "")) {
#ifdef IRON_IOS
		char *document_directory = iron_save_dialog("", "");
		document_directory       = string_copy(substring(document_directory, 0, string_length(document_directory) - 8)); // Strip /"untitled"
		gc_unroot(project_filepath);
		project_filepath = string("%s/%s.arm", document_directory, sys_title());
		gc_root(project_filepath);
#elif defined(IRON_ANDROID)
		gc_unroot(project_filepath);
		project_filepath = string("%s/%s.arm", iron_internal_save_path(), sys_title());
		gc_root(project_filepath);
#else
		project_save_as(save_and_quit);
		return;
#endif
	}

#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
	char *filename = substring(project_filepath, string_last_index_of(project_filepath, PATH_SEP) + 1, string_length(project_filepath) - 4);
	sys_title_set(string("%s - %s", filename, manifest_title));
#endif

	_project_save_and_quit = save_and_quit;

	sys_notify_on_next_frame(&project_save_on_next_frame, NULL);
}

void project_save_as_on_file_picked(char *path) {
	char *f = ui_files_filename;
	if (string_equals(f, "")) {
		f = string_copy(tr("untitled", NULL));
	}
	gc_unroot(project_filepath);
	project_filepath = string("%s%s%s", path, PATH_SEP, f);
	gc_root(project_filepath);
	if (!ends_with(project_filepath, ".arm")) {
		gc_unroot(project_filepath);
		project_filepath = string("%s.arm", project_filepath);
		gc_root(project_filepath);
	}
	project_save(_project_save_and_quit);
}

void project_save_as(bool save_and_quit) {
	_project_save_and_quit = save_and_quit;
	ui_files_show("arm", true, false, &project_save_as_on_file_picked);
}

void project_fetch_default_meshes() {
	if (project_mesh_list == NULL) {
		gc_unroot(project_mesh_list);
		project_mesh_list = file_read_directory(string("%s%smeshes", path_data(), PATH_SEP));
		gc_root(project_mesh_list);
		for (i32 i = 0; i < project_mesh_list->length; ++i) {
			char *s                      = project_mesh_list->buffer[i];
			project_mesh_list->buffer[i] = substring(project_mesh_list->buffer[i], 0, string_length(s) - 4); // Trim .arm
		}
		any_array_push(project_mesh_list, "plane");
		any_array_push(project_mesh_list, "sphere");
	}
}

void project_new_box_draw() {
	project_fetch_default_meshes();
	ui_row2();
	ui_handle_t *h_project_type = ui_handle(__ID__);
	if (h_project_type->init) {
		h_project_type->i = context_raw->project_type;
	}
	context_raw->project_type           = ui_combo(h_project_type, project_mesh_list, tr("Template", NULL), true, UI_ALIGN_LEFT, true);
	ui_handle_t *h_project_aspect_ratio = ui_handle(__ID__);
	if (h_project_aspect_ratio->init) {
		h_project_aspect_ratio->i = context_raw->project_aspect_ratio;
	}
	string_t_array_t *project_aspect_ratio_combo = any_array_create_from_raw(
	    (void *[]){
	        "1:1",
	        "2:1",
	        "1:2",
	    },
	    3);
	context_raw->project_aspect_ratio = ui_combo(h_project_aspect_ratio, project_aspect_ratio_combo, tr("Aspect Ratio", NULL), true, UI_ALIGN_LEFT, true);
	ui_end_element();
	ui_row2();
	if (ui_icon_button(tr("Cancel", NULL), ICON_CLOSE, UI_ALIGN_CENTER)) {
		ui_box_hide();
	}
	if (ui_icon_button(tr("OK", NULL), ICON_CHECK, UI_ALIGN_CENTER) || ui->is_return_down) {
		project_new(true);
		ui_box_hide();
	}
}

void project_new_box() {
	ui_box_show_custom(&project_new_box_draw, 400, 200, NULL, true, tr("New Project", NULL));
}

void project_cleanup() {
	if (context_raw->merged_object != NULL) {
		mesh_object_remove(context_raw->merged_object);
		data_delete_mesh(context_raw->merged_object->data->_->handle);
		context_raw->merged_object = NULL;
	}

	if (project_paint_objects != NULL) {
		for (i32 i = 1; i < project_paint_objects->length; ++i) {
			mesh_object_t *p = project_paint_objects->buffer[i];
			if (p == context_raw->paint_object) {
				continue;
			}
			data_delete_mesh(p->data->_->handle);
			mesh_object_remove(p);
		}
	}

	mesh_object_t_array_t *meshes = scene_meshes;
	i32                    len    = meshes->length;
	for (i32 i = 0; i < len; ++i) {
		mesh_object_t *m = meshes->buffer[len - i - 1];
		if (array_index_of(context_raw->project_objects, m) == -1 && !string_equals(m->base->name, ".ParticleEmitter") &&
		    !string_equals(m->base->name, ".Particle")) {
			data_delete_mesh(m->data->_->handle);
			mesh_object_remove(m);
		}
	}

	if (context_raw->paint_object != NULL) {
		char *handle = context_raw->paint_object->data->_->handle;
		data_delete_mesh(handle);
	}

	for (i32 i = 0; i < project_assets->length; ++i) {
		asset_t *a = project_assets->buffer[i];
		data_delete_image(a->file);
	}
}

void project_new_on_next_frame(void *_) {
	// Once layers and meshes are populated on project open
	util_render_make_material_preview();
	context_raw->ddirty = 4;
}

void project_new_reset_layers(void *_) {
	layers_init();
}

void project_new_resize_layers(void *_) {
	layers_resize();
}

void project_new(bool reset_layers) {
	if (context_raw->paint_object != NULL) {
		project_cleanup();
		gc_unroot(project_filepath);
		project_filepath = "";
		gc_root(project_filepath);
	}

	if (project_layers->length == 0) {
		any_array_push(project_layers, slot_layer_create("", LAYER_SLOT_TYPE_LAYER, NULL));
		context_raw->layer = project_layers->buffer[0];
	}

	context_raw->layer_preview_dirty = true;
	context_raw->layer_filter        = 0;
	context_raw->texture             = NULL;
	gc_unroot(project_mesh_assets);
	project_mesh_assets = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_mesh_assets);

	mesh_data_t *raw       = NULL;
	char        *mesh_name = project_mesh_list == NULL ? "box_bevel" : project_mesh_list->buffer[context_raw->project_type];

	if (string_equals(mesh_name, "sphere")) {
		raw_mesh_t *mesh = geom_make_uv_sphere(1, 512, 256, true, 1.0);
		mesh->name       = "Tessellated";
		raw              = import_mesh_raw_mesh(mesh);
	}
	else if (string_equals(mesh_name, "plane")) {
		raw_mesh_t *mesh = geom_make_plane(1, 1, 512, 512, 1.0);
		mesh->name       = "Tessellated";
		raw              = import_mesh_raw_mesh(mesh);
		// viewport_set_view(0, 0, 0.75, 0, 0, 0); // Top
	}
	else {
		buffer_t *b = data_get_blob(string("meshes/%s.arm", mesh_name));
		gc_unroot(_project_scene_mesh_gc);
		_project_scene_mesh_gc = armpack_decode(b);
		gc_root(_project_scene_mesh_gc);
		raw = _project_scene_mesh_gc->mesh_datas->buffer[0];
	}

	mesh_data_t *md = mesh_data_create(raw);
	any_map_set(data_cached_meshes, "SceneTessellated", md);

	gpu_texture_t *current = _draw_current;
	bool           in_use  = gpu_in_use;
	if (in_use)
		draw_end();

	material_data_t *m = data_get_material("Scene", "Material");
	if (context_raw->paint_object == NULL) {
		context_raw->paint_object             = mesh_object_create(md, m);
		context_raw->paint_object->base->name = "paint_object";
	}
	else {
		mesh_object_set_data(context_raw->paint_object, md);
	}

	gc_unroot(project_paint_objects);
	project_paint_objects = any_array_create_from_raw(
	    (void *[]){
	        context_raw->paint_object,
	    },
	    1);
	gc_root(project_paint_objects);
	context_raw->paint_object = context_main_object();
	context_select_paint_object(context_main_object());

	context_raw->paint_object->base->transform->scale = vec4_create(1, 1, 1, 1.0);
	transform_build_matrix(context_raw->paint_object->base->transform);
	context_raw->paint_object->base->name = "Tessellated";

	while (project_materials->length > 0) {
		slot_material_unload(array_pop(project_materials));
	}
	any_array_push(project_materials, slot_material_create(m, NULL));

	context_raw->picker_mask_handle->i = PICKER_MASK_NONE;
	context_raw->material              = project_materials->buffer[0];
	ui_nodes_hwnd->redraws             = 2;
	gc_unroot(ui_nodes_group_stack);
	ui_nodes_group_stack = any_array_create_from_raw((void *[]){}, 0);
	gc_root(ui_nodes_group_stack);
	gc_unroot(project_material_groups);
	project_material_groups = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_material_groups);
	gc_unroot(project_brushes);
	project_brushes = any_array_create_from_raw(
	    (void *[]){
	        slot_brush_create(NULL),
	    },
	    1);
	gc_root(project_brushes);
	context_raw->brush = project_brushes->buffer[0];
	gc_unroot(project_fonts);
	project_fonts = any_array_create_from_raw(
	    (void *[]){
	        slot_font_create("default.ttf", base_font, ""),
	    },
	    1);
	gc_root(project_fonts);
	context_raw->font = project_fonts->buffer[0];
	project_set_default_swatches();
	context_raw->swatch                = project_raw->swatches->buffer[0];
	context_raw->picked_color          = project_make_swatch(0xffffffff);
	context_raw->color_picker_callback = NULL;
	history_reset();

	make_material_parse_paint_material(true);
	make_material_parse_brush();

	gc_unroot(project_assets);
	project_assets = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_assets);
	gc_unroot(project_asset_names);
	project_asset_names = any_array_create_from_raw((void *[]){}, 0);
	gc_root(project_asset_names);
	gc_unroot(project_asset_map);
	project_asset_map = any_imap_create();
	gc_root(project_asset_map);
	project_asset_id                                  = 0;
	project_raw->packed_assets                        = any_array_create_from_raw((void *[]){}, 0);
	context_raw->ddirty                               = 4;
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;

	if (reset_layers) {
		bool aspect_ratio_changed = project_layers->buffer[0]->texpaint->width != config_get_texture_res_x() ||
		                            project_layers->buffer[0]->texpaint->height != config_get_texture_res_y();
		while (project_layers->length > 0) {
			slot_layer_unload(array_pop(project_layers));
		}
		slot_layer_t *layer = slot_layer_create("", LAYER_SLOT_TYPE_LAYER, NULL);
		any_array_push(project_layers, layer);
		context_set_layer(layer);
		if (aspect_ratio_changed) {
			sys_notify_on_next_frame(&project_new_resize_layers, NULL);
		}
		sys_notify_on_next_frame(&project_new_reset_layers, NULL);
	}

	if (in_use)
		draw_begin(current, false, 0);

	project_set_default_envmap();
	context_init_tool();
	viewport_reset();
	viewport_scale_to_bounds(1.8);
	render_path_raytrace_ready = false;

	sys_notify_on_next_frame(&project_new_on_next_frame, NULL);
}

void project_set_default_envmap() {
	context_raw->saved_envmap          = NULL;
	context_raw->envmap_loaded         = false;
	scene_world->_->envmap             = context_raw->empty_envmap;
	scene_world->envmap                = "World_radiance.k";
	context_raw->show_envmap_handle->b = context_raw->show_envmap = false;
	scene_world->_->radiance                                      = context_raw->default_radiance;
	scene_world->_->radiance_mipmaps                              = context_raw->default_radiance_mipmaps;
	scene_world->_->irradiance                                    = context_raw->default_irradiance;
	scene_world->strength                                         = 2.0;
	context_raw->envmap_angle                                     = 0.0;
	context_raw->show_envmap_blur                                 = false;
	project_raw->envmap                                           = NULL;
}

void project_import_material_on_file_picked(char *path) {
	ends_with(path, ".blend") ? import_blend_material_run(path) : import_arm_run_material(path);
}

void project_import_material() {
	ui_files_show("arm,blend", false, true, &project_import_material_on_file_picked);
}

ui_node_link_t *project_create_node_link(ui_node_link_t_array_t *links, i32 from_id, i32 from_socket, i32 to_id, i32 to_socket) {
	ui_node_link_t *link =
	    GC_ALLOC_INIT(ui_node_link_t, {.id = ui_next_link_id(links), .from_id = from_id, .from_socket = from_socket, .to_id = to_id, .to_socket = to_socket});
	return link;
}

void project_import_brush_make_brush_preview(void *_) {
	util_render_make_brush_preview();
}

void project_import_brush_on_file_picked(char *path) {
	// Create brush from texture
	if (path_is_texture(path)) {
		// Import texture
		import_asset_run(path, -1.0, -1.0, true, true, NULL);
		i32 asset_index = 0;
		for (i32 i = 0; i < project_assets->length; ++i) {
			if (string_equals(project_assets->buffer[i]->file, path)) {
				asset_index = i;
				break;
			}
		}

		// Create a new brush
		context_raw->brush = slot_brush_create(NULL);
		any_array_push(project_brushes, context_raw->brush);

		// Create and link image node
		ui_node_t *n                         = nodes_brush_create_node("TEX_IMAGE");
		n->x                                 = 83;
		n->y                                 = 340;
		n->buttons->buffer[0]->default_value = f32_array_create_x(asset_index);
		ui_node_link_t_array_t *links        = context_raw->brush->canvas->links;
		ui_node_link_t         *link         = project_create_node_link(links, n->id, 0, 0, 4);
		any_array_push(links, link);

		// Parse brush
		make_material_parse_brush();
		ui_nodes_hwnd->redraws = 2;
		sys_notify_on_next_frame(&project_import_brush_make_brush_preview, NULL);
	}
	// Import from project file
	else {
		import_arm_run_brush(path);
	}
}

void project_import_brush() {
	char *formats = string_array_join(path_texture_formats(), ",");
	ui_files_show(string("arm,%s", formats), false, true, &project_import_brush_on_file_picked);
}

void project_import_mesh_on_file_picked(char *path) {
	project_import_mesh_box(path, _project_import_mesh_replace_existing, true, _project_import_mesh_done);
}

void project_import_mesh(bool replace_existing, void (*done)(void)) {
	_project_import_mesh_replace_existing = replace_existing;
	gc_unroot(_project_import_mesh_done);
	_project_import_mesh_done = done;
	gc_root(_project_import_mesh_done);
	char *formats = string_array_join(path_mesh_formats(), ",");
	if (string_index_of(formats, "fbx") == -1) {
		// Show .fbx in the file picker even when fbx plugin is not yet enabled
		formats = string("%s,fbx", formats);
	}
	if (string_index_of(formats, "gltf") == -1) {
		formats = string("%s,gltf", formats);
	}
	if (string_index_of(formats, "glb") == -1) {
		formats = string("%s,glb", formats);
	}
	ui_files_show(formats, false, false, &project_import_mesh_on_file_picked);
}

void project_append_mesh() {
	project_import_mesh(false, import_mesh_finish_import);
}

void project_import_mesh_box_draw() {
	char *path             = _project_import_mesh_box_path;
	bool  replace_existing = _project_import_mesh_box_replace_existing;
	bool  clear_layers     = _project_import_mesh_box_clear_layers;
	void (*done)(void)     = _project_import_mesh_box_done;

	if (ends_with(to_lower_case(path), ".obj")) {
		string_t_array_t *split_by_combo = any_array_create_from_raw(
		    (void *[]){
		        tr("Object", NULL),
		        tr("Group", NULL),
		        tr("Material", NULL),
		        tr("UDIM Tile", NULL),
		    },
		    4);
		context_raw->split_by = ui_combo(ui_handle(__ID__), split_by_combo, tr("Split By", NULL), true, UI_ALIGN_LEFT, true);
		if (ui->is_hovered) {
			ui_tooltip(tr("Split .obj mesh into objects", NULL));
		}
	}

	if (ends_with(to_lower_case(path), ".blend")) {
		import_blend_mesh_ui();
	}

	f32_array_t *row = f32_array_create_from_raw(
	    (f32[]){
	        0.45,
	        0.45,
	        0.1,
	    },
	    3);

	ui_row(row);
	if (ui_icon_button(tr("Cancel", NULL), ICON_CLOSE, UI_ALIGN_CENTER)) {
		ui_box_hide();
	}
	if (ui_icon_button(tr("Import", NULL), ICON_CHECK, UI_ALIGN_CENTER) || ui->is_return_down) {
		ui_box_hide();

#if defined(IRON_ANDROID) || defined(IRON_IOS)
		console_toast(tr("Importing mesh", NULL));
#endif

		import_mesh_run(path, clear_layers, replace_existing);
		if (done != NULL) {
			done();
		}
	}
	if (ui_button(tr("?", NULL), UI_ALIGN_CENTER, "")) {
		iron_load_url("https://github.com/armory3d/armorpaint_web/blob/main/manual.md#faq");
	}
}

void project_import_mesh_box(char *path, bool replace_existing, bool clear_layers, void (*done)(void)) {
	gc_unroot(_project_import_mesh_box_path);
	_project_import_mesh_box_path = string_copy(path);
	gc_root(_project_import_mesh_box_path);
	_project_import_mesh_box_replace_existing = replace_existing;
	_project_import_mesh_box_clear_layers     = clear_layers;
	gc_unroot(_project_import_mesh_box_done);
	_project_import_mesh_box_done = done;
	gc_root(_project_import_mesh_box_done);
	ui_box_show_custom(&project_import_mesh_box_draw, 400, 200, NULL, true, tr("Import Mesh", NULL));
	ui_box_click_to_hide = false; // Prevent closing when going back to window from file browser
}

void project_reimport_mesh() {
	if (project_mesh_assets != NULL && project_mesh_assets->length > 0 && iron_file_exists(project_mesh_assets->buffer[0])) {
		project_import_mesh_box(project_mesh_assets->buffer[0], true, false, NULL);
	}
	else {
		project_import_asset(NULL, true);
	}
}

string_t_array_t *project_get_unwrap_plugins() {
	string_t_array_t *unwrap_plugins = any_array_create_from_raw((void *[]){}, 0);
	if (box_preferences_files_plugin == NULL) {
		box_preferences_fetch_plugins();
	}
	for (i32 i = 0; i < box_preferences_files_plugin->length; ++i) {
		char *f = box_preferences_files_plugin->buffer[i];
		if (string_index_of(f, "uv_unwrap") >= 0 && ends_with(f, ".js")) {
			any_array_push(unwrap_plugins, f);
		}
	}
	any_array_push(unwrap_plugins, "equirect");
	return unwrap_plugins;
}

void project_unwrap_mesh_box_draw() {
	raw_mesh_t *mesh           = _project_unwrap_mesh_box_mesh;
	void (*done)(raw_mesh_t *) = _project_unwrap_mesh_box_done;

	string_t_array_t *unwrap_plugins = project_get_unwrap_plugins();
	_project_unwrap_by               = ui_combo(ui_handle(__ID__), unwrap_plugins, tr("Plugin", NULL), true, UI_ALIGN_LEFT, true);

	ui_row2();
	if (ui_icon_button(tr("Cancel", NULL), ICON_CLOSE, UI_ALIGN_CENTER)) {
		ui_box_hide();
	}
	if (ui_icon_button(tr("Unwrap", NULL), ICON_CHECK, UI_ALIGN_CENTER) || ui->is_return_down) {
		ui_box_hide();

#if defined(IRON_ANDROID) || defined(IRON_IOS)
		console_toast(tr("Unwrapping mesh", NULL));
#endif

		project_unwrap_mesh(mesh, done);
	}
}

void project_unwrap_mesh_box(raw_mesh_t *mesh, void (*done)(raw_mesh_t *), bool skip_ui) {
	gc_unroot(_project_unwrap_mesh_box_mesh);
	_project_unwrap_mesh_box_mesh = mesh;
	gc_root(_project_unwrap_mesh_box_mesh);
	gc_unroot(_project_unwrap_mesh_box_done);
	_project_unwrap_mesh_box_done = done;
	gc_root(_project_unwrap_mesh_box_done);

	if (skip_ui) {
		project_unwrap_mesh(mesh, done);
		return;
	}

	ui_box_show_custom(&project_unwrap_mesh_box_draw, 400, 200, NULL, true, tr("Unwrap Mesh", NULL));
}

void project_unwrap_mesh(raw_mesh_t *mesh, void (*done)(raw_mesh_t *)) {
	string_t_array_t *unwrap_plugins = project_get_unwrap_plugins();

	if (_project_unwrap_by == unwrap_plugins->length - 1) {
		util_mesh_equirect_unwrap(mesh);
	}
	else {
		char *f = unwrap_plugins->buffer[_project_unwrap_by];
		if (string_array_index_of(config_raw->plugins, f) == -1) {
			config_enable_plugin(f);
			console_info(string("%s %s", f, tr("plugin enabled", NULL)));
		}
		void *cb = any_map_get(util_mesh_unwrappers, f); // JSValue * -> (a: raw_mesh_t)=>void
		js_call_ptr(cb, mesh);
	}
	done(mesh);
}

void project_import_asset_on_file_picked(char *path) {
	import_asset_run(path, -1.0, -1.0, true, _project_import_asset_hdr_as_envmap, NULL);
}

void project_import_asset(char *filters, bool hdr_as_envmap) {
	if (filters == NULL) {
		filters = string("%s,%s", string_array_join(path_texture_formats(), ","), string_array_join(path_mesh_formats(), ","));
	}

	_project_import_asset_hdr_as_envmap = hdr_as_envmap;

	ui_files_show(filters, false, true, &project_import_asset_on_file_picked);
}

void project_import_swatches_on_file_picked(char *path) {
	if (path_is_gimp_color_palette(path)) {
		// import_gpl_run(path, _project_import_swatches_replace_existing);
	}
	else {
		import_arm_run_swatches(path, _project_import_swatches_replace_existing);
	}
}

void project_import_swatches(bool replace_existing) {
	_project_import_swatches_replace_existing = replace_existing;
	ui_files_show("arm,gpl", false, false, &project_import_swatches_on_file_picked);
}

void project_reimport_textures() {
	for (i32 i = 0; i < project_assets->length; ++i) {
		asset_t *asset = project_assets->buffer[i];
		project_reimport_texture(asset);
	}
}

void project_reimport_texture_load_on_next_frame(void *_) {
	make_material_parse_paint_material(true);
	util_render_make_material_preview();
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
}

void project_reimport_texture_load(char *path, asset_t *asset) {
	asset->file = string_copy(path);
	i32 i       = array_index_of(project_assets, asset);
	data_delete_image(asset->file);
	imap_delete(project_asset_map, asset->id);
	asset_t *old_asset = project_assets->buffer[i];
	array_splice(project_assets, i, 1);
	array_splice(project_asset_names, i, 1);
	import_texture_run(asset->file, true);
	array_insert(project_assets, i, array_pop(project_assets));
	array_insert(project_asset_names, i, array_pop(project_asset_names));

	if (context_raw->texture == old_asset) {
		context_raw->texture = project_assets->buffer[i];
	}

	sys_notify_on_next_frame(&project_reimport_texture_load_on_next_frame, NULL);
}

void project_reimport_texture_on_file_picked(char *path) {
	project_reimport_texture_load(path, _project_reimport_texture_asset);
}

void project_reimport_texture(asset_t *asset) {
	if (!iron_file_exists(asset->file)) {
		char *filters = string_array_join(path_texture_formats(), ",");
		gc_unroot(_project_reimport_texture_asset);
		_project_reimport_texture_asset = asset;
		gc_root(_project_reimport_texture_asset);
		ui_files_show(filters, false, false, &project_reimport_texture_on_file_picked);
	}
	else {
		project_reimport_texture_load(asset->file, asset);
	}
}

gpu_texture_t *project_get_image(asset_t *asset) {
	return asset != NULL ? any_imap_get(project_asset_map, asset->id) : NULL;
}

string_t_array_t *project_get_used_atlases() {
	if (project_atlas_objects == NULL) {
		return NULL;
	}
	i32_array_t *used = i32_array_create_from_raw((i32[]){}, 0);
	for (i32 i = 0; i < project_atlas_objects->length; ++i) {
		i32 ao = project_atlas_objects->buffer[i];
		if (i32_array_index_of(used, ao) == -1) {
			i32_array_push(used, ao);
		}
	}
	if (used->length > 1) {
		string_t_array_t *res = any_array_create_from_raw((void *[]){}, 0);
		for (i32 i = 0; i < used->length; ++i) {
			i32 u = used->buffer[i];
			any_array_push(res, project_atlas_names->buffer[u]);
		}
		return res;
	}
	else
		return NULL;
}

bool project_is_atlas_object(mesh_object_t *p) {
	if (context_raw->layer_filter <= project_paint_objects->length) {
		return false;
	}
	char *atlas_name = project_get_used_atlases()->buffer[context_raw->layer_filter - project_paint_objects->length - 1];
	i32   atlas_i    = string_array_index_of(project_atlas_names, atlas_name);
	return atlas_i == project_atlas_objects->buffer[array_index_of(project_paint_objects, p)];
}

mesh_object_t_array_t *project_get_atlas_objects(i32 object_mask) {
	string_t_array_t *atlases = project_get_used_atlases();
	i32               i       = object_mask - project_paint_objects->length - 1;
	if (atlases == NULL || i >= atlases->length) {
		return project_paint_objects;
	}
	char                  *atlas_name = atlases->buffer[i];
	i32                    atlas_i    = string_array_index_of(project_atlas_names, atlas_name);
	mesh_object_t_array_t *visibles   = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		if (project_atlas_objects->buffer[i] == atlas_i) {
			any_array_push(visibles, project_paint_objects->buffer[i]);
		}
	}
	return visibles;
}

bool project_packed_asset_exists(packed_asset_t_array_t *packed_assets, char *name) {
	for (i32 i = 0; i < packed_assets->length; ++i) {
		packed_asset_t *pa = packed_assets->buffer[i];
		if (string_equals(pa->name, name)) {
			return true;
		}
	}
	return false;
}

void project_export_swatches_on_file_picked(char *path) {
	char *f = ui_files_filename;
	if (string_equals(f, "")) {
		f = string_copy(tr("untitled", NULL));
	}
	if (path_is_gimp_color_palette(f)) {
		// export_gpl_run(path + PATH_SEP + f, substring(f, 0, string_last_index_of(f, ".")), project_raw.swatches);
	}
	else {
		export_arm_run_swatches(string("%s%s%s", path, PATH_SEP, f));
	}
}

void project_export_swatches() {
	ui_files_show("arm,gpl", true, false, &project_export_swatches_on_file_picked);
}

swatch_color_t *project_make_swatch(i32 base) {
	swatch_color_t *s = GC_ALLOC_INIT(swatch_color_t, {.base       = base,
	                                                   .opacity    = 1.0,
	                                                   .occlusion  = 1.0,
	                                                   .roughness  = 0.0,
	                                                   .metallic   = 0.0,
	                                                   .normal     = 0xff8080ff,
	                                                   .emission   = 0.0,
	                                                   .height     = 0.0,
	                                                   .subsurface = 0.0});
	return s;
}

swatch_color_t *project_clone_swatch(swatch_color_t *swatch) {
	swatch_color_t *s = GC_ALLOC_INIT(swatch_color_t, {.base       = swatch->base,
	                                                   .opacity    = swatch->opacity,
	                                                   .occlusion  = swatch->occlusion,
	                                                   .roughness  = swatch->roughness,
	                                                   .metallic   = swatch->metallic,
	                                                   .normal     = swatch->normal,
	                                                   .emission   = swatch->emission,
	                                                   .height     = swatch->height,
	                                                   .subsurface = swatch->subsurface});
	return s;
}

void project_set_default_swatches() {
	// 32-Color Palette by Andrew Kensler
	// http://eastfarthing.com/blog/2016-05-06-palette/
	project_raw->swatches = any_array_create_from_raw((void *[]){}, 0);
	i32_array_t *colors   = i32_array_create_from_raw(
        (i32[]){
            0xffffffff, 0xff000000, 0xffd6a090, 0xffa12c32, 0xfffa2f7a, 0xfffb9fda, 0xffe61cf7, 0xff992f7c, 0xff47011f, 0xff051155, 0xff4f02ec,
            0xff2d69cb, 0xff00a6ee, 0xff6febff, 0xff08a29a, 0xff2a666a, 0xff063619, 0xff4a4957, 0xff8e7ba4, 0xffb7c0ff, 0xffacbe9c, 0xff827c70,
            0xff5a3b1c, 0xffae6507, 0xfff7aa30, 0xfff4ea5c, 0xff9b9500, 0xff566204, 0xff11963b, 0xff51e113, 0xff08fdcc,
        },
        31);
	for (i32 i = 0; i < colors->length; ++i) {
		i32 c = colors->buffer[i];
		any_array_push(project_raw->swatches, project_make_swatch(c));
	}
}

node_group_t *project_get_material_group_by_name(char *group_name) {
	for (i32 i = 0; i < project_material_groups->length; ++i) {
		node_group_t *g = project_material_groups->buffer[i];
		if (string_equals(g->canvas->name, group_name)) {
			return g;
		}
	}
	return NULL;
}

bool project_is_material_group_in_use(node_group_t *group) {
	ui_node_canvas_t_array_t *canvases = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < project_materials->length; ++i) {
		slot_material_t *m = project_materials->buffer[i];
		any_array_push(canvases, m->canvas);
	}
	for (i32 i = 0; i < project_material_groups->length; ++i) {
		node_group_t *m = project_material_groups->buffer[i];
		any_array_push(canvases, m->canvas);
	}
	for (i32 i = 0; i < canvases->length; ++i) {
		ui_node_canvas_t *canvas = canvases->buffer[i];
		for (i32 i = 0; i < canvas->nodes->length; ++i) {
			ui_node_t *n     = canvas->nodes->buffer[i];
			char      *cname = group->canvas->name;
			if (string_equals(n->type, "GROUP") && string_equals(n->name, cname)) {
				return true;
			}
		}
	}
	return false;
}
