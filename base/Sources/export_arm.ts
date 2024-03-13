
function export_arm_run_mesh(path: string, paint_objects: mesh_object_t[]) {
	let mesh_datas: mesh_data_t[] = [];
	for (let p of paint_objects) mesh_datas.push(p.data);
	let raw: scene_t = { mesh_datas: mesh_datas };
	let b: buffer_t = armpack_encode(raw);
	if (!path.endsWith(".arm")) path += ".arm";
	krom_file_save_bytes(path, b, b.byteLength + 1);
}

function export_arm_run_project() {
	///if (is_paint || is_sculpt)
	let mnodes: zui_node_canvas_t[] = [];
	for (let m of project_materials) {
		let c: zui_node_canvas_t = json_parse(json_stringify(m.canvas));
		for (let n of c.nodes) export_arm_export_node(n);
		mnodes.push(c);
	}

	let bnodes: zui_node_canvas_t[] = [];
	for (let b of project_brushes) bnodes.push(b.canvas);
	///end

	///if is_lab
	let c: zui_node_canvas_t = json_parse(json_stringify(project_canvas));
	for (let n of c.nodes) export_arm_export_node(n);
	///end

	let mgroups: zui_node_canvas_t[] = null;
	if (project_material_groups.length > 0) {
		mgroups = [];
		for (let g of project_material_groups) {
			let c: zui_node_canvas_t = json_parse(json_stringify(g.canvas));
			for (let n of c.nodes) export_arm_export_node(n);
			mgroups.push(c);
		}
	}

	///if (is_paint || is_sculpt)
	let md: mesh_data_t[] = [];
	for (let p of project_paint_objects) md.push(p.data);
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
	for (let l of project_layers) {
		ld.push({
			name: l.name,
			res: l.texpaint != null ? l.texpaint.width : project_layers[0].texpaint.width,
			bpp: bpp,
			texpaint: l.texpaint != null ? lz4_encode(image_get_pixels(l.texpaint)) : null,
			uv_scale: l.scale,
			uv_rot: l.angle,
			uv_type: l.uv_type,
			decal_mat: l.uv_type == uv_type_t.PROJECT ? mat4_to_f32_array(l.decal_mat) : null,
			opacity_mask: l.mask_opacity,
			fill_layer: l.fill_layer != null ? project_materials.indexOf(l.fill_layer) : -1,
			object_mask: l.object_mask,
			blending: l.blending,
			parent: l.parent != null ? project_layers.indexOf(l.parent) : -1,
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
		});
	}
	///end

	let packed_assets: packed_asset_t[] = (project_raw.packed_assets == null || project_raw.packed_assets.length == 0) ? null : project_raw.packed_assets;
	///if krom_ios
	let same_drive: bool = false;
	///else
	let same_drive: bool = project_raw.envmap != null ? project_filepath.charAt(0) == project_raw.envmap.charAt(0) : true;
	///end

	project_raw = {
		version: manifest_version,
		material_groups: mgroups,
		assets: texture_files,
		packed_assets: packed_assets,
		swatches: project_raw.swatches,
		envmap: project_raw.envmap != null ? (same_drive ? path_to_relative(project_filepath, project_raw.envmap) : project_raw.envmap) : null,
		envmap_strength: scene_world.strength,
		camera_world: mat4_to_f32_array(scene_camera.base.transform.local),
		camera_origin: export_arm_vec3f32(camera_origins[0]),
		camera_fov: scene_camera.data.fov,

		///if (is_paint || is_sculpt)
		mesh_datas: md,
		material_nodes: mnodes,
		brush_nodes: bnodes,
		layer_datas: ld,
		font_assets: font_files,
		mesh_assets: mesh_files,
		///end

		///if is_paint
		atlas_objects: project_atlas_objects,
		atlas_names: project_atlas_names,
		///end

		///if is_lab
		mesh_data: md,
		material: c,
		///end

		///if (krom_metal || krom_vulkan)
		is_bgra: true
		///else
		is_bgra: false
		///end
	};

	///if (krom_android || krom_ios)
	let tex: image_t = render_path_render_targets.get(context_raw.render_mode == render_mode_t.FORWARD ? "buf" : "tex")._image;
	let mesh_icon: image_t = image_create_render_target(256, 256);
	let r: f32 = app_w() / app_h();
	g2_begin(mesh_icon);
	///if krom_opengl
	g2_draw_scaled_image(tex, -(256 * r - 256) / 2, 256, 256 * r, -256);
	///else
	g2_draw_scaled_image(tex, -(256 * r - 256) / 2, 0, 256 * r, 256);
	///end
	g2_end();
	///if krom_metal
	// Flush command list
	g2_begin(mesh_icon);
	g2_end();
	///end
	let mesh_icon_pixels: buffer_t = image_get_pixels(mesh_icon);
	let u8a: Uint8Array = new Uint8Array(mesh_icon_pixels);
	for (let i: i32 = 0; i < 256 * 256 * 4; ++i) {
		u8a[i] = math_floor(math_pow(u8a[i] / 255, 1.0 / 2.2) * 255);
	}
	///if (krom_metal || krom_vulkan)
	export_arm_bgra_swap(mesh_icon_pixels);
	///end
	base_notify_on_next_frame(() => {
		image_unload(mesh_icon);
	});
	// raw.mesh_icons =
	// 	///if (krom_metal || krom_vulkan)
	// 	[encode(bgraSwap(mesh_icon_pixels)];
	// 	///else
	// 	[encode(mesh_icon_pixels)];
	// 	///end
	krom_write_png(project_filepath.substr(0, project_filepath.length - 4) + "_icon.png", mesh_icon_pixels, 256, 256, 0);
	///end

	///if (is_paint || is_sculpt)
	let is_packed: bool = project_filepath.endsWith("_packed_.arm");
	if (is_packed) { // Pack textures
		export_arm_pack_assets(project_raw, project_assets);
	}
	///end

	let buffer: buffer_t = armpack_encode(project_raw);
	krom_file_save_bytes(project_filepath, buffer, buffer.byteLength + 1);

	// Save to recent
	///if krom_ios
	let recent_path: string = project_filepath.substr(project_filepath.lastIndexOf("/") + 1);
	///else
	let recent_path: string = project_filepath;
	///end
	let recent: string[] = config_raw.recent_projects;
	array_remove(recent, recent_path);
	recent.unshift(recent_path);
	config_save();

	console_info(tr("Project saved"));
}

function export_arm_texture_node_name(): string {
	///if (is_paint || is_sculpt)
	return "TEX_IMAGE";
	///else
	return "ImageTextureNode";
	///end
}

function export_arm_export_node(n: zui_node_t, assets: asset_t[] = null) {
	if (n.type == export_arm_texture_node_name()) {
		let index: i32 = n.buttons[0].default_value;
		n.buttons[0].data = base_enum_texts(n.type)[index];

		if (assets != null) {
			let asset: asset_t = project_assets[index];
			if (assets.indexOf(asset) == -1) {
				assets.push(asset);
			}
		}
	}
	// Pack colors
	if (n.color > 0) n.color -= 4294967296;
	for (let inp of n.inputs) if (inp.color > 0) inp.color -= 4294967296;
	for (let out of n.outputs) if (out.color > 0) out.color -= 4294967296;
}

///if (is_paint || is_sculpt)
function export_arm_run_material(path: string) {
	if (!path.endsWith(".arm")) path += ".arm";
	let mnodes: zui_node_canvas_t[] = [];
	let mgroups: zui_node_canvas_t[] = null;
	let m: SlotMaterialRaw = context_raw.material;
	let c: zui_node_canvas_t = json_parse(json_stringify(m.canvas));
	let assets: asset_t[] = [];
	if (ui_nodes_has_group(c)) {
		mgroups = [];
		ui_nodes_traverse_group(mgroups, c);
		for (let gc of mgroups) for (let n of gc.nodes) export_arm_export_node(n, assets);
	}
	for (let n of c.nodes) export_arm_export_node(n, assets);
	mnodes.push(c);

	let texture_files: string[] = export_arm_assets_to_files(path, assets);
	let is_cloud: bool = path.endsWith("_cloud_.arm");
	if (is_cloud) path = string_replace_all(path, "_cloud_", "");
	let packed_assets: packed_asset_t[] = null;
	if (!context_raw.pack_assets_on_export) {
		packed_assets = export_arm_get_packed_assets(path, texture_files);
	}

	let raw: project_format_t = {
		version: manifest_version,
		material_nodes: mnodes,
		material_groups: mgroups,
		material_icons: is_cloud ? null :
			///if (krom_metal || krom_vulkan)
			[lz4_encode(export_arm_bgra_swap(image_get_pixels(m.image)))],
			///else
			[lz4_encode(image_get_pixels(m.image))],
			///end
		assets: texture_files,
		packed_assets: packed_assets
	};

	if (context_raw.write_icon_on_export) { // Separate icon files
		krom_write_png(path.substr(0, path.length - 4) + "_icon.png", image_get_pixels(m.image), m.image.width, m.image.height, 0);
		if (is_cloud) {
			krom_write_jpg(path.substr(0, path.length - 4) + "_icon.jpg", image_get_pixels(m.image), m.image.width, m.image.height, 0, 50);
		}
	}

	if (context_raw.pack_assets_on_export) { // Pack textures
		export_arm_pack_assets(raw, assets);
	}

	let buffer: buffer_t = armpack_encode(raw);
	krom_file_save_bytes(path, buffer, buffer.byteLength + 1);
}
///end

///if (krom_metal || krom_vulkan)
function export_arm_bgra_swap(buffer: ArrayBuffer) {
	let view: DataView = new DataView(buffer);
	for (let i: i32 = 0; i < math_floor(buffer.byteLength / 4); ++i) {
		let r: i32 = view.getUint8(i * 4);
		view.setUint8(i * 4, view.getUint8(i * 4 + 2));
		view.setUint8(i * 4 + 2, r);
	}
	return buffer;
}
///end

///if (is_paint || is_sculpt)
function export_arm_run_brush(path: string) {
	if (!path.endsWith(".arm")) path += ".arm";
	let bnodes: zui_node_canvas_t[] = [];
	let b: SlotBrushRaw = context_raw.brush;
	let c: zui_node_canvas_t = json_parse(json_stringify(b.canvas));
	let assets: asset_t[] = [];
	for (let n of c.nodes) export_arm_export_node(n, assets);
	bnodes.push(c);

	let texture_files: string[] = export_arm_assets_to_files(path, assets);
	let is_cloud: bool = path.endsWith("_cloud_.arm");
	if (is_cloud) path = string_replace_all(path, "_cloud_", "");
	let packed_assets: packed_asset_t[] = null;
	if (!context_raw.pack_assets_on_export) {
		packed_assets = export_arm_get_packed_assets(path, texture_files);
	}

	let raw: project_format_t = {
		version: manifest_version,
		brush_nodes: bnodes,
		brush_icons: is_cloud ? null :
		///if (krom_metal || krom_vulkan)
		[lz4_encode(export_arm_bgra_swap(image_get_pixels(b.image)))],
		///else
		[lz4_encode(image_get_pixels(b.image))],
		///end
		assets: texture_files,
		packed_assets: packed_assets
	};

	if (context_raw.write_icon_on_export) { // Separate icon file
		krom_write_png(path.substr(0, path.length - 4) + "_icon.png", image_get_pixels(b.image), b.image.width, b.image.height, 0);
	}

	if (context_raw.pack_assets_on_export) { // Pack textures
		export_arm_pack_assets(raw, assets);
	}

	let buffer: buffer_t = armpack_encode(raw);
	krom_file_save_bytes(path, buffer, buffer.byteLength + 1);
}
///end

function export_arm_assets_to_files(projectPath: string, assets: asset_t[]): string[] {
	let texture_files: string[] = [];
	for (let a of assets) {
		///if krom_ios
		let same_drive: bool = false;
		///else
		let same_drive: bool = projectPath.charAt(0) == a.file.charAt(0);
		///end
		// Convert image path from absolute to relative
		if (same_drive) {
			texture_files.push(path_to_relative(projectPath, a.file));
		}
		else {
			texture_files.push(a.file);
		}
	}
	return texture_files;
}

///if (is_paint || is_sculpt)
function export_arm_meshes_to_files(project_path: string): string[] {
	let mesh_files: string[] = [];
	for (let file of project_mesh_assets) {
		///if krom_ios
		let same_drive: bool = false;
		///else
		let same_drive: bool = project_path.charAt(0) == file.charAt(0);
		///end
		// Convert mesh path from absolute to relative
		if (same_drive) {
			mesh_files.push(path_to_relative(project_path, file));
		}
		else {
			mesh_files.push(file);
		}
	}
	return mesh_files;
}

function export_arm_fonts_to_files(project_path: string, fonts: SlotFontRaw[]): string[] {
	let font_files: string[] = [];
	for (let i = 1; i <fonts.length; ++i) {
		let f: SlotFontRaw = fonts[i];
		///if krom_ios
		let same_drive: bool = false;
		///else
		let same_drive: bool = project_path.charAt(0) == f.file.charAt(0);
		///end
		// Convert font path from absolute to relative
		if (same_drive) {
			font_files.push(path_to_relative(project_path, f.file));
		}
		else {
			font_files.push(f.file);
		}
	}
	return font_files;
}
///end

function export_arm_get_packed_assets(project_path: string, texture_files: string[]): packed_asset_t[] {
	let packed_assets: packed_asset_t[] = null;
	if (project_raw.packed_assets != null) {
		for (let pa of project_raw.packed_assets) {
			///if krom_ios
			let same_drive: bool = false;
			///else
			let same_drive: bool = project_path.charAt(0) == pa.name.charAt(0);
			///end
			// Convert path from absolute to relative
			pa.name = same_drive ? path_to_relative(project_path, pa.name) : pa.name;
			for (let tf of texture_files) {
				if (pa.name == tf) {
					if (packed_assets == null) {
						packed_assets = [];
					}
					packed_assets.push(pa);
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
			temp_images.push(temp);
			raw.packed_assets.push({
				name: assets[i].file,
				bytes: assets[i].file.endsWith(".jpg") ?
					krom_encode_jpg(image_get_pixels(temp), temp.width, temp.height, 0, 80) :
					krom_encode_png(image_get_pixels(temp), temp.width, temp.height, 0)
			});
		}
	}
	base_notify_on_next_frame(() => {
		for (let image of temp_images) image_unload(image);
	});
}

function export_arm_run_swatches(path: string) {
	if (!path.endsWith(".arm")) path += ".arm";
	let raw: any = {
		version: manifest_version,
		swatches: project_raw.swatches
	};
	let buffer: buffer_t = armpack_encode(raw);
	krom_file_save_bytes(path, buffer, buffer.byteLength + 1);
}

function export_arm_vec3f32(v: vec4_t): Float32Array {
	let res: Float32Array = new Float32Array(3);
	res[0] = v.x;
	res[1] = v.y;
	res[2] = v.z;
	return res;
}
