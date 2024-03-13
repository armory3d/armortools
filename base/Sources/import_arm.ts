
function import_arm_run_project(path: string) {
	let b: ArrayBuffer = data_get_blob(path);
	let project: project_format_t = armpack_decode(b);

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

	let import_as_mesh: bool = project.version == null;
	context_raw.layers_preview_dirty = true;
	context_raw.layer_filter = 0;
	///end

	///if is_lab
	let import_as_mesh: bool = true;
	///end

	project_new(import_as_mesh);
	project_filepath = path;
	ui_files_filename = path.substring(path.lastIndexOf(path_sep) + 1, path.lastIndexOf("."));
	///if (krom_android || krom_ios)
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
	///if krom_ios
	let recent_path: string = path.substr(path.lastIndexOf("/") + 1);
	///else
	let recent_path: string = path;
	///end
	let recent: string[] = config_raw.recent_projects;
	array_remove(recent, recent_path);
	recent.unshift(recent_path);
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
	if (project_raw.envmap_strength != null) {
		scene_world.strength = project_raw.envmap_strength;
	}
	if (project_raw.camera_world != null) {
		scene_camera.base.transform.local = mat4_from_f32_array(project_raw.camera_world);
		transform_decompose(scene_camera.base.transform);
		scene_camera.data.fov = project_raw.camera_fov;
		camera_object_build_proj(scene_camera);
		let origin: Float32Array = project_raw.camera_origin;
		camera_origins[0].x = origin[0];
		camera_origins[0].y = origin[1];
		camera_origins[0].z = origin[2];
	}

	for (let file of project.assets) {
		///if krom_windows
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
		if (data_cached_images.get(abs) == null && !file_exists(abs)) {
			import_arm_make_pink(abs);
		}
		let hdr_as_envmap: bool = abs.endsWith(".hdr") && project_raw.envmap == abs;
		import_texture_run(abs, hdr_as_envmap);
	}

	///if (is_paint || is_sculpt)
	if (project.font_assets != null) {
		for (let file of project.font_assets) {
			///if krom_windows
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
	vec4_set(context_raw.paint_object.base.transform.scale, 1, 1, 1);
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
		project_paint_objects.push(object);
	}

	if (project.mesh_assets != null && project.mesh_assets.length > 0) {
		let file: string = project.mesh_assets[0];
		let abs: string = data_is_abs(file) ? file : base + file;
		project_mesh_assets = [abs];
	}

	///if is_paint
	if (project.atlas_objects != null) project_atlas_objects = project.atlas_objects;
	if (project.atlas_names != null) project_atlas_names = project.atlas_names;
	///end

	// No mask by default
	if (context_raw.merged_object == null) util_mesh_merge();
	///end

	context_select_paint_object(context_main_object());
	viewport_scale_to_bounds();
	context_raw.paint_object.skip_context = "paint";
	context_raw.merged_object.base.visible = true;

	///if (is_paint || is_sculpt)
	let tex: image_t = project_layers[0].texpaint;
	if (tex.width != config_get_texture_res_x() || tex.height != config_get_texture_res_y()) {
		if (history_undo_layers != null) for (let l of history_undo_layers) SlotLayer.slot_layer_resize_and_set_bits(l);
		let rts: map_t<string, render_target_t> = render_path_render_targets;
		let _texpaint_blend0: image_t = rts.get("texpaint_blend0")._image;
		base_notify_on_next_frame(() => {
			image_unload(_texpaint_blend0);
		});
		rts.get("texpaint_blend0").width = config_get_texture_res_x();
		rts.get("texpaint_blend0").height = config_get_texture_res_y();
		rts.get("texpaint_blend0")._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8, depth_format_t.NO_DEPTH);
		let _texpaint_blend1: image_t = rts.get("texpaint_blend1")._image;
		base_notify_on_next_frame(() => {
			image_unload(_texpaint_blend1);
		});
		rts.get("texpaint_blend1").width = config_get_texture_res_x();
		rts.get("texpaint_blend1").height = config_get_texture_res_y();
		rts.get("texpaint_blend1")._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8, depth_format_t.NO_DEPTH);
		context_raw.brush_blend_dirty = true;
	}

	for (let l of project_layers) SlotLayer.slot_layer_unload(l);
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

		let l: SlotLayerRaw = SlotLayer.slot_layer_create("", is_group ? layer_slot_type_t.GROUP : is_mask ? layer_slot_type_t.MASK : layer_slot_type_t.LAYER);
		if (ld.name != null) l.name = ld.name;
		l.visible = ld.visible;
		project_layers.push(l);

		if (!is_group) {
			if (base_pipe_merge == null) base_make_pipe();

			let _texpaint: image_t = null;

			///if is_paint
			let _texpaint_nor: image_t = null;
			let _texpaint_pack: image_t = null;
			///end

			if (is_mask) {
				_texpaint = image_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4), ld.res, ld.res, tex_format_t.RGBA32);
				g2_begin(l.texpaint);
				// g2_set_pipeline(base_pipe_copy8);
				g2_set_pipeline(project.is_bgra ? base_pipe_copy_bgra : base_pipe_copy); // Full bits for undo support, R8 is used
				g2_draw_image(_texpaint, 0, 0);
				g2_set_pipeline(null);
				g2_end();
			}
			else { // Layer
				// TODO: create render target from bytes
				_texpaint = image_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4 * bytes_per_pixel), ld.res, ld.res, format);
				g2_begin(l.texpaint);
				g2_set_pipeline(project.is_bgra ? base_pipe_copy_bgra : base_pipe_copy);
				g2_draw_image(_texpaint, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				///if is_paint
				_texpaint_nor = image_from_bytes(lz4_decode(ld.texpaint_nor, ld.res * ld.res * 4 * bytes_per_pixel), ld.res, ld.res, format);
				g2_begin(l.texpaint_nor);
				g2_set_pipeline(project.is_bgra ? base_pipe_copy_bgra : base_pipe_copy);
				g2_draw_image(_texpaint_nor, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				_texpaint_pack = image_from_bytes(lz4_decode(ld.texpaint_pack, ld.res * ld.res * 4 * bytes_per_pixel), ld.res, ld.res, format);
				g2_begin(l.texpaint_pack);
				g2_set_pipeline(project.is_bgra ? base_pipe_copy_bgra : base_pipe_copy);
				g2_draw_image(_texpaint_pack, 0, 0);
				g2_set_pipeline(null);
				g2_end();
				///end
			}

			l.scale = ld.uv_scale;
			l.angle = ld.uv_rot;
			l.uv_type = ld.uv_type;
			if (ld.decal_mat != null) l.decal_mat = mat4_from_f32_array(ld.decal_mat);
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
			l.paint_nor_blend = ld.paint_nor_blend != null ? ld.paint_nor_blend : true; // TODO: deprecated
			l.paint_height = ld.paint_height;
			l.paint_height_blend = ld.paint_height_blend != null ? ld.paint_height_blend : true; // TODO: deprecated
			l.paint_emis = ld.paint_emis;
			l.paint_subs = ld.paint_subs;
			///end

			base_notify_on_next_frame(() => {
				image_unload(_texpaint);
				///if is_paint
				if (_texpaint_nor != null) image_unload(_texpaint_nor);
				if (_texpaint_pack != null) image_unload(_texpaint_pack);
				///end
			});
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
	for (let n of project.material_nodes) {
		import_arm_init_nodes(n.nodes);
		context_raw.material = SlotMaterial.slot_material_create(m0, n);
		project_materials.push(context_raw.material);
	}
	///end

	ui_nodes_hwnd.redraws = 2;
	ui_nodes_group_stack = [];
	project_material_groups = [];
	if (project.material_groups != null) {
		for (let g of project.material_groups) project_material_groups.push({ canvas: g, nodes: zui_nodes_create() });
	}

	///if (is_paint || is_sculpt)
	for (let m of project_materials) {
		context_raw.material = m;
		MakeMaterial.make_material_parse_paint_material();
		util_render_make_material_preview();
	}

	project_brushes = [];
	for (let n of project.brush_nodes) {
		import_arm_init_nodes(n.nodes);
		context_raw.brush = SlotBrush.slot_brush_create(n);
		project_brushes.push(context_raw.brush);
		MakeMaterial.make_material_parse_brush();
		util_render_make_brush_preview();
	}

	// Fill layers
	for (let i: i32 = 0; i < project.layer_datas.length; ++i) {
		let ld: layer_data_t = project.layer_datas[i];
		let l: SlotLayerRaw = project_layers[i];
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

///if (is_paint || is_sculpt)
function import_arm_run_mesh(raw: scene_t) {
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
			data_cached_meshes.set(md._.handle, md);
		}
		vec4_set(object.base.transform.scale, 1, 1, 1);
		transform_build_matrix(object.base.transform);
		object.base.name = md.name;
		project_paint_objects.push(object);
		util_mesh_merge();
		viewport_scale_to_bounds();
	}
	app_notify_on_init(base_init_layers);
	history_reset();
}

function import_arm_run_material(path: string) {
	let b: ArrayBuffer = data_get_blob(path);
	let project: project_format_t = armpack_decode(b);
	if (project.version == null) { data_delete_blob(path); return; }
	import_arm_run_material_from_project(project, path);
}

function import_arm_run_material_from_project(project: project_format_t, path: string) {
	let base: string = path_base_dir(path);
	for (let file of project.assets) {
		///if krom_windows
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
		if (data_cached_images.get(abs) == null && !file_exists(abs)) {
			import_arm_make_pink(abs);
		}
		import_texture_run(abs);
	}

	let m0: material_data_t = data_get_material("Scene", "Material");

	let imported: SlotMaterialRaw[] = [];

	for (let c of project.material_nodes) {
		import_arm_init_nodes(c.nodes);
		context_raw.material = SlotMaterial.slot_material_create(m0, c);
		project_materials.push(context_raw.material);
		imported.push(context_raw.material);
		history_new_material();
	}

	if (project.material_groups != null) {
		for (let c of project.material_groups) {
			while (import_arm_group_exists(c)) import_arm_rename_group(c.name, imported, project.material_groups); // Ensure unique group name
			import_arm_init_nodes(c.nodes);
			project_material_groups.push({ canvas: c, nodes: zui_nodes_create() });
		}
	}

	let _init = () => {
		for (let m of imported) {
			context_set_material(m);
			MakeMaterial.make_material_parse_paint_material();
			util_render_make_material_preview();
		}
	}
	app_notify_on_init(_init);

	ui_nodes_group_stack = [];
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	data_delete_blob(path);
}

function import_arm_group_exists(c: zui_node_canvas_t): bool {
	for (let g of project_material_groups) {
		if (g.canvas.name == c.name) return true;
	}
	return false;
}

function import_arm_rename_group(name: string, materials: SlotMaterialRaw[], groups: zui_node_canvas_t[]) {
	for (let m of materials) {
		for (let n of m.canvas.nodes) {
			if (n.type == "GROUP" && n.name == name) n.name += ".1";
		}
	}
	for (let c of groups) {
		if (c.name == name) c.name += ".1";
		for (let n of c.nodes) {
			if (n.type == "GROUP" && n.name == name) n.name += ".1";
		}
	}
}

function import_arm_run_brush(path: string) {
	let b: ArrayBuffer = data_get_blob(path);
	let project: project_format_t = armpack_decode(b);
	if (project.version == null) { data_delete_blob(path); return; }
	import_arm_run_brush_from_project(project, path);
}

function import_arm_run_brush_from_project(project: project_format_t, path: string) {
	let base: string = path_base_dir(path);
	for (let file of project.assets) {
		///if krom_windows
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
		if (data_cached_images.get(abs) == null && !file_exists(abs)) {
			import_arm_make_pink(abs);
		}
		import_texture_run(abs);
	}

	let imported: SlotBrushRaw[] = [];

	for (let n of project.brush_nodes) {
		import_arm_init_nodes(n.nodes);
		context_raw.brush = SlotBrush.slot_brush_create(n);
		project_brushes.push(context_raw.brush);
		imported.push(context_raw.brush);
	}

	let _init = () => {
		for (let b of imported) {
			context_set_brush(b);
			util_render_make_brush_preview();
		}
	}
	app_notify_on_init(_init);

	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	data_delete_blob(path);
}
///end

function import_arm_run_swatches(path: string, replace_existing: bool = false) {
	let b: ArrayBuffer = data_get_blob(path);
	let project: project_format_t = armpack_decode(b);
	if (project.version == null) { data_delete_blob(path); return; }
	import_arm_run_swatches_from_project(project, path, replace_existing);
}

function import_arm_run_swatches_from_project(project: project_format_t, path: string, replace_existing: bool = false) {
	if (replace_existing) {
		project_raw.swatches = [];

		if (project.swatches == null) { // No swatches contained
			project_raw.swatches.push(make_swatch());
		}
	}

	if (project.swatches != null) {
		for (let s of project.swatches) {
			project_raw.swatches.push(s);
		}
	}
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
	data_delete_blob(path);
}

function import_arm_make_pink(abs: string) {
	console_error(strings_error2() + " " + abs);
	let b: Uint8Array = new Uint8Array(4);
	b[0] = 255;
	b[1] = 0;
	b[2] = 255;
	b[3] = 255;
	let pink: image_t = image_from_bytes(b.buffer, 1, 1);
	data_cached_images.set(abs, pink);
}

function import_arm_texture_node_name(): string {
	///if (is_paint || is_sculpt)
	return "TEX_IMAGE";
	///else
	return "ImageTextureNode";
	///end
}

function import_arm_init_nodes(nodes: zui_node_t[]) {
	for (let node of nodes) {
		if (node.type == import_arm_texture_node_name()) {
			node.buttons[0].default_value = base_get_asset_index(node.buttons[0].data);
			node.buttons[0].data = "";
		}
	}
}

function import_arm_unpack_asset(project: project_format_t, abs: string, file: string) {
	if (project_raw.packed_assets == null) {
		project_raw.packed_assets = [];
	}
	for (let pa of project.packed_assets) {
		///if krom_windows
		pa.name = string_replace_all(pa.name, "/", "\\");
		///else
		pa.name = string_replace_all(pa.name, "\\", "/");
		///end
		pa.name = path_normalize(pa.name);
		if (pa.name == file) pa.name = abs; // From relative to absolute
		if (pa.name == abs) {
			if (!project_packed_asset_exists(project_raw.packed_assets, pa.name)) {
				project_raw.packed_assets.push(pa);
			}
			let image: image_t = image_from_encoded_bytes(pa.bytes, pa.name.endsWith(".jpg") ? ".jpg" : ".png");
			data_cached_images.set(abs, image);
			break;
		}
	}
}
