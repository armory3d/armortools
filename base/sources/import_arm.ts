
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
		scene_raw_gc = armpack_decode(b);
		project = {
			mesh_datas: scene_raw_gc.mesh_datas
		};
	}
	else {
		project = armpack_decode(b);
	}

	///if (is_paint || is_sculpt)
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
	context_raw.layer_filter = 0;
	///end

	project_new(import_as_mesh);
	project_filepath = path;
	ui_files_filename = substring(path, string_last_index_of(path, path_sep) + 1, string_last_index_of(path, "."));
	///if (arm_android || arm_ios)
	sys_title_set(ui_files_filename);
	///else
	sys_title_set(ui_files_filename + " - " + manifest_title);
	///end

	///if (is_paint || is_sculpt)
	// Import as mesh instead
	if (import_as_mesh) {
		import_arm_run_mesh(project);
		return;
	}
	///end

	// Save to recent
	///if arm_ios
	let recent_path: string = substring(path, string_last_index_of(path, "/") + 1, path.length);
	///else
	let recent_path: string = path;
	///end
	///if arm_windows
	recent_path = string_replace_all(recent_path, "\\", "/");
	///end
	let recent: string[] = config_raw.recent_projects;
	array_remove(recent, recent_path);
	array_insert(recent, 0, recent_path);
	config_save();

	project_raw = project;

	///if (is_paint || is_sculpt)
	let l0: layer_data_t = project.layer_datas[0];
	base_res_handle.position = config_get_texture_res_pos(l0.res);
	let bits_pos: texture_bits_t = l0.bpp == 8 ? texture_bits_t.BITS8 : l0.bpp == 16 ? texture_bits_t.BITS16 : texture_bits_t.BITS32;
	base_bits_handle.position = bits_pos;
	let bytes_per_pixel: i32 = math_floor(l0.bpp / 8);
	let format: tex_format_t = l0.bpp == 8 ? tex_format_t.RGBA32 : l0.bpp == 16 ? tex_format_t.RGBA64 : tex_format_t.RGBA128;
	///end

	let base: string = path_base_dir(path);
	if (project_raw.envmap != null) {
		project_raw.envmap = data_is_abs(project_raw.envmap) ? project_raw.envmap : base + project_raw.envmap;
	}
	scene_world.strength = project_raw.envmap_strength;

	if (project_raw.camera_world != null) {
		scene_camera.base.transform.local = mat4_from_f32_array(project_raw.camera_world);
		transform_decompose(scene_camera.base.transform);
		scene_camera.data.fov = project_raw.camera_fov;
		camera_object_build_proj(scene_camera);
		let origin: f32_array_t = project_raw.camera_origin;
		camera_origins[0].v = vec4_create(origin[0], origin[1], origin[2]);
	}

	for (let i: i32 = 0; i < project.assets.length; ++i) {
		let file: string = project.assets[i];
		///if arm_windows
		file = string_replace_all(file, "/", "\\");
		///else
		file = string_replace_all(file, "\\", "/");
		///end
		// Convert image path from relative to absolute
		let abs: string = data_is_abs(file) ? file : base + file;
		if (project.packed_assets != null) {
			abs = path_normalize(abs);
			import_arm_unpack_asset(project, abs, file);
		}
		if (map_get(data_cached_images, abs) == null && !file_exists(abs)) {
			import_arm_make_pink(abs);
		}
		let hdr_as_envmap: bool = ends_with(abs, ".hdr") && project_raw.envmap == abs;
		import_texture_run(abs, hdr_as_envmap);
	}

	///if (is_paint || is_sculpt)
	if (project.font_assets != null) {
		for (let i: i32 = 0; i < project.font_assets.length; ++i) {
			let file: string = project.font_assets[i];
			///if arm_windows
			file = string_replace_all(file, "/", "\\");
			///else
			file = string_replace_all(file, "\\", "/");
			///end
			// Convert font path from relative to absolute
			let abs: string = data_is_abs(file) ? file : base + file;
			if (file_exists(abs)) {
				import_font_run(abs);
			}
		}
	}
	///end

	///if (is_paint || is_sculpt)
	let md: mesh_data_t = mesh_data_create(project.mesh_datas[0]);
	///end

	///if is_lab
	let md: mesh_data_t = mesh_data_create(project.mesh_data);
	///end

	mesh_object_set_data(context_raw.paint_object, md);
	context_raw.paint_object.base.transform.scale = vec4_create(1, 1, 1);
	transform_build_matrix(context_raw.paint_object.base.transform);
	context_raw.paint_object.base.name = md.name;
	project_paint_objects = [context_raw.paint_object];

	///if (is_paint || is_sculpt)
	for (let i: i32 = 1; i < project.mesh_datas.length; ++i) {
		let raw: mesh_data_t = project.mesh_datas[i];
		let md: mesh_data_t = mesh_data_create(raw);
		let object: mesh_object_t = scene_add_mesh_object(md, context_raw.paint_object.materials, context_raw.paint_object.base);
		object.base.name = md.name;
		object.skip_context = "paint";
		array_push(project_paint_objects, object);
	}

	if (project.mesh_assets != null && project.mesh_assets.length > 0) {
		let file: string = project.mesh_assets[0];
		let abs: string = data_is_abs(file) ? file : base + file;
		project_mesh_assets = [abs];
	}

	///if is_paint
	if (project.atlas_objects != null) {
		project_atlas_objects = project.atlas_objects;
	}
	if (project.atlas_names != null) {
		project_atlas_names = project.atlas_names;
	}
	///end

	// No mask by default
	if (context_raw.merged_object == null) {
		util_mesh_merge();
	}
	///end

	context_select_paint_object(context_main_object());
	viewport_scale_to_bounds();
	context_raw.paint_object.skip_context = "paint";
	context_raw.merged_object.base.visible = true;

	///if (is_paint || is_sculpt)
	let tex: image_t = project_layers[0].texpaint;
	if (tex.width != config_get_texture_res_x() || tex.height != config_get_texture_res_y()) {
		if (history_undo_layers != null) {
			for (let i: i32 = 0; i < history_undo_layers.length; ++i) {
				let l: slot_layer_t = history_undo_layers[i];
				slot_layer_resize_and_set_bits(l);
			}
		}
		let rts: map_t<string, render_target_t> = render_path_render_targets;
		let blend0: render_target_t = map_get(rts, "texpaint_blend0");
		let _texpaint_blend0: image_t = blend0._image;
		app_notify_on_next_frame(function (_texpaint_blend0: image_t) {
			image_unload(_texpaint_blend0);
		}, _texpaint_blend0);
		blend0.width = config_get_texture_res_x();
		blend0.height = config_get_texture_res_y();
		blend0._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8, depth_format_t.NO_DEPTH);
		let blend1: render_target_t = map_get(rts, "texpaint_blend1");
		let _texpaint_blend1: image_t = blend1._image;
		app_notify_on_next_frame(function (_texpaint_blend1: image_t) {
			image_unload(_texpaint_blend1);
		}, _texpaint_blend1);
		blend1.width = config_get_texture_res_x();
		blend1.height = config_get_texture_res_y();
		blend1._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8, depth_format_t.NO_DEPTH);
		context_raw.brush_blend_dirty = true;
	}

	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		slot_layer_unload(l);
	}
	project_layers = [];
	for (let i: i32 = 0; i < project.layer_datas.length; ++i) {
		let ld: layer_data_t = project.layer_datas[i];
		let is_group: bool = ld.texpaint == null;

		///if is_paint
		let is_mask: bool = ld.texpaint != null && ld.texpaint_nor == null;
		///end
		///if is_sculpt
		let is_mask: bool = false;
		///end

		let l: slot_layer_t = slot_layer_create("", is_group ? layer_slot_type_t.GROUP : is_mask ? layer_slot_type_t.MASK : layer_slot_type_t.LAYER);
		if (ld.name != null) {
			l.name = ld.name;
		}
		l.visible = ld.visible;
		array_push(project_layers, l);

		if (!is_group) {
			let _texpaint: image_t = null;
			let _texpaint_nor: image_t = null;
			let _texpaint_pack: image_t = null;

			if (is_mask) {
				_texpaint = image_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4), ld.res, ld.res, tex_format_t.RGBA32);
				g2_begin(l.texpaint);
				// g2_set_pipeline(pipes_copy8);
				g2_set_pipeline(project.is_bgra ? pipes_copy_bgra : pipes_copy); // Full bits for undo support, R8 is used
				g2_draw_image(_texpaint, 0, 0);
				g2_set_pipeline(null);
				g2_end();
			}
			else { // Layer
				// TODO: create render target from bytes
				_texpaint = image_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4 * bytes_per_pixel), ld.res, ld.res, format);
				g2_begin(l.texpaint);
				g2_set_pipeline(project.is_bgra ? pipes_copy_bgra : pipes_copy);
				g2_draw_image(_texpaint, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				///if is_paint
				_texpaint_nor = image_from_bytes(lz4_decode(ld.texpaint_nor, ld.res * ld.res * 4 * bytes_per_pixel), ld.res, ld.res, format);
				g2_begin(l.texpaint_nor);
				g2_set_pipeline(project.is_bgra ? pipes_copy_bgra : pipes_copy);
				g2_draw_image(_texpaint_nor, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				_texpaint_pack = image_from_bytes(lz4_decode(ld.texpaint_pack, ld.res * ld.res * 4 * bytes_per_pixel), ld.res, ld.res, format);
				g2_begin(l.texpaint_pack);
				g2_set_pipeline(project.is_bgra ? pipes_copy_bgra : pipes_copy);
				g2_draw_image(_texpaint_pack, 0, 0);
				g2_set_pipeline(null);
				g2_end();
				///end
			}

			l.scale = ld.uv_scale;
			l.angle = ld.uv_rot;
			l.uv_type = ld.uv_type;
			if (ld.decal_mat != null) {
				l.decal_mat = mat4_from_f32_array(ld.decal_mat);
			}
			l.mask_opacity = ld.opacity_mask;
			l.object_mask = ld.object_mask;
			l.blending = ld.blending;

			///if is_paint
			l.paint_base = ld.paint_base;
			l.paint_opac = ld.paint_opac;
			l.paint_occ = ld.paint_occ;
			l.paint_rough = ld.paint_rough;
			l.paint_met = ld.paint_met;
			l.paint_nor = ld.paint_nor;
			l.paint_nor_blend = ld.paint_nor_blend;
			l.paint_height = ld.paint_height;
			l.paint_height_blend = ld.paint_height_blend;
			l.paint_emis = ld.paint_emis;
			l.paint_subs = ld.paint_subs;
			///end

			app_notify_on_next_frame(function (_texpaint: image_t) {
				image_unload(_texpaint);
			}, _texpaint);

			///if is_paint
			if (_texpaint_nor != null) {
				app_notify_on_next_frame(function (_texpaint_nor: image_t) {
					image_unload(_texpaint_nor);
				}, _texpaint_nor);
			}
			if (_texpaint_pack != null) {
				app_notify_on_next_frame(function (_texpaint_pack: image_t) {
					image_unload(_texpaint_pack);
				}, _texpaint_pack);
			}
			///end
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
	///end

	ui_nodes_hwnd.redraws = 2;
	ui_nodes_group_stack = [];
	project_material_groups = [];
	if (project.material_groups != null) {
		for (let i: i32 = 0; i < project.material_groups.length; ++i) {
			let g: ui_node_canvas_t = project.material_groups[i];
			let ng: node_group_t = {
				canvas: g,
				nodes: ui_nodes_create()
			};
			array_push(project_material_groups, ng);
		}
	}

	///if (is_paint || is_sculpt)
	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		context_raw.material = m;
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
		let l: slot_layer_t = project_layers[i];
		let is_group: bool = ld.texpaint == null;
		if (!is_group) {
			l.fill_layer = ld.fill_layer > -1 ? project_materials[ld.fill_layer] : null;
		}
	}

	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	///end

	///if is_lab
	import_arm_init_nodes(project.material.nodes);
	project_canvas = project.material;
	parser_logic_parse(project_canvas);
	///end

	context_raw.ddirty = 4;
	data_delete_blob(path);
}

function import_arm_run_mesh(raw: project_format_t) {
	project_paint_objects = [];
	for (let i: i32 = 0; i < raw.mesh_datas.length; ++i) {
		let md: mesh_data_t = mesh_data_create(raw.mesh_datas[i]);
		let object: mesh_object_t = null;
		if (i == 0) {
			mesh_object_set_data(context_raw.paint_object, md);
			object = context_raw.paint_object;
		}
		else {
			object = scene_add_mesh_object(md, context_raw.paint_object.materials, context_raw.paint_object.base);
			object.base.name = md.name;
			object.skip_context = "paint";
			md._.handle = md.name;
			map_set(data_cached_meshes, md._.handle, md);
		}
		object.base.transform.scale = vec4_create(1, 1, 1);
		transform_build_matrix(object.base.transform);
		object.base.name = md.name;
		array_push(project_paint_objects, object);
		util_mesh_merge();
		viewport_scale_to_bounds();
	}
	app_notify_on_init(layers_init);
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
		///if arm_windows
		file = string_replace_all(file, "/", "\\");
		///else
		file = string_replace_all(file, "\\", "/");
		///end
		// Convert image path from relative to absolute
		let abs: string = data_is_abs(file) ? file : base + file;
		if (project.packed_assets != null) {
			abs = path_normalize(abs);
			import_arm_unpack_asset(project, abs, file);
		}
		if (map_get(data_cached_images, abs) == null && !file_exists(abs)) {
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
			let c: ui_node_canvas_t = util_clone_canvas(project.material_groups[i]);
			while (import_arm_group_exists(c)) {
				import_arm_rename_group(c.name, imported, project.material_groups); // Ensure unique group name
			}
			import_arm_init_nodes(c.nodes);
			let ng: node_group_t = {
				canvas: c,
				nodes: ui_nodes_create()
			};
			array_push(project_material_groups, ng);
		}
	}

	app_notify_on_init(function (imported: slot_material_t[]) {
		for (let i: i32 = 0; i < imported.length; ++i) {
			let m: slot_material_t = imported[i];
			context_set_material(m);
			make_material_parse_paint_material();
			util_render_make_material_preview();
		}
	}, imported);

	ui_nodes_group_stack = [];
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	data_delete_blob(path);
}

function import_arm_group_exists(c: ui_node_canvas_t): bool {
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		let cname: string = g.canvas.name;
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
	let b: buffer_t = data_get_blob(path);
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
		///if arm_windows
		file = string_replace_all(file, "/", "\\");
		///else
		file = string_replace_all(file, "\\", "/");
		///end
		// Convert image path from relative to absolute
		let abs: string = data_is_abs(file) ? file : base + file;
		if (project.packed_assets != null) {
			abs = path_normalize(abs);
			import_arm_unpack_asset(project, abs, file);
		}
		if (map_get(data_cached_images, abs) == null && !file_exists(abs)) {
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

	app_notify_on_init(function (imported: slot_brush_t[]) {
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
	let b: buffer_t = data_get_blob(path);
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
			let s: swatch_color_t = project.swatches[i];
			array_push(project_raw.swatches, s);
		}
	}
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
	data_delete_blob(path);
}

function import_arm_make_pink(abs: string) {
	console_error(strings_error2() + " " + abs);
	let b: u8_array_t = u8_array_create(4);
	b[0] = 255;
	b[1] = 0;
	b[2] = 255;
	b[3] = 255;
	let pink: image_t = image_from_bytes(b, 1, 1);
	map_set(data_cached_images, abs, pink);
}

function import_arm_texture_node_name(): string {
	///if (is_paint || is_sculpt)
	return "TEX_IMAGE";
	///else
	return "image_texture_node";
	///end
}

function import_arm_init_nodes(nodes: ui_node_t[]) {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let node: ui_node_t = nodes[i];
		if (node.type == import_arm_texture_node_name()) {
			node.buttons[0].default_value = f32_array_create_x(base_get_asset_index(u8_array_to_string(node.buttons[0].data)));
			node.buttons[0].data = u8_array_create_from_string("");
		}
	}
}

function import_arm_unpack_asset(project: project_format_t, abs: string, file: string) {
	if (project_raw.packed_assets == null) {
		project_raw.packed_assets = [];
	}
	for (let i: i32 = 0; i < project.packed_assets.length; ++i) {
		let pa: packed_asset_t = project.packed_assets[i];
		///if arm_windows
		pa.name = string_replace_all(pa.name, "/", "\\");
		///else
		pa.name = string_replace_all(pa.name, "\\", "/");
		///end
		pa.name = path_normalize(pa.name);
		if (pa.name == file) {
			pa.name = abs; // From relative to absolute
		}
		if (pa.name == abs) {
			if (!project_packed_asset_exists(project_raw.packed_assets, pa.name)) {
				array_push(project_raw.packed_assets, pa);
			}
			let image: image_t = image_from_encoded_bytes(pa.bytes, ends_with(pa.name, ".jpg") ? ".jpg" : ".png");
			map_set(data_cached_images, abs, image);
			break;
		}
	}
}

function _import_arm_get_node_socket_array(old: map_t<string, any>, key: string): ui_node_socket_t[] {
	let sockets: ui_node_socket_t[] = [];
	let ias: any[] = map_get(old, key);
	for (let i: i32 = 0; i < ias.length; ++i) {
		let old: map_t<string, any> = ias[i];
		let s: ui_node_socket_t = {};
		s.id = armpack_map_get_i32(old, "id");
		s.node_id = armpack_map_get_i32(old, "node_id");
		s.name = map_get(old, "name");
		s.type = map_get(old, "type");
		s.color = armpack_map_get_i32(old, "color");
		if (s.type == "VALUE") {
			let x: f32 = armpack_map_get_f32(old, "default_value");
			s.default_value = f32_array_create_x(x);
		}
		else { // VECTOR, RGBA
			let dv: map_t<string, any> = map_get(old, "default_value");
			let x: f32 = armpack_map_get_f32(dv, "0");
			let y: f32 = armpack_map_get_f32(dv, "1");
			let z: f32 = armpack_map_get_f32(dv, "2");
			if (s.type == "VECTOR") {
				s.default_value = f32_array_create_xyz(x, y, z);
			}
			else { // RGBA
				let w: f32 = armpack_map_get_f32(dv, "3");
				s.default_value = f32_array_create_xyzw(x, y, z, w);
			}
		}
		s.min = armpack_map_get_f32(old, "min");
		s.max = armpack_map_get_f32(old, "max");
		if (s.max == 0.0) {
			s.max = 1.0;
		}
		s.precision = armpack_map_get_f32(old, "precision");
		if (s.precision == 0.0) {
			s.precision = 100.0;
		}
		s.display = armpack_map_get_i32(old, "display");
		array_push(sockets, s);
	}
	return sockets;
}

function _import_arm_get_node_canvas_array(map: map_t<string, any>, key: string): ui_node_canvas_t[] {
	let cas: any[] = map_get(map, key);
	if (cas == null) {
		return null;
	}
	let ar: ui_node_canvas_t[] = [];
	for (let i: i32 = 0; i < cas.length; ++i) {
		let old: map_t<string, any> = cas[i];
		let c: ui_node_canvas_t = {};
		c.name = map_get(old, "name");

		c.nodes = [];
		let nas: any[] = map_get(old, "nodes");
		for (let i: i32 = 0; i < nas.length; ++i) {
			let old: map_t<string, any> = nas[i];
			let n: ui_node_t = {};

			n.id = armpack_map_get_i32(old, "id");
			n.name = map_get(old, "name");
			n.type = map_get(old, "type");
			n.x = armpack_map_get_f32(old, "x");
			n.y = armpack_map_get_f32(old, "y");
			n.color = armpack_map_get_i32(old, "color");
			n.inputs = _import_arm_get_node_socket_array(old, "inputs");
			n.outputs = _import_arm_get_node_socket_array(old, "outputs");

			n.buttons = [];
			let bas: any[] = map_get(old, "buttons");
			for (let i: i32 = 0; i < bas.length; ++i) {
				let old: map_t<string, any> = bas[i];
				let b: ui_node_button_t = {};
				b.name = map_get(old, "name");
				b.type = map_get(old, "type");
				b.output = armpack_map_get_i32(old, "output");

				if (b.type == "ENUM") {
					let x: f32 = armpack_map_get_i32(old, "default_value");
					b.default_value = f32_array_create_x(x);

					if (b.name == "File") {
						let data_string: string = map_get(old, "data");
						b.data = sys_string_to_buffer(data_string);
					}
					else {
						let data_strings: string[] = map_get(old, "data");
						let joined: string = string_array_join(data_strings, "\n");
						b.data = sys_string_to_buffer(joined);
					}
				}
				else if (b.type == "BOOL") {
					let x: f32 = armpack_map_get_i32(old, "default_value");
					b.default_value = f32_array_create_x(x);
				}
				else if (b.type == "CUSTOM") {
					if (b.name == "arm.shader.NodesMaterial.newGroupButton") {
						b.name = "nodes_material_new_group_button";
					}
					else if (b.name == "arm.shader.NodesMaterial.groupOutputButton") {
						b.name = "nodes_material_group_output_button";
					}
					else if (b.name == "arm.shader.NodesMaterial.groupInputButton") {
						b.name = "nodes_material_group_input_button";
					}
					else if (b.name == "arm.shader.NodesMaterial.vectorCurvesButton") {
						b.name = "nodes_material_vector_curves_button";
					}
					else if (b.name == "arm.shader.NodesMaterial.colorRampButton") {
						b.name = "nodes_material_color_ramp_button";
					}
				}

				b.min = armpack_map_get_f32(old, "min");
				b.max = armpack_map_get_f32(old, "max");
				b.precision = armpack_map_get_f32(old, "precision");
				b.height = armpack_map_get_f32(old, "height");
				array_push(n.buttons, b);
			}

			n.width = armpack_map_get_f32(old, "width");

			array_push(c.nodes, n);
		}

		c.links = [];
		let las: any[] = map_get(old, "links");
		for (let i: i32 = 0; i < las.length; ++i) {
			let old: map_t<string, any> = las[i];
			let l: ui_node_link_t = {};
			l.id = armpack_map_get_i32(old, "id");
			l.from_id = armpack_map_get_i32(old, "from_id");
			l.from_socket = armpack_map_get_i32(old, "from_socket");
			l.to_id = armpack_map_get_i32(old, "to_id");
			l.to_socket = armpack_map_get_i32(old, "to_socket");
			array_push(c.links, l);
		}

		array_push(ar, c);
	}
	return ar;
}

function import_arm_has_version(b: buffer_t): bool {
	let has_version: bool = b[10] == 118; // 'v';
	return has_version;
}

function import_arm_is_legacy(b: buffer_t): bool {
	// Cloud materials are at version 0.8 / 0.9
	let has_version: bool = b[10] == 118; // 'v'
	let has_zero: bool = b[22] == 48; // '0'
	let has_dot: bool = b[23] == 46; // '.'
	let has_eight: bool = b[24] == 56; // '8'
	let has_nine: bool = b[24] == 57; // '9'
	if (has_version && has_zero && has_dot && (has_eight || has_nine)) {
		return true;
	}
	return false;
}

function import_arm_from_legacy(b: buffer_t): project_format_t {
	// Deprecated
	let old: map_t<string, any> = armpack_decode_to_map(b);
	let project: project_format_t = {};
	project.version = "1.0 alpha";
	project.assets = map_get(old, "assets");
	if (project.assets == null) {
		project.assets = [];
	}
	project.is_bgra = armpack_map_get_i32(old, "is_bgra") > 0;
	let pas: any[] = map_get(old, "packed_assets");
	if (pas != null) {
		project.packed_assets = [];
		for (let i: i32 = 0; i < pas.length; ++i) {
			let old: map_t<string, any> = pas[i];
			let pa: packed_asset_t = {};
			pa.name = map_get(old, "name");
			pa.bytes = map_get(old, "bytes");
			array_push(project.packed_assets, pa);
		}
	}
	project.brush_nodes = _import_arm_get_node_canvas_array(old, "brush_nodes");
	project.brush_icons = map_get(old, "brush_icons");
	project.material_nodes = _import_arm_get_node_canvas_array(old, "material_nodes");
	project.material_groups = _import_arm_get_node_canvas_array(old, "material_groups");
	project.material_icons = map_get(old, "material_icons");
	return project;
}
