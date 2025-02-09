
function export_arm_run_mesh(path: string, paint_objects: mesh_object_t[]) {
	let mesh_datas: mesh_data_t[] = [];
	for (let i: i32 = 0; i < paint_objects.length; ++i) {
		let p: mesh_object_t = paint_objects[i];
		array_push(mesh_datas, p.data);
	}

	let raw: scene_t = {
		mesh_datas: mesh_datas
	};
	let b: buffer_t = util_encode_scene(raw);

	if (!ends_with(path, ".arm")) {
		path += ".arm";
	}
	iron_file_save_bytes(path, b, b.length + 1);
}

function export_arm_run_project() {
	///if (is_paint || is_sculpt)
	let mnodes: ui_node_canvas_t[] = [];
	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		let c: ui_node_canvas_t = util_clone_canvas(m.canvas);
		for (let i: i32 = 0; i < c.nodes.length; ++i) {
			let n: ui_node_t = c.nodes[i];
			export_arm_export_node(n);
		}
		array_push(mnodes, c);
	}

	let bnodes: ui_node_canvas_t[] = [];
	for (let i: i32 = 0; i < project_brushes.length; ++i) {
		let b: slot_brush_t = project_brushes[i];
		array_push(bnodes, b.canvas);
	}
	///end

	///if is_lab
	let c: ui_node_canvas_t = util_clone_canvas(project_canvas);
	for (let i: i32 = 0; i < c.nodes.length; ++i) {
		let n: ui_node_t = c.nodes[i];
		export_arm_export_node(n);
	}
	///end

	let mgroups: ui_node_canvas_t[] = null;
	if (project_material_groups.length > 0) {
		mgroups = [];
		for (let i: i32 = 0; i < project_material_groups.length; ++i) {
			let g: node_group_t = project_material_groups[i];
			let c: ui_node_canvas_t = util_clone_canvas(g.canvas);
			for (let i: i32 = 0; i < c.nodes.length; ++i) {
				let n: ui_node_t = c.nodes[i];
				export_arm_export_node(n);
			}
			array_push(mgroups, c);
		}
	}

	///if (is_paint || is_sculpt)
	let md: mesh_data_t[] = [];
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		array_push(md, p.data);
	}
	///end

	///if is_lab
	let md: mesh_data_t = project_paint_objects[0].data;
	///end

	let texture_files: string[] = export_arm_assets_to_files(project_filepath, project_assets);

	///if (is_paint || is_sculpt)
	let font_files: string[] = export_arm_fonts_to_files(project_filepath, project_fonts);
	let mesh_files: string[] = export_arm_meshes_to_files(project_filepath);

	let bits_pos: i32 = base_bits_handle.position;
	let bpp: i32 = bits_pos == texture_bits_t.BITS8 ? 8 : bits_pos == texture_bits_t.BITS16 ? 16 : 32;

	let ld: layer_data_t[] = [];
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		let d: layer_data_t = {
			name: l.name,
			res: l.texpaint != null ? l.texpaint.width : project_layers[0].texpaint.width,
			bpp: bpp,
			texpaint: l.texpaint != null ? lz4_encode(image_get_pixels(l.texpaint)) : null,
			uv_scale: l.scale,
			uv_rot: l.angle,
			uv_type: l.uv_type,
			decal_mat: l.uv_type == uv_type_t.PROJECT ? mat4_to_f32_array(l.decal_mat) : null,
			opacity_mask: l.mask_opacity,
			fill_layer: l.fill_layer != null ? array_index_of(project_materials, l.fill_layer) : -1,
			object_mask: l.object_mask,
			blending: l.blending,
			parent: l.parent != null ? array_index_of(project_layers, l.parent) : -1,
			visible: l.visible,
			///if is_paint
			texpaint_nor: l.texpaint_nor != null ? lz4_encode(image_get_pixels(l.texpaint_nor)) : null,
			texpaint_pack: l.texpaint_pack != null ? lz4_encode(image_get_pixels(l.texpaint_pack)) : null,
			paint_base: l.paint_base,
			paint_opac: l.paint_opac,
			paint_occ: l.paint_occ,
			paint_rough: l.paint_rough,
			paint_met: l.paint_met,
			paint_nor: l.paint_nor,
			paint_nor_blend: l.paint_nor_blend,
			paint_height: l.paint_height,
			paint_height_blend: l.paint_height_blend,
			paint_emis: l.paint_emis,
			paint_subs: l.paint_subs
			///end
		};
		array_push(ld, d);
	}
	///end

	let packed_assets: packed_asset_t[] = (project_raw.packed_assets == null || project_raw.packed_assets.length == 0) ? null : project_raw.packed_assets;
	///if arm_ios
	let same_drive: bool = false;
	///else
	let same_drive: bool = project_raw.envmap != null ? char_at(project_filepath, 0) == char_at(project_raw.envmap, 0) : true;
	///end

	project_raw.version = manifest_version;
	project_raw.material_groups = mgroups;
	project_raw.assets = texture_files;
	project_raw.packed_assets = packed_assets;
	project_raw.swatches = project_raw.swatches;
	project_raw.envmap = project_raw.envmap != null ?
		(same_drive ?
			path_to_relative(project_filepath, project_raw.envmap) :
			project_raw.envmap) :
		null;
	project_raw.envmap_strength = scene_world.strength;
	project_raw.camera_world = mat4_to_f32_array(scene_camera.base.transform.local);
	project_raw.camera_origin = export_arm_vec3f32(camera_origins[0].v);
	project_raw.camera_fov = scene_camera.data.fov;

	///if (is_paint || is_sculpt)
	// project_raw.mesh_datas = md; // TODO: fix GC ref
	if (project_raw.mesh_datas == null) {
		project_raw.mesh_datas = md;
	}
	else {
		project_raw.mesh_datas.length = 0;
		for (let i: i32 = 0; i < md.length; ++i) {
			array_push(project_raw.mesh_datas, md[i]);
		}
	}

	project_raw.material_nodes = mnodes;
	project_raw.brush_nodes = bnodes;
	project_raw.layer_datas = ld;
	project_raw.font_assets = font_files;
	project_raw.mesh_assets = mesh_files;
	project_raw.atlas_objects = project_atlas_objects;
	project_raw.atlas_names = project_atlas_names;
	///end

	///if is_lab
	project_raw.mesh_data = md;
	project_raw.material = c;
	///end

	///if (arm_metal || arm_vulkan)
	project_raw.is_bgra = true;
	///else
	project_raw.is_bgra = false;
	///end

	///if (arm_android || arm_ios)
	let rt: render_target_t = map_get(render_path_render_targets, context_raw.render_mode == render_mode_t.FORWARD ? "buf" : "tex");
	let tex: image_t = rt._image;
	let mesh_icon: image_t = image_create_render_target(256, 256);
	let r: f32 = app_w() / app_h();
	g2_begin(mesh_icon);
	g2_draw_scaled_image(tex, -(256 * r - 256) / 2, 0, 256 * r, 256);
	g2_end();

	///if arm_metal
	// Flush command list
	g2_begin(mesh_icon);
	g2_end();
	///end

	let mesh_icon_pixels: buffer_t = image_get_pixels(mesh_icon);
	let u8a: u8_array_t = mesh_icon_pixels;
	for (let i: i32 = 0; i < 256 * 256 * 4; ++i) {
		u8a[i] = math_floor(math_pow(u8a[i] / 255, 1.0 / 2.2) * 255);
	}
	///if (arm_metal || arm_vulkan)
	export_arm_bgra_swap(mesh_icon_pixels);
	///end

	app_notify_on_next_frame(function (mesh_icon: image_t) {
		image_unload(mesh_icon);
	});

	// raw.mesh_icons =
	// 	///if (arm_metal || arm_vulkan)
	// 	[encode(bgra_swap(mesh_icon_pixels)];
	// 	///else
	// 	[encode(mesh_icon_pixels)];
	// 	///end
	iron_write_png(substring(project_filepath, 0, project_filepath.length - 4) + "_icon.png", mesh_icon_pixels, 256, 256, 0);
	///end

	///if (is_paint || is_sculpt)
	let is_packed: bool = ends_with(project_filepath, "_packed_.arm");
	if (is_packed) { // Pack textures
		export_arm_pack_assets(project_raw, project_assets);
	}
	///end

	let buffer: buffer_t = util_encode_project(project_raw);
	iron_file_save_bytes(project_filepath, buffer, buffer.length + 1);

	// Save to recent
	///if arm_ios
	let recent_path: string = substring(project_filepath, string_last_index_of(project_filepath, "/") + 1, project_filepath.length);
	///else
	let recent_path: string = project_filepath;
	///end

	///if arm_windows
	recent_path = string_replace_all(recent_path, "\\", "/");
	///end
	let recent: string[] = config_raw.recent_projects;
	array_remove(recent, recent_path);
	array_insert(recent, 0, recent_path);
	config_save();

	console_info(tr("Project saved"));
}

function export_arm_texture_node_name(): string {
	///if (is_paint || is_sculpt)
	return "TEX_IMAGE";
	///else
	return "image_texture_node";
	///end
}

function export_arm_export_node(n: ui_node_t, assets: asset_t[] = null) {
	if (n.type == export_arm_texture_node_name()) {
		let index: i32 = n.buttons[0].default_value[0];
		n.buttons[0].data = u8_array_create_from_string(base_enum_texts(n.type)[index]);

		if (assets != null) {
			let asset: asset_t = project_assets[index];
			if (array_index_of(assets, asset) == -1) {
				array_push(assets, asset);
			}
		}
	}
}

function export_arm_run_material(path: string) {
	if (!ends_with(path, ".arm")) {
		path += ".arm";
	}
	let mnodes: ui_node_canvas_t[] = [];
	let mgroups: ui_node_canvas_t[] = null;
	let m: slot_material_t = context_raw.material;
	let c: ui_node_canvas_t = util_clone_canvas(m.canvas);
	let assets: asset_t[] = [];
	if (ui_nodes_has_group(c)) {
		mgroups = [];
		ui_nodes_traverse_group(mgroups, c);
		for (let i: i32 = 0; i < mgroups.length; ++i) {
			let gc: ui_node_canvas_t = mgroups[i];
			for (let i: i32 = 0; i < gc.nodes.length; ++i) {
				let n: ui_node_t = gc.nodes[i];
				export_arm_export_node(n, assets);
			}
		}
	}
	for (let i: i32 = 0; i < c.nodes.length; ++i) {
		let n: ui_node_t = c.nodes[i];
		export_arm_export_node(n, assets);
	}
	array_push(mnodes, c);

	let texture_files: string[] = export_arm_assets_to_files(path, assets);
	let is_cloud: bool = ends_with(path, "_cloud_.arm");
	if (is_cloud) {
		path = string_replace_all(path, "_cloud_", "");
	}
	let packed_assets: packed_asset_t[] = null;
	if (!context_raw.pack_assets_on_export) {
		packed_assets = export_arm_get_packed_assets(path, texture_files);
	}

	let micons: buffer_t[] = null;
	if (!is_cloud) {
		///if (arm_metal || arm_vulkan)
		let buf: buffer_t = lz4_encode(export_arm_bgra_swap(image_get_pixels(m.image)));
		///else
		let buf: buffer_t = lz4_encode(image_get_pixels(m.image));
		///end
		micons = [buf];
	}

	let raw: project_format_t = {
		version: manifest_version,
		material_nodes: mnodes,
		material_groups: mgroups,
		material_icons: micons,
		assets: texture_files,
		packed_assets: packed_assets
	};

	if (context_raw.write_icon_on_export) { // Separate icon files
		iron_write_png(substring(path, 0, path.length - 4) + "_icon.png", image_get_pixels(m.image), m.image.width, m.image.height, 0);
		if (is_cloud) {
			iron_write_jpg(substring(path, 0, path.length - 4) + "_icon.jpg", image_get_pixels(m.image), m.image.width, m.image.height, 0, 50);
		}
	}

	if (context_raw.pack_assets_on_export) { // Pack textures
		export_arm_pack_assets(raw, assets);
	}

	let buffer: buffer_t = util_encode_project(raw);
	iron_file_save_bytes(path, buffer, buffer.length + 1);
}

function export_arm_bgra_swap(buffer: buffer_t): buffer_t {
	for (let i: i32 = 0; i < math_floor((buffer.length) / 4); ++i) {
		let r: i32 = buffer[i * 4];
		buffer[i * 4] = buffer[i * 4 + 2];
		buffer[i * 4 + 2] = r;
	}
	return buffer;
}

function export_arm_run_brush(path: string) {
	if (!ends_with(path, ".arm")) {
		path += ".arm";
	}
	let bnodes: ui_node_canvas_t[] = [];
	let b: slot_brush_t = context_raw.brush;
	let c: ui_node_canvas_t = util_clone_canvas(b.canvas);
	let assets: asset_t[] = [];
	for (let i: i32 = 0; i < c.nodes.length; ++i) {
		let n: ui_node_t = c.nodes[i];
		export_arm_export_node(n, assets);
	}
	array_push(bnodes, c);

	let texture_files: string[] = export_arm_assets_to_files(path, assets);
	let is_cloud: bool = ends_with(path, "_cloud_.arm");
	if (is_cloud) {
		path = string_replace_all(path, "_cloud_", "");
	}
	let packed_assets: packed_asset_t[] = null;
	if (!context_raw.pack_assets_on_export) {
		packed_assets = export_arm_get_packed_assets(path, texture_files);
	}

	let bicons: buffer_t[] = null;
	if (!is_cloud) {
		///if (arm_metal || arm_vulkan)
		let buf: buffer_t = lz4_encode(export_arm_bgra_swap(image_get_pixels(b.image)));
		///else
		let buf: buffer_t = lz4_encode(image_get_pixels(b.image));
		///end
		bicons = [buf];
	}

	let raw: project_format_t = {
		version: manifest_version,
		brush_nodes: bnodes,
		brush_icons: bicons,
		assets: texture_files,
		packed_assets: packed_assets
	};

	if (context_raw.write_icon_on_export) { // Separate icon file
		iron_write_png(substring(path, 0, path.length - 4) + "_icon.png", image_get_pixels(b.image), b.image.width, b.image.height, 0);
	}

	if (context_raw.pack_assets_on_export) { // Pack textures
		export_arm_pack_assets(raw, assets);
	}

	let buffer: buffer_t = util_encode_project(raw);
	iron_file_save_bytes(path, buffer, buffer.length + 1);
}

function export_arm_assets_to_files(project_path: string, assets: asset_t[]): string[] {
	let texture_files: string[] = [];
	for (let i: i32 = 0; i < assets.length; ++i) {
		let a: asset_t = assets[i];
		///if arm_ios
		let same_drive: bool = false;
		///else
		let same_drive: bool = char_at(project_path, 0) == char_at(a.file, 0);
		///end
		// Convert image path from absolute to relative
		if (same_drive) {
			array_push(texture_files, path_to_relative(project_path, a.file));
		}
		else {
			array_push(texture_files, a.file);
		}
	}
	return texture_files;
}

function export_arm_meshes_to_files(project_path: string): string[] {
	let mesh_files: string[] = [];
	for (let i: i32 = 0; i < project_mesh_assets.length; ++i) {
		let file: string = project_mesh_assets[i];
		///if arm_ios
		let same_drive: bool = false;
		///else
		let same_drive: bool = char_at(project_path, 0) == char_at(file, 0);
		///end
		// Convert mesh path from absolute to relative
		if (same_drive) {
			array_push(mesh_files, path_to_relative(project_path, file));
		}
		else {
			array_push(mesh_files, file);
		}
	}
	return mesh_files;
}

function export_arm_fonts_to_files(project_path: string, fonts: slot_font_t[]): string[] {
	let font_files: string[] = [];
	for (let i: i32 = 1; i < fonts.length; ++i) {
		let f: slot_font_t = fonts[i];
		///if arm_ios
		let same_drive: bool = false;
		///else
		let same_drive: bool = char_at(project_path, 0) == char_at(f.file, 0);
		///end
		// Convert font path from absolute to relative
		if (same_drive) {
			array_push(font_files, path_to_relative(project_path, f.file));
		}
		else {
			array_push(font_files, f.file);
		}
	}
	return font_files;
}

function export_arm_get_packed_assets(project_path: string, texture_files: string[]): packed_asset_t[] {
	let packed_assets: packed_asset_t[] = null;
	if (project_raw.packed_assets != null) {
		for (let i: i32 = 0; i < project_raw.packed_assets.length; ++i) {
			let pa: packed_asset_t = project_raw.packed_assets[i];
			///if arm_ios
			let same_drive: bool = false;
			///else
			let same_drive: bool = char_at(project_path, 0) == char_at(pa.name, 0);
			///end
			// Convert path from absolute to relative
			pa.name = same_drive ? path_to_relative(project_path, pa.name) : pa.name;
			for (let i: i32 = 0; i < texture_files.length; ++i) {
				let tf: string = texture_files[i];
				if (pa.name == tf) {
					if (packed_assets == null) {
						packed_assets = [];
					}
					array_push(packed_assets, pa);
					break;
				}
			}
		}
	}
	return packed_assets;
}

function export_arm_pack_assets(raw: project_format_t, assets: asset_t[]) {
	if (raw.packed_assets == null) {
		raw.packed_assets = [];
	}
	let temp_images: image_t[] = [];
	for (let i: i32 = 0; i < assets.length; ++i) {
		if (!project_packed_asset_exists(raw.packed_assets, assets[i].file)) {
			let image: image_t = project_get_image(assets[i]);
			let temp: image_t = image_create_render_target(image.width, image.height);
			g2_begin(temp);
			g2_draw_image(image, 0, 0);
			g2_end();
			array_push(temp_images, temp);
			let pa: packed_asset_t = {
				name: assets[i].file,
				bytes: ends_with(assets[i].file, ".jpg") ?
					iron_encode_jpg(image_get_pixels(temp), temp.width, temp.height, 0, 80) :
					iron_encode_png(image_get_pixels(temp), temp.width, temp.height, 0)
			};
			array_push(raw.packed_assets, pa);
		}
	}
	app_notify_on_next_frame(function (temp_images: image_t[]) {
		for (let i: i32 = 0; i < temp_images.length; ++i) {
			let image: image_t = temp_images[i];
			image_unload(image);
		}
	}, temp_images);
}

function export_arm_run_swatches(path: string) {
	if (!ends_with(path, ".arm")) {
		path += ".arm";
	}
	let raw: project_format_t = {
		version: manifest_version,
		swatches: project_raw.swatches
	};
	let buffer: buffer_t = util_encode_project(raw);
	iron_file_save_bytes(path, buffer, buffer.length + 1);
}

function export_arm_vec3f32(v: vec4_t): f32_array_t {
	let res: f32_array_t = f32_array_create(3);
	res[0] = v.x;
	res[1] = v.y;
	res[2] = v.z;
	return res;
}
