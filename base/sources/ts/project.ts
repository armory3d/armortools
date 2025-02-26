
let project_raw: project_format_t = {};
let project_filepath: string = "";
let project_assets: asset_t[] = [];
let project_asset_names: string[] = [];
let project_asset_id: i32 = 0;
let project_mesh_assets: string[] = [];
let project_material_groups: node_group_t[] = [];
let project_paint_objects: mesh_object_t[] = null;
let project_asset_map: map_t<i32, any> = map_create(); // image_t | font_t
let project_mesh_list: string[] = null;

let project_materials: slot_material_t[] = null;
let project_brushes: slot_brush_t[] = null;
let project_layers: slot_layer_t[] = null;
let project_fonts: slot_font_t[] = null;
let project_atlas_objects: i32[] = null;
let project_atlas_names: string[] = null;

// lab
let project_material_data: material_data_t = null;
let project_nodes: ui_nodes_t;
let project_canvas: ui_node_canvas_t;
let project_default_canvas: buffer_t = null;

let _project_save_and_quit: bool;
let _project_import_mesh_replace_existing: bool;
let _project_import_mesh_done: ()=>void;
let _project_import_mesh_box_path: string;
let _project_import_mesh_box_replace_existing: bool;
let _project_import_mesh_box_clear_layers: bool;
let _project_import_mesh_box_done: ()=>void;
let _project_unwrap_mesh_box_mesh: raw_mesh_t;
let _project_unwrap_mesh_box_done: (a: raw_mesh_t)=>void;
let _project_unwrap_mesh_box_skip_ui: bool;
let _project_import_asset_hdr_as_envmap: bool;
let _project_import_swatches_replace_existing: bool;
let _project_reimport_texture_asset: asset_t;
let _project_scene_mesh_gc: scene_t;

function project_open() {
	ui_files_show("arm", false, false, function (path: string) {
		if (!ends_with(path, ".arm")) {
			console_error(strings_arm_file_expected());
			return;
		}

		let current: image_t = _g2_current;
		let g2_in_use: bool = _g2_in_use;
		if (g2_in_use) g2_end();

		import_arm_run_project(path);

		if (g2_in_use) g2_begin(current);
	});
}

function project_save(save_and_quit: bool = false) {
	if (project_filepath == "") {
		///if arm_ios
		let document_directory: string = iron_save_dialog("", "");
		document_directory = substring(document_directory, 0, document_directory.length - 8); // Strip /"untitled"
		project_filepath = document_directory + "/" + sys_title() + ".arm";
		///elseif arm_android
		project_filepath = iron_save_path() + "/" + sys_title() + ".arm";
		///else
		project_save_as(save_and_quit);
		return;
		///end
	}

	///if (arm_windows || arm_linux || arm_macos)
	let filename: string = substring(project_filepath, string_last_index_of(project_filepath, path_sep) + 1, project_filepath.length - 4);
	sys_title_set(filename + " - " + manifest_title);
	///end

	_project_save_and_quit = save_and_quit;

	app_notify_on_init(function () {
		export_arm_run_project();
		if (_project_save_and_quit) {
			sys_stop();
		}
	});
}

function project_save_as(save_and_quit: bool = false) {
	_project_save_and_quit = save_and_quit;
	ui_files_show("arm", true, false, function (path: string) {
		let f: string = ui_files_filename;
		if (f == "") {
			f = tr("untitled");
		}
		project_filepath = path + path_sep + f;
		if (!ends_with(project_filepath, ".arm")) {
			project_filepath += ".arm";
		}
		project_save(_project_save_and_quit);
	});
}

function project_new_box() {
	///if (is_paint || is_sculpt)
	ui_box_show_custom(function (ui: ui_t) {
		if (ui_tab(ui_handle(__ID__), tr("New Project"))) {
			if (project_mesh_list == null) {
				project_mesh_list = file_read_directory(path_data() + path_sep + "meshes");
				for (let i: i32 = 0; i < project_mesh_list.length; ++i) {
					let s: string = project_mesh_list[i];
					project_mesh_list[i] = substring(project_mesh_list[i], 0, s.length - 4); // Trim .arm
				}
				array_insert(project_mesh_list, 0, "plane");
				array_insert(project_mesh_list, 0, "sphere");
				array_insert(project_mesh_list, 0, "rounded_cube");
			}

			ui_row2();
			let h_project_type: ui_handle_t = ui_handle(__ID__);
			if (h_project_type.init) {
				h_project_type.position = context_raw.project_type;
			}
			context_raw.project_type = ui_combo(h_project_type, project_mesh_list, tr("Template"), true);

			let h_project_aspect_ratio: ui_handle_t = ui_handle(__ID__);
			if (h_project_aspect_ratio.init) {
				h_project_aspect_ratio.position = context_raw.project_aspect_ratio;
			}
			let project_aspect_ratio_combo: string[] = ["1:1", "2:1", "1:2"];
			context_raw.project_aspect_ratio = ui_combo(h_project_aspect_ratio, project_aspect_ratio_combo, tr("Aspect Ratio"), true);

			_ui_end_element();
			ui_row2();
			if (ui_button(tr("Cancel"))) {
				ui_box_hide();
			}
			if (ui_button(tr("OK")) || ui.is_return_down) {
				project_new();
				viewport_scale_to_bounds();
				ui_box_hide();
			}
		}
	});
	///end

	///if is_lab
	project_new();
	viewport_scale_to_bounds();
	///end
}

function project_new(reset_layers: bool = true) {
	///if (arm_windows || arm_linux || arm_macos)
	sys_title_set(manifest_title);
	///end
	project_filepath = "";

	if (context_raw.merged_object != null) {
		mesh_object_remove(context_raw.merged_object);
		data_delete_mesh(context_raw.merged_object.data._.handle);
		context_raw.merged_object = null;
	}
	context_raw.layer_preview_dirty = true;
	context_raw.layer_filter = 0;
	context_raw.texture = null;
	project_mesh_assets = [];

	viewport_reset();
	context_raw.paint_object = context_main_object();

	context_select_paint_object(context_main_object());
	for (let i: i32 = 1; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		if (p == context_raw.paint_object) {
			continue;
		}
		data_delete_mesh(p.data._.handle);
		mesh_object_remove(p);
	}
	let meshes: mesh_object_t[] = scene_meshes;
	let len: i32 = meshes.length;
	for (let i: i32 = 0; i < len; ++i) {
		let m: mesh_object_t = meshes[len - i - 1];
		if (array_index_of(context_raw.project_objects, m) == -1 &&
			m.base.name != ".ParticleEmitter" &&
			m.base.name != ".Particle") {
			data_delete_mesh(m.data._.handle);
			mesh_object_remove(m);
		}
	}
	let handle: string = context_raw.paint_object.data._.handle;
	if (handle != "SceneSphere" && handle != "ScenePlane") {
		data_delete_mesh(handle);
	}

	if (context_raw.project_type != project_model_t.ROUNDED_CUBE) {
		let raw: mesh_data_t = null;
		if (context_raw.project_type == project_model_t.SPHERE || context_raw.project_type == project_model_t.TESSELLATED_PLANE) {
			let mesh: raw_mesh_t = context_raw.project_type == project_model_t.SPHERE ?
				geom_make_uv_sphere(1, 512, 256) :
				geom_make_plane(1, 1, 512, 512);
			mesh.name = "Tessellated";
			raw = import_mesh_raw_mesh(mesh);

			///if is_sculpt
			app_notify_on_next_frame(function (mesh: raw_mesh_t) {
				app_notify_on_init(function (mesh: raw_mesh_t) {
					import_mesh_pack_to_texture(mesh);
				}, mesh);
			}, mesh);
			///end
		}
		else {
			let b: buffer_t = data_get_blob("meshes/" + project_mesh_list[context_raw.project_type] + ".arm");
			_project_scene_mesh_gc = armpack_decode(b);
			raw = _project_scene_mesh_gc.mesh_datas[0];
		}

		let md: mesh_data_t = mesh_data_create(raw);
		map_set(data_cached_meshes, "SceneTessellated", md);

		if (context_raw.project_type == project_model_t.TESSELLATED_PLANE) {
			viewport_set_view(0, 0, 0.75, 0, 0, 0); // Top
		}
	}

	let n: string = context_raw.project_type == project_model_t.ROUNDED_CUBE ? ".Cube" : "Tessellated";
	let md: mesh_data_t = data_get_mesh("Scene", n);

	let current: image_t = _g2_current;
	let g2_in_use: bool = _g2_in_use;
	if (g2_in_use) g2_end();

	///if is_paint
	context_raw.picker_mask_handle.position = picker_mask_t.NONE;
	///end

	mesh_object_set_data(context_raw.paint_object, md);
	context_raw.paint_object.base.transform.scale = vec4_create(1, 1, 1);
	transform_build_matrix(context_raw.paint_object.base.transform);
	context_raw.paint_object.base.name = n;
	project_paint_objects = [context_raw.paint_object];
	///if (is_paint || is_sculpt)
	while (project_materials.length > 0) {
		slot_material_unload(array_pop(project_materials));
	}
	///end
	let m: material_data_t = data_get_material("Scene", "Material");
	///if (is_paint || is_sculpt)
	array_push(project_materials, slot_material_create(m));
	///end
	///if is_lab
	project_material_data = m;
	///end

	///if (is_paint || is_sculpt)
	context_raw.material = project_materials[0];
	///end

	ui_nodes_hwnd.redraws = 2;
	ui_nodes_group_stack = [];
	project_material_groups = [];

	///if (is_paint || is_sculpt)
	project_brushes = [slot_brush_create()];
	context_raw.brush = project_brushes[0];

	project_fonts = [slot_font_create("default.ttf", base_font)];
	context_raw.font = project_fonts[0];
	///end

	project_set_default_swatches();
	context_raw.swatch = project_raw.swatches[0];

	context_raw.picked_color = make_swatch();
	context_raw.color_picker_callback = null;
	history_reset();

	make_material_parse_paint_material();
	make_material_parse_brush();

	///if (is_paint || is_sculpt)
	util_render_make_material_preview();
	///end

	for (let i: i32 = 0; i < project_assets.length; ++i) {
		let a: asset_t = project_assets[i];
		data_delete_image(a.file);
	}
	project_assets = [];
	project_asset_names = [];
	project_asset_map = map_create();
	project_asset_id = 0;
	project_raw.packed_assets = [];
	context_raw.ddirty = 4;

	///if (is_paint || is_sculpt)
	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	///end

	if (reset_layers) {

		///if (is_paint || is_sculpt)
		let aspect_ratio_changed: bool = project_layers[0].texpaint.width != config_get_texture_res_x() || project_layers[0].texpaint.height != config_get_texture_res_y();
		while (project_layers.length > 0) {
			slot_layer_unload(array_pop(project_layers));
		}
		let layer: slot_layer_t = slot_layer_create();
		array_push(project_layers, layer);
		context_set_layer(layer);
		if (aspect_ratio_changed) {
			app_notify_on_init(layers_resize);
		}
		///end

		app_notify_on_init(layers_init);
	}

	if (g2_in_use) g2_begin(current);

	context_raw.saved_envmap = null;
	context_raw.envmap_loaded = false;
	scene_world._.envmap = context_raw.empty_envmap;
	scene_world.envmap = "World_radiance.k";
	context_raw.show_envmap_handle.selected = context_raw.show_envmap = false;
	scene_world._.radiance = context_raw.default_radiance;
	scene_world._.radiance_mipmaps = context_raw.default_radiance_mipmaps;
	scene_world._.irradiance = context_raw.default_irradiance;
	scene_world.strength = 4.0;

	///if (is_paint || is_sculpt)
	context_init_tool();
	///end

	render_path_raytrace_ready = false;
}

function project_import_material() {
	ui_files_show("arm,blend", false, true, function (path: string) {
		ends_with(path, ".blend") ?
			import_blend_material_run(path) :
			import_arm_run_material(path);
	});
}

function project_create_node_link(links: ui_node_link_t[], from_id: i32, from_socket: i32, to_id: i32, to_socket: i32): ui_node_link_t {
	let link: ui_node_link_t = {
		id: ui_next_link_id(links),
		from_id: from_id,
		from_socket: from_socket,
		to_id: to_id,
		to_socket: to_socket
	};
	return link;
}

function project_import_brush() {
	let formats: string = string_array_join(path_texture_formats, ",");
	ui_files_show("arm," + formats, false, true, function (path: string) {
		// Create brush from texture
		if (path_is_texture(path)) {
			// Import texture
			import_asset_run(path);
			let asset_index: i32 = 0;
			for (let i: i32 = 0; i < project_assets.length; ++i) {
				if (project_assets[i].file == path) {
					asset_index = i;
					break;
				}
			}

			// Create a new brush
			context_raw.brush = slot_brush_create();
			array_push(project_brushes, context_raw.brush);

			// Create and link image node
			let n: ui_node_t = nodes_brush_create_node("TEX_IMAGE");
			n.x = 83;
			n.y = 340;
			n.buttons[0].default_value = f32_array_create_x(asset_index);
			let links: ui_node_link_t[] = context_raw.brush.canvas.links;
			let link: ui_node_link_t = project_create_node_link(links, n.id, 0, 0, 4);
			array_push(links, link);

			// Parse brush
			make_material_parse_brush();
			ui_nodes_hwnd.redraws = 2;
			app_notify_on_init(util_render_make_brush_preview);
		}
		// Import from project file
		else {
			import_arm_run_brush(path);
		}
	});
}

function project_import_mesh(replace_existing: bool = true, done: ()=>void = null) {
	_project_import_mesh_replace_existing = replace_existing;
	_project_import_mesh_done = done;
	let formats: string = string_array_join(path_mesh_formats, ",");
	if (string_index_of(formats, "fbx") == -1) {
		// Show .fbx in the file picker even when fbx plugin is not yet enabled
		formats += ",fbx";
	}
	ui_files_show(formats, false, false, function (path: string) {
		project_import_mesh_box(path, _project_import_mesh_replace_existing, true, _project_import_mesh_done);
	});
}

function project_append_mesh() {
	project_import_mesh(false, import_mesh_finish_import);
}

function project_import_mesh_box(path: string, replace_existing: bool = true, clear_layers: bool = true, done: ()=>void = null) {

	_project_import_mesh_box_path = path;
	_project_import_mesh_box_replace_existing = replace_existing;
	_project_import_mesh_box_clear_layers = clear_layers;
	_project_import_mesh_box_done = done;

	///if arm_ios
	// Import immediately while access to resource is unlocked
	// data_get_blob(path);
	///end

	ui_box_show_custom(function (ui: ui_t) {

		let path: string = _project_import_mesh_box_path;
		let replace_existing: bool = _project_import_mesh_box_replace_existing;
		let clear_layers: bool = _project_import_mesh_box_clear_layers;
		let done: ()=>void = _project_import_mesh_box_done;

		let tab_vertical: bool = config_raw.touch_ui;
		if (ui_tab(ui_handle(__ID__), tr("Import Mesh"), tab_vertical)) {

			if (ends_with(to_lower_case(path), ".obj")) {
				let split_by_combo: string[] = [tr("Object"), tr("Group"), tr("Material"), tr("UDIM Tile")];
				context_raw.split_by = ui_combo(ui_handle(__ID__), split_by_combo, tr("Split By"), true);
				if (ui.is_hovered) {
					ui_tooltip(tr("Split .obj mesh into objects"));
				}
			}

			///if (is_paint || is_sculpt)
			if (ends_with(to_lower_case(path), ".fbx")) {
				let h: ui_handle_t = ui_handle(__ID__);
				h.selected = context_raw.parse_vcols;
				context_raw.parse_vcols = ui_check(h, tr("Parse Vertex Colors"));
				if (ui.is_hovered) {
					ui_tooltip(tr("Import vertex color data"));
				}
			}

			if (ends_with(to_lower_case(path), ".blend")) {
				import_blend_mesh_ui();
			}
			///end

			let row: f32 [] = [0.45, 0.45, 0.1];
			ui_row(row);
			if (ui_button(tr("Cancel"))) {
				ui_box_hide();
			}
			if (ui_button(tr("Import")) || ui.is_return_down) {
				ui_box_hide();

				///if (arm_android || arm_ios)
				console_toast(tr("Importing mesh"));
				///end

				///if (is_paint || is_sculpt)
				import_mesh_run(path, clear_layers, replace_existing);
				///end
				///if is_lab
				import_mesh_run(path, replace_existing);
				///end
				if (done != null) {
					done();
				}
			}
			if (ui_button(tr("?"))) {
				file_load_url("https://github.com/armory3d/armorpaint_web/blob/main/manual.md#faq");
			}
		}
	});

	ui_box_click_to_hide = false; // Prevent closing when going back to window from file browser
}

function project_reimport_mesh() {
	if (project_mesh_assets != null && project_mesh_assets.length > 0 && file_exists(project_mesh_assets[0])) {
		project_import_mesh_box(project_mesh_assets[0], true, false);
	}
	else {
		project_import_asset();
	}
}

let _project_unwrap_by: i32 = 0;

function project_get_unwrap_plugins(): string[] {
	let unwrap_plugins: string[] = [];
	if (box_preferences_files_plugin == null) {
		box_preferences_fetch_plugins();
	}
	for (let i: i32 = 0; i < box_preferences_files_plugin.length; ++i) {
		let f: string = box_preferences_files_plugin[i];
		if (string_index_of(f, "uv_unwrap") >= 0 && ends_with(f, ".js")) {
			array_push(unwrap_plugins, f);
		}
	}
	array_push(unwrap_plugins, "equirect");
	return unwrap_plugins;
}

function project_unwrap_mesh_box(mesh: raw_mesh_t, done: (a: raw_mesh_t)=>void, skip_ui: bool = false) {

	_project_unwrap_mesh_box_mesh = mesh;
	_project_unwrap_mesh_box_done = done;

	if (skip_ui) {
		project_unwrap_mesh(mesh, done);
		return;
	}

	ui_box_show_custom(function (ui: ui_t) {
		let mesh: raw_mesh_t = _project_unwrap_mesh_box_mesh;
		let done: (a: raw_mesh_t)=>void = _project_unwrap_mesh_box_done;

		let tab_vertical: bool = config_raw.touch_ui;
		if (ui_tab(ui_handle(__ID__), tr("Unwrap Mesh"), tab_vertical)) {

			let unwrap_plugins: string[] = project_get_unwrap_plugins();
			_project_unwrap_by = ui_combo(ui_handle(__ID__), unwrap_plugins, tr("Plugin"), true);

			ui_row2();
			if (ui_button(tr("Cancel"))) {
				ui_box_hide();
			}
			if (ui_button(tr("Unwrap")) || ui.is_return_down) {
				ui_box_hide();

				///if (arm_android || arm_ios)
				console_toast(tr("Unwrapping mesh"));
				///end

				project_unwrap_mesh(mesh, done);
			}
		}
	});
}

function project_unwrap_mesh(mesh: raw_mesh_t, done: (a: raw_mesh_t)=>void) {
	let unwrap_plugins: string[] = project_get_unwrap_plugins();

	if (_project_unwrap_by == unwrap_plugins.length - 1) {
		util_mesh_equirect_unwrap(mesh);
	}
	else {
		let f: string = unwrap_plugins[_project_unwrap_by];
		if (array_index_of(config_raw.plugins, f) == -1) {
			config_enable_plugin(f);
			console_info(f + " " + tr("plugin enabled"));
		}
		let cb: any = map_get(util_mesh_unwrappers, f); // JSValue * -> (a: raw_mesh_t)=>void
		js_call_ptr(cb, mesh);
	}
	done(mesh);
}

function project_import_asset(filters: string = null, hdr_as_envmap: bool = true) {
	if (filters == null) {
		filters = string_array_join(path_texture_formats, ",") + "," + string_array_join(path_mesh_formats, ",");
	}

	_project_import_asset_hdr_as_envmap = hdr_as_envmap;

	ui_files_show(filters, false, true, function (path: string) {
		import_asset_run(path, -1.0, -1.0, true, _project_import_asset_hdr_as_envmap);
	});
}

function project_import_swatches(replace_existing: bool = false) {
	_project_import_swatches_replace_existing = replace_existing;
	ui_files_show("arm,gpl", false, false, function (path: string) {
		if (path_is_gimp_color_palette(path)) {
			// import_gpl_run(path, _project_import_swatches_replace_existing);
		}
		else {
			import_arm_run_swatches(path, _project_import_swatches_replace_existing);
		}
	});
}

function project_reimport_textures() {
	for (let i: i32 = 0; i < project_assets.length; ++i) {
		let asset: asset_t = project_assets[i];
		project_reimport_texture(asset);
	}
}

function project_reimport_texture_load(path: string, asset: asset_t) {
	asset.file = path;
	let i: i32 = array_index_of(project_assets, asset);
	data_delete_image(asset.file);
	map_delete(project_asset_map, asset.id);
	let old_asset: asset_t = project_assets[i];
	array_splice(project_assets, i, 1);
	array_splice(project_asset_names, i, 1);
	import_texture_run(asset.file);
	array_insert(project_assets, i, array_pop(project_assets));
	array_insert(project_asset_names, i, array_pop(project_asset_names));

	///if (is_paint || is_sculpt)
	if (context_raw.texture == old_asset) {
		context_raw.texture = project_assets[i];
	}
	///end

	app_notify_on_next_frame(function () {
		make_material_parse_paint_material();

		///if (is_paint || is_sculpt)
		util_render_make_material_preview();
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		///end
	});
}

function project_reimport_texture(asset: asset_t) {
	if (!file_exists(asset.file)) {
		let filters: string = string_array_join(path_texture_formats, ",");
		_project_reimport_texture_asset = asset;
		ui_files_show(filters, false, false, function (path: string) {
			project_reimport_texture_load(path, _project_reimport_texture_asset);
		});
	}
	else {
		project_reimport_texture_load(asset.file, asset);
	}
}

function project_get_image(asset: asset_t): image_t {
	return asset != null ? map_get(project_asset_map, asset.id) : null;
}

function project_get_used_atlases(): string[] {
	if (project_atlas_objects == null) {
		return null;
	}
	let used: i32[] = [];
	for (let i: i32 = 0; i < project_atlas_objects.length; ++i) {
		let ao: i32 = project_atlas_objects[i];
		if (array_index_of(used, ao) == -1) {
			array_push(used, ao);
		}
	}
	if (used.length > 1) {
		let res: string[] = [];
		for (let i: i32 = 0; i < used.length; ++i) {
			let u: i32 = used[i];
			array_push(res, project_atlas_names[u]);
		}
		return res;
	}
	else return null;
}

function project_is_atlas_object(p: mesh_object_t): bool {
	if (context_raw.layer_filter <= project_paint_objects.length) {
		return false;
	}
	let atlas_name: string = project_get_used_atlases()[context_raw.layer_filter - project_paint_objects.length - 1];
	let atlas_i: i32 = array_index_of(project_atlas_names, atlas_name);
	return atlas_i == project_atlas_objects[array_index_of(project_paint_objects, p)];
}

function project_get_atlas_objects(object_mask: i32): mesh_object_t[] {
	let atlas_name: string = project_get_used_atlases()[object_mask - project_paint_objects.length - 1];
	let atlas_i: i32 = array_index_of(project_atlas_names, atlas_name);
	let visibles: mesh_object_t[] = [];
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		if (project_atlas_objects[i] == atlas_i) {
			array_push(visibles, project_paint_objects[i]);
		}
	}
	return visibles;
}

function project_packed_asset_exists(packed_assets: packed_asset_t[], name: string): bool {
	for (let i: i32 = 0; i < packed_assets.length; ++i) {
		let pa: packed_asset_t = packed_assets[i];
		if (pa.name == name) {
			return true;
		}
	}
	return false;
}

function project_export_swatches() {
	ui_files_show("arm,gpl", true, false, function (path: string) {
		let f: string = ui_files_filename;
		if (f == "") {
			f = tr("untitled");
		}
		if (path_is_gimp_color_palette(f)) {
			// export_gpl_run(path + path_sep + f, substring(f, 0, string_last_index_of(f, ".")), project_raw.swatches);
		}
		else {
			export_arm_run_swatches(path + path_sep + f);
		}
	});
}

function make_swatch(base: i32 = 0xffffffff): swatch_color_t {
	let s: swatch_color_t = {
		base: base,
		opacity: 1.0,
		occlusion: 1.0,
		roughness: 0.0,
		metallic: 0.0,
		normal: 0xff8080ff,
		emission: 0.0,
		height: 0.0,
		subsurface: 0.0
	};
	return s;
}

function project_clone_swatch(swatch: swatch_color_t): swatch_color_t {
	let s: swatch_color_t = {
		base: swatch.base,
		opacity: swatch.opacity,
		occlusion: swatch.occlusion,
		roughness: swatch.roughness,
		metallic: swatch.metallic,
		normal: swatch.normal,
		emission: swatch.emission,
		height: swatch.height,
		subsurface: swatch.subsurface
	};
	return s;
}

function project_set_default_swatches() {
	// 32-Color Palette by Andrew Kensler
	// http://eastfarthing.com/blog/2016-05-06-palette/
	project_raw.swatches = [];
	let colors: i32[] = [
		0xffffffff,
		0xff000000,
		0xffd6a090,
		0xffa12c32,
		0xfffa2f7a,
		0xfffb9fda,
		0xffe61cf7,
		0xff992f7c,
		0xff47011f,
		0xff051155,
		0xff4f02ec,
		0xff2d69cb,
		0xff00a6ee,
		0xff6febff,
		0xff08a29a,
		0xff2a666a,
		0xff063619,
		0xff4a4957,
		0xff8e7ba4,
		0xffb7c0ff,
		0xffacbe9c,
		0xff827c70,
		0xff5a3b1c,
		0xffae6507,
		0xfff7aa30,
		0xfff4ea5c,
		0xff9b9500,
		0xff566204,
		0xff11963b,
		0xff51e113,
		0xff08fdcc
	];
	for (let i: i32 = 0; i < colors.length; ++i) {
		let c: i32 = colors[i];
		array_push(project_raw.swatches, make_swatch(c));
	}
}

function project_get_material_group_by_name(group_name: string): node_group_t {
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		if (g.canvas.name == group_name) {
			return g;
		}
	}
	return null;
}

function project_is_material_group_in_use(group: node_group_t): bool {
	let canvases: ui_node_canvas_t[] = [];
	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		array_push(canvases, m.canvas);
	}
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let m: node_group_t = project_material_groups[i];
		array_push(canvases, m.canvas);
	}
	for (let i: i32 = 0; i < canvases.length; ++i) {
		let canvas: ui_node_canvas_t = canvases[i];
		for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
			let n: ui_node_t = canvas.nodes[i];
			let cname: string = group.canvas.name;
			if (n.type == "GROUP" && n.name == cname) {
				return true;
			}
		}
	}
	return false;
}

type node_group_t = {
	nodes?: ui_nodes_t;
	canvas?: ui_node_canvas_t;
};

type project_format_t = {
	version?: string;
	assets?: string[]; // texture_assets
	is_bgra?: bool; // Swapped red and blue channels for layer textures
	packed_assets?: packed_asset_t[];
	envmap?: string; // Asset name
	envmap_strength?: f32;
	camera_world?: f32_array_t;
	camera_origin?: f32_array_t;
	camera_fov?: f32;
	swatches?: swatch_color_t[];

	brush_nodes?: ui_node_canvas_t[];
	brush_icons?: buffer_t[];
	material_nodes?: ui_node_canvas_t[];
	material_groups?: ui_node_canvas_t[];
	material_icons?: buffer_t[];
	font_assets?: string[];
	layer_datas?: layer_data_t[];
	mesh_datas?: mesh_data_t[];
	mesh_assets?: string[];
	mesh_icons?: buffer_t[];

	atlas_objects?: i32[];
	atlas_names?: string[];

	///if is_lab
	material?: ui_node_canvas_t;
	mesh_data?: mesh_data_t;
	mesh_icon?: buffer_t;
	///end
};

type asset_t = {
	id?: i32;
	name?: string;
	file?: string;
};

type packed_asset_t = {
	name?: string;
	bytes?: buffer_t;
};

type swatch_color_t = {
	base?: color_t;
	opacity?: f32;
	occlusion?: f32;
	roughness?: f32;
	metallic?: f32;
	normal?: color_t;
	emission?: f32;
	height?: f32;
	subsurface?: f32;
};

type layer_data_t = {
	name?: string;
	res?: i32; // Width pixels
	bpp?: i32; // Bits per pixel
	texpaint?: buffer_t;
	uv_scale?: f32;
	uv_rot?: f32;
	uv_type?: i32;
	decal_mat?: f32_array_t;
	opacity_mask?: f32;
	fill_layer?: i32;
	object_mask?: i32;
	blending?: i32;
	parent?: i32;
	visible?: bool;
	texpaint_nor?: buffer_t;
	texpaint_pack?: buffer_t;
	paint_base?: bool;
	paint_opac?: bool;
	paint_occ?: bool;
	paint_rough?: bool;
	paint_met?: bool;
	paint_nor?: bool;
	paint_nor_blend?: bool;
	paint_height?: bool;
	paint_height_blend?: bool;
	paint_emis?: bool;
	paint_subs?: bool;
};
