
let scene_raw_gc: scene_t;

function import_arm_run_project(path: string) {
	let b: buffer_t = data_get_blob(path);
	let project: project_format_t;
	let import_as_mesh: bool = false;

	if (import_arm_is_legacy(b)) {
		project = import_arm_from_legacy(b);
	}
	else if (!import_arm_has_version(b)) {
		import_as_mesh = true;
		scene_raw_gc   = armpack_decode(b);
		project        = {mesh_datas : scene_raw_gc.mesh_datas};
	}
	else {
		project = armpack_decode(b);
	}

	if (project.version != null && project.layer_datas == null) {
		// Import as material
		if (project.material_nodes != null) {
			import_arm_run_material_from_project(project, path);
		}
		// Import as brush
		else if (project.brush_nodes != null) {
			import_arm_run_brush_from_project(project, path);
		}
		// Import as swatches
		else if (project.swatches != null) {
			import_arm_run_swatches_from_project(project, path);
		}
		return;
	}
	context_raw.layers_preview_dirty = true;
	context_raw.layer_filter         = 0;

	project_new(import_as_mesh);
	project_filepath  = path;
	ui_files_filename = substring(path, string_last_index_of(path, path_sep) + 1, string_last_index_of(path, "."));
	/// if (arm_android || arm_ios)
	sys_title_set(ui_files_filename);
	/// else
	sys_title_set(ui_files_filename + " - " + manifest_title);
	/// end

	// Import as mesh instead
	if (import_as_mesh) {
		import_arm_run_mesh(project);
		return;
	}

	// Save to recent
	/// if arm_ios
	let recent_path: string = substring(path, string_last_index_of(path, "/") + 1, path.length);
	/// else
	let recent_path: string = path;
	/// end
	/// if arm_windows
	recent_path = string_replace_all(recent_path, "\\", "/");
	/// end
	let recent: string[] = config_raw.recent_projects;
	array_remove(recent, recent_path);
	array_insert(recent, 0, recent_path);
	config_save();

	project_raw = project;

	let l0: layer_data_t         = project.layer_datas[0];
	base_res_handle.i            = config_get_texture_res_pos(l0.res);
	let bits_pos: texture_bits_t = l0.bpp == 8 ? texture_bits_t.BITS8 : l0.bpp == 16 ? texture_bits_t.BITS16 : texture_bits_t.BITS32;
	base_bits_handle.i           = bits_pos;
	let bytes_per_pixel: i32     = math_floor(l0.bpp / 8);
	let format: tex_format_t     = l0.bpp == 8 ? tex_format_t.RGBA32 : l0.bpp == 16 ? tex_format_t.RGBA64 : tex_format_t.RGBA128;

	let base: string = path_base_dir(path);
	if (project_raw.envmap != null) {
		project_raw.envmap = data_is_abs(project_raw.envmap) ? project_raw.envmap : base + project_raw.envmap;
		/// if arm_windows
		project_raw.envmap = string_replace_all(project_raw.envmap, "/", "\\");
		/// else
		project_raw.envmap = string_replace_all(project_raw.envmap, "\\", "/");
		/// end
	}
	scene_world.strength = project_raw.envmap_strength;

	if (project_raw.camera_world != null) {
		scene_camera.base.transform.local = mat4_from_f32_array(project_raw.camera_world);
		transform_decompose(scene_camera.base.transform);
		scene_camera.data.fov = project_raw.camera_fov;
		camera_object_build_proj(scene_camera);
		let origin: f32_array_t = project_raw.camera_origin;
		camera_origins[0].v     = vec4_create(origin[0], origin[1], origin[2]);
	}

	for (let i: i32 = 0; i < project.assets.length; ++i) {
		let file: string = project.assets[i];
		/// if arm_windows
		file = string_replace_all(file, "/", "\\");
		/// else
		file = string_replace_all(file, "\\", "/");
		/// end
		// Convert image path from relative to absolute
		let abs: string = data_is_abs(file) ? file : base + file;
		if (project.packed_assets != null) {
			abs = path_normalize(abs);
			import_arm_unpack_asset(project, abs, file);
		}
		if (map_get(data_cached_images, abs) == null && !iron_file_exists(abs)) {
			import_arm_make_pink(abs);
		}
		let hdr_as_envmap: bool = ends_with(abs, ".hdr") && project_raw.envmap == abs;
		import_texture_run(abs, hdr_as_envmap);
	}

	if (project.font_assets != null) {
		for (let i: i32 = 0; i < project.font_assets.length; ++i) {
			let file: string = project.font_assets[i];
			/// if arm_windows
			file = string_replace_all(file, "/", "\\");
			/// else
			file = string_replace_all(file, "\\", "/");
			/// end
			// Convert font path from relative to absolute
			let abs: string = data_is_abs(file) ? file : base + file;
			if (iron_file_exists(abs)) {
				import_font_run(abs);
			}
		}
	}

	let md: mesh_data_t = mesh_data_create(project.mesh_datas[0]);

	mesh_object_set_data(context_raw.paint_object, md);
	context_raw.paint_object.base.transform.scale = vec4_create(1, 1, 1);
	transform_build_matrix(context_raw.paint_object.base.transform);
	context_raw.paint_object.base.name = md.name;
	project_paint_objects              = [ context_raw.paint_object ];

	for (let i: i32 = 1; i < project.mesh_datas.length; ++i) {
		let raw: mesh_data_t      = project.mesh_datas[i];
		let md: mesh_data_t       = mesh_data_create(raw);
		let object: mesh_object_t = scene_add_mesh_object(md, context_raw.paint_object.material, context_raw.paint_object.base);
		object.base.name          = md.name;
		object.skip_context       = "paint";
		array_push(project_paint_objects, object);
	}

	if (project.mesh_assets != null && project.mesh_assets.length > 0) {
		let file: string    = project.mesh_assets[0];
		let abs: string     = data_is_abs(file) ? file : base + file;
		project_mesh_assets = [ abs ];
	}

	if (project.atlas_objects != null) {
		project_atlas_objects = project.atlas_objects;
	}
	if (project.atlas_names != null) {
		project_atlas_names = project.atlas_names;
	}

	// No mask by default
	if (context_raw.merged_object == null) {
		util_mesh_merge();
	}

	context_select_paint_object(context_main_object());
	viewport_scale_to_bounds();
	context_raw.paint_object.skip_context  = "paint";
	context_raw.merged_object.base.visible = true;

	let tex: gpu_texture_t = project_layers[0].texpaint;
	if (tex.width != config_get_texture_res_x() || tex.height != config_get_texture_res_y()) {
		if (history_undo_layers != null) {
			for (let i: i32 = 0; i < history_undo_layers.length; ++i) {
				let l: slot_layer_t = history_undo_layers[i];
				slot_layer_resize_and_set_bits(l);
			}
		}
		let rts: map_t<string, render_target_t> = render_path_render_targets;
		let blend0: render_target_t             = map_get(rts, "texpaint_blend0");
		let _texpaint_blend0: gpu_texture_t     = blend0._image;
		gpu_delete_texture(_texpaint_blend0);
		blend0.width                        = config_get_texture_res_x();
		blend0.height                       = config_get_texture_res_y();
		blend0._image                       = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);
		let blend1: render_target_t         = map_get(rts, "texpaint_blend1");
		let _texpaint_blend1: gpu_texture_t = blend1._image;
		gpu_delete_texture(_texpaint_blend1);
		blend1.width                  = config_get_texture_res_x();
		blend1.height                 = config_get_texture_res_y();
		blend1._image                 = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);
		context_raw.brush_blend_dirty = true;
	}

	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		slot_layer_unload(l);
	}
	project_layers = [];
	for (let i: i32 = 0; i < project.layer_datas.length; ++i) {
		let ld: layer_data_t = project.layer_datas[i];
		let is_group: bool   = ld.texpaint == null;
		let is_mask: bool    = ld.texpaint != null && ld.texpaint_nor == null;

		let l: slot_layer_t = slot_layer_create("", is_group ? layer_slot_type_t.GROUP : is_mask ? layer_slot_type_t.MASK : layer_slot_type_t.LAYER);
		if (ld.name != null) {
			l.name = ld.name;
		}
		l.visible = ld.visible;
		array_push(project_layers, l);

		if (!is_group) {
			let _texpaint: gpu_texture_t      = null;
			let _texpaint_nor: gpu_texture_t  = null;
			let _texpaint_pack: gpu_texture_t = null;

			if (is_mask) {
				_texpaint = gpu_create_texture_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4), ld.res, ld.res, tex_format_t.RGBA32);
				draw_begin(l.texpaint);
				// draw_set_pipeline(pipes_copy8);
				draw_set_pipeline(project.is_bgra ? pipes_copy_bgra : pipes_copy); // Full bits for undo support, R8 is used
				draw_image(_texpaint, 0, 0);
				draw_set_pipeline(null);
				draw_end();
			}
			else { // Layer
				// TODO: create render target from bytes
				_texpaint = gpu_create_texture_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4 * bytes_per_pixel), ld.res, ld.res, format);
				draw_begin(l.texpaint);
				draw_set_pipeline(project.is_bgra ? pipes_copy_bgra : pipes_copy);
				draw_image(_texpaint, 0, 0);
				draw_set_pipeline(null);
				draw_end();

				_texpaint_nor = gpu_create_texture_from_bytes(lz4_decode(ld.texpaint_nor, ld.res * ld.res * 4 * bytes_per_pixel), ld.res, ld.res, format);
				draw_begin(l.texpaint_nor);
				draw_set_pipeline(project.is_bgra ? pipes_copy_bgra : pipes_copy);
				draw_image(_texpaint_nor, 0, 0);
				draw_set_pipeline(null);
				draw_end();

				_texpaint_pack = gpu_create_texture_from_bytes(lz4_decode(ld.texpaint_pack, ld.res * ld.res * 4 * bytes_per_pixel), ld.res, ld.res, format);
				draw_begin(l.texpaint_pack);
				draw_set_pipeline(project.is_bgra ? pipes_copy_bgra : pipes_copy);
				draw_image(_texpaint_pack, 0, 0);
				draw_set_pipeline(null);
				draw_end();
			}

			l.scale   = ld.uv_scale;
			l.angle   = ld.uv_rot;
			l.uv_type = ld.uv_type;
			if (ld.decal_mat != null) {
				l.decal_mat = mat4_from_f32_array(ld.decal_mat);
			}
			l.mask_opacity = ld.opacity_mask;
			l.object_mask  = ld.object_mask;
			l.blending     = ld.blending;

			l.paint_base         = ld.paint_base;
			l.paint_opac         = ld.paint_opac;
			l.paint_occ          = ld.paint_occ;
			l.paint_rough        = ld.paint_rough;
			l.paint_met          = ld.paint_met;
			l.paint_nor          = ld.paint_nor;
			l.paint_nor_blend    = ld.paint_nor_blend;
			l.paint_height       = ld.paint_height;
			l.paint_height_blend = ld.paint_height_blend;
			l.paint_emis         = ld.paint_emis;
			l.paint_subs         = ld.paint_subs;

			gpu_delete_texture(_texpaint);

			if (_texpaint_nor != null) {
				gpu_delete_texture(_texpaint_nor);
			}
			if (_texpaint_pack != null) {
				gpu_delete_texture(_texpaint_pack);
			}
		}
	}

	// Assign parents to groups and masks
	for (let i: i32 = 0; i < project.layer_datas.length; ++i) {
		let ld: layer_data_t = project.layer_datas[i];
		if (ld.parent >= 0) {
			project_layers[i].parent = project_layers[ld.parent];
		}
	}

	context_set_layer(project_layers[0]);

	// Materials
	let m0: material_data_t = data_get_material("Scene", "Material");

	project_materials = [];
	for (let i: i32 = 0; i < project.material_nodes.length; ++i) {
		let n: ui_node_canvas_t = project.material_nodes[i];
		import_arm_init_nodes(n.nodes);
		context_raw.material = slot_material_create(m0, n);
		array_push(project_materials, context_raw.material);
	}

	ui_nodes_hwnd.redraws   = 2;
	ui_nodes_group_stack    = [];
	project_material_groups = [];
	if (project.material_groups != null) {
		for (let i: i32 = 0; i < project.material_groups.length; ++i) {
			let g: ui_node_canvas_t = project.material_groups[i];
			let ng: node_group_t    = {canvas : g, nodes : ui_nodes_create()};
			array_push(project_material_groups, ng);
		}
	}

	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		context_raw.material   = m;
		make_material_parse_paint_material();
		util_render_make_material_preview();
	}

	project_brushes = [];
	for (let i: i32 = 0; i < project.brush_nodes.length; ++i) {
		let n: ui_node_canvas_t = project.brush_nodes[i];
		import_arm_init_nodes(n.nodes);
		context_raw.brush = slot_brush_create(n);
		array_push(project_brushes, context_raw.brush);
		make_material_parse_brush();
		util_render_make_brush_preview();
	}

	// Fill layers
	for (let i: i32 = 0; i < project.layer_datas.length; ++i) {
		let ld: layer_data_t = project.layer_datas[i];
		let l: slot_layer_t  = project_layers[i];
		let is_group: bool   = ld.texpaint == null;
		if (!is_group) {
			l.fill_layer = ld.fill_layer > -1 ? project_materials[ld.fill_layer] : null;
		}
	}

	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;

	context_raw.ddirty = 4;
	data_delete_blob(path);
}

function import_arm_run_mesh(raw: project_format_t) {
	project_paint_objects = [];
	for (let i: i32 = 0; i < raw.mesh_datas.length; ++i) {
		let md: mesh_data_t       = mesh_data_create(raw.mesh_datas[i]);
		let object: mesh_object_t = null;
		if (i == 0) {
			mesh_object_set_data(context_raw.paint_object, md);
			object = context_raw.paint_object;
		}
		else {
			object              = scene_add_mesh_object(md, context_raw.paint_object.material, context_raw.paint_object.base);
			object.base.name    = md.name;
			object.skip_context = "paint";
			md._.handle         = md.name;
			map_set(data_cached_meshes, md._.handle, md);
		}
		object.base.transform.scale = vec4_create(1, 1, 1);
		transform_build_matrix(object.base.transform);
		object.base.name = md.name;
		array_push(project_paint_objects, object);
		util_mesh_merge();
		viewport_scale_to_bounds();
	}
	sys_notify_on_next_frame(layers_init);
	history_reset();
}

function import_arm_run_material(path: string) {
	let b: buffer_t = data_get_blob(path);
	let project: project_format_t;
	if (import_arm_is_legacy(b)) {
		project = import_arm_from_legacy(b);
	}
	else {
		project = armpack_decode(b);
	}

	if (project.version == null) {
		data_delete_blob(path);
		return;
	}
	import_arm_run_material_from_project(project, path);
}

function import_arm_run_material_from_project(project: project_format_t, path: string) {
	let base: string = path_base_dir(path);
	for (let i: i32 = 0; i < project.assets.length; ++i) {
		let file: string = project.assets[i];
		/// if arm_windows
		file = string_replace_all(file, "/", "\\");
		/// else
		file = string_replace_all(file, "\\", "/");
		/// end
		// Convert image path from relative to absolute
		let abs: string = data_is_abs(file) ? file : base + file;
		if (project.packed_assets != null) {
			abs = path_normalize(abs);
			import_arm_unpack_asset(project, abs, file, true);
		}
		if (map_get(data_cached_images, abs) == null && !iron_file_exists(abs)) {
			import_arm_make_pink(abs);
		}
		import_texture_run(abs);
	}

	let m0: material_data_t = data_get_material("Scene", "Material");

	let imported: slot_material_t[] = [];

	for (let i: i32 = 0; i < project.material_nodes.length; ++i) {
		let c: ui_node_canvas_t = util_clone_canvas(project.material_nodes[i]); // project will get GCed
		import_arm_init_nodes(c.nodes);
		context_raw.material = slot_material_create(m0, c);
		array_push(project_materials, context_raw.material);
		array_push(imported, context_raw.material);
		history_new_material();
	}

	if (project.material_groups != null) {
		for (let i: i32 = 0; i < project.material_groups.length; ++i) {
			project.material_groups[i] = util_clone_canvas(project.material_groups[i]);
		}
		for (let i: i32 = 0; i < project.material_groups.length; ++i) {
			let c: ui_node_canvas_t = project.material_groups[i];
			while (import_arm_group_exists(c)) {
				import_arm_rename_group(c.name, imported, project.material_groups); // Ensure unique group name
			}
			import_arm_init_nodes(c.nodes);
			let ng: node_group_t = {canvas : c, nodes : ui_nodes_create()};
			array_push(project_material_groups, ng);
		}
	}

	sys_notify_on_next_frame(function(imported: slot_material_t[]) {
		for (let i: i32 = 0; i < imported.length; ++i) {
			let m: slot_material_t = imported[i];
			context_set_material(m);
			make_material_parse_paint_material();
			util_render_make_material_preview();
		}
	}, imported);

	ui_nodes_group_stack                       = [];
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	data_delete_blob(path);
}

function import_arm_group_exists(c: ui_node_canvas_t): bool {
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		let cname: string   = g.canvas.name;
		if (cname == c.name) {
			return true;
		}
	}
	return false;
}

function import_arm_rename_group(name: string, materials: slot_material_t[], groups: ui_node_canvas_t[]) {
	for (let i: i32 = 0; i < materials.length; ++i) {
		let m: slot_material_t = materials[i];
		for (let i: i32 = 0; i < m.canvas.nodes.length; ++i) {
			let n: ui_node_t = m.canvas.nodes[i];
			if (n.type == "GROUP" && n.name == name) {
				n.name += ".1";
			}
		}
	}
	for (let i: i32 = 0; i < groups.length; ++i) {
		let c: ui_node_canvas_t = groups[i];
		if (c.name == name) {
			c.name += ".1";
		}
		for (let i: i32 = 0; i < c.nodes.length; ++i) {
			let n: ui_node_t = c.nodes[i];
			if (n.type == "GROUP" && n.name == name) {
				n.name += ".1";
			}
		}
	}
}

function import_arm_run_brush(path: string) {
	let b: buffer_t               = data_get_blob(path);
	let project: project_format_t = armpack_decode(b);
	if (project.version == null) {
		data_delete_blob(path);
		return;
	}
	import_arm_run_brush_from_project(project, path);
}

function import_arm_run_brush_from_project(project: project_format_t, path: string) {
	let base: string = path_base_dir(path);
	for (let i: i32 = 0; i < project.assets.length; ++i) {
		let file: string = project.assets[i];
		/// if arm_windows
		file = string_replace_all(file, "/", "\\");
		/// else
		file = string_replace_all(file, "\\", "/");
		/// end
		// Convert image path from relative to absolute
		let abs: string = data_is_abs(file) ? file : base + file;
		if (project.packed_assets != null) {
			abs = path_normalize(abs);
			import_arm_unpack_asset(project, abs, file, true);
		}
		if (map_get(data_cached_images, abs) == null && !iron_file_exists(abs)) {
			import_arm_make_pink(abs);
		}
		import_texture_run(abs);
	}

	let imported: slot_brush_t[] = [];

	for (let i: i32 = 0; i < project.brush_nodes.length; ++i) {
		let c: ui_node_canvas_t = util_clone_canvas(project.brush_nodes[i]);
		import_arm_init_nodes(c.nodes);
		context_raw.brush = slot_brush_create(c);
		array_push(project_brushes, context_raw.brush);
		array_push(imported, context_raw.brush);
	}

	sys_notify_on_next_frame(function(imported: slot_brush_t[]) {
		for (let i: i32 = 0; i < imported.length; ++i) {
			let b: slot_brush_t = imported[i];
			context_set_brush(b);
			make_material_parse_brush();
			util_render_make_brush_preview();
		}
	}, imported);

	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	data_delete_blob(path);
}

function import_arm_run_swatches(path: string, replace_existing: bool = false) {
	let b: buffer_t               = data_get_blob(path);
	let project: project_format_t = armpack_decode(b);
	if (project.version == null) {
		data_delete_blob(path);
		return;
	}
	import_arm_run_swatches_from_project(project, path, replace_existing);
}

function import_arm_run_swatches_from_project(project: project_format_t, path: string, replace_existing: bool = false) {
	if (replace_existing) {
		project_raw.swatches = [];

		if (project.swatches == null) { // No swatches contained
			array_push(project_raw.swatches, make_swatch());
		}
	}

	if (project.swatches != null) {
		for (let i: i32 = 0; i < project.swatches.length; ++i) {
			let s: swatch_color_t = util_clone_swatch_color(project.swatches[i]);
			array_push(project_raw.swatches, s);
		}
	}
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
	data_delete_blob(path);
}

function import_arm_make_pink(abs: string) {
	console_error(strings_could_not_locate_texture() + " " + abs);
	let b: u8_array_t       = u8_array_create(4);
	b[0]                    = 255;
	b[1]                    = 0;
	b[2]                    = 255;
	b[3]                    = 255;
	let pink: gpu_texture_t = gpu_create_texture_from_bytes(b, 1, 1);
	map_set(data_cached_images, abs, pink);
}

function import_arm_init_nodes(nodes: ui_node_t[]) {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let node: ui_node_t = nodes[i];
		if (node.type == "TEX_IMAGE") {
			node.buttons[0].default_value = f32_array_create_x(base_get_asset_index(u8_array_to_string(node.buttons[0].data)));
			node.buttons[0].data          = u8_array_create_from_string("");
		}
	}
}

function import_arm_unpack_asset(project: project_format_t, abs: string, file: string, gc_copy: bool = false) {
	if (project_raw.packed_assets == null) {
		project_raw.packed_assets = [];
	}
	for (let i: i32 = 0; i < project.packed_assets.length; ++i) {
		let pa: packed_asset_t = project.packed_assets[i];
		/// if arm_windows
		pa.name = string_replace_all(pa.name, "/", "\\");
		/// else
		pa.name = string_replace_all(pa.name, "\\", "/");
		/// end
		pa.name = path_normalize(pa.name);
		if (pa.name == file) {
			pa.name = abs; // From relative to absolute
		}
		if (pa.name == abs) {
			if (!project_packed_asset_exists(project_raw.packed_assets, pa.name)) {

				if (gc_copy) {
					let pa_gc: packed_asset_t = {
						// project will get GCed
						name : string_copy(pa.name),
						bytes : u8_array_create_from_array(pa.bytes)
					};
					pa = pa_gc;
				}

				array_push(project_raw.packed_assets, pa);
			}
			let image: gpu_texture_t = gpu_create_texture_from_encoded_bytes(pa.bytes, ends_with(pa.name, ".jpg") ? ".jpg" : ".png");
			map_set(data_cached_images, abs, image);
			break;
		}
	}
}

function import_arm_has_version(b: buffer_t): bool {
	let has_version: bool = b[10] == 118; // 'v';
	return has_version;
}
