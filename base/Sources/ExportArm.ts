
class ExportArm {

	static run_mesh = (path: string, paintObjects: mesh_object_t[]) => {
		let mesh_datas: mesh_data_t[] = [];
		for (let p of paintObjects) mesh_datas.push(p.data);
		let raw: scene_t = { mesh_datas: mesh_datas };
		let b: buffer_t = armpack_encode(raw);
		if (!path.endsWith(".arm")) path += ".arm";
		krom_file_save_bytes(path, b, b.byteLength + 1);
	}

	static run_project = () => {
		///if (is_paint || is_sculpt)
		let mnodes: zui_node_canvas_t[] = [];
		for (let m of Project.materials) {
			let c: zui_node_canvas_t = JSON.parse(JSON.stringify(m.canvas));
			for (let n of c.nodes) ExportArm.export_node(n);
			mnodes.push(c);
		}

		let bnodes: zui_node_canvas_t[] = [];
		for (let b of Project.brushes) bnodes.push(b.canvas);
		///end

		///if is_lab
		let c: zui_node_canvas_t = JSON.parse(JSON.stringify(Project.canvas));
		for (let n of c.nodes) ExportArm.export_node(n);
		///end

		let mgroups: zui_node_canvas_t[] = null;
		if (Project.material_groups.length > 0) {
			mgroups = [];
			for (let g of Project.material_groups) {
				let c: zui_node_canvas_t = JSON.parse(JSON.stringify(g.canvas));
				for (let n of c.nodes) ExportArm.export_node(n);
				mgroups.push(c);
			}
		}

		///if (is_paint || is_sculpt)
		let md: mesh_data_t[] = [];
		for (let p of Project.paint_objects) md.push(p.data);
		///end

		///if is_lab
		let md: mesh_data_t = Project.paint_objects[0].data;
		///end

		let texture_files: string[] = ExportArm.assets_to_files(Project.filepath, Project.assets);

		///if (is_paint || is_sculpt)
		let font_files: string[] = ExportArm.fonts_to_files(Project.filepath, Project.fonts);
		let mesh_files: string[] = ExportArm.meshes_to_files(Project.filepath);

		let bitsPos: i32 = Base.bits_handle.position;
		let bpp: i32 = bitsPos == texture_bits_t.BITS8 ? 8 : bitsPos == texture_bits_t.BITS16 ? 16 : 32;

		let ld: layer_data_t[] = [];
		for (let l of Project.layers) {
			ld.push({
				name: l.name,
				res: l.texpaint != null ? l.texpaint.width : Project.layers[0].texpaint.width,
				bpp: bpp,
				texpaint: l.texpaint != null ? lz4_encode(image_get_pixels(l.texpaint)) : null,
				uv_scale: l.scale,
				uv_rot: l.angle,
				uv_type: l.uvType,
				decal_mat: l.uvType == uv_type_t.PROJECT ? mat4_to_f32_array(l.decalMat) : null,
				opacity_mask: l.maskOpacity,
				fill_layer: l.fill_layer != null ? Project.materials.indexOf(l.fill_layer) : -1,
				object_mask: l.objectMask,
				blending: l.blending,
				parent: l.parent != null ? Project.layers.indexOf(l.parent) : -1,
				visible: l.visible,
				///if is_paint
				texpaint_nor: l.texpaint_nor != null ? lz4_encode(image_get_pixels(l.texpaint_nor)) : null,
				texpaint_pack: l.texpaint_pack != null ? lz4_encode(image_get_pixels(l.texpaint_pack)) : null,
				paint_base: l.paintBase,
				paint_opac: l.paintOpac,
				paint_occ: l.paintOcc,
				paint_rough: l.paintRough,
				paint_met: l.paintMet,
				paint_nor: l.paintNor,
				paint_nor_blend: l.paintNorBlend,
				paint_height: l.paintHeight,
				paint_height_blend: l.paintHeightBlend,
				paint_emis: l.paintEmis,
				paint_subs: l.paintSubs
				///end
			});
		}
		///end

		let packed_assets: packed_asset_t[] = (Project.raw.packed_assets == null || Project.raw.packed_assets.length == 0) ? null : Project.raw.packed_assets;
		///if krom_ios
		let sameDrive: bool = false;
		///else
		let sameDrive: bool = Project.raw.envmap != null ? Project.filepath.charAt(0) == Project.raw.envmap.charAt(0) : true;
		///end

		Project.raw = {
			version: manifest_version,
			material_groups: mgroups,
			assets: texture_files,
			packed_assets: packed_assets,
			swatches: Project.raw.swatches,
			envmap: Project.raw.envmap != null ? (sameDrive ? Path.to_relative(Project.filepath, Project.raw.envmap) : Project.raw.envmap) : null,
			envmap_strength: scene_world.strength,
			camera_world: mat4_to_f32_array(scene_camera.base.transform.local),
			camera_origin: ExportArm.vec3f32(Camera.origins[0]),
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
			atlas_objects: Project.atlas_objects,
			atlas_names: Project.atlas_names,
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
		let tex: image_t = render_path_render_targets.get(Context.raw.render_mode == render_mode_t.FORWARD ? "buf" : "tex")._image;
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
			u8a[i] = Math.floor(Math.pow(u8a[i] / 255, 1.0 / 2.2) * 255);
		}
		///if (krom_metal || krom_vulkan)
		ExportArm.bgra_swap(mesh_icon_pixels);
		///end
		Base.notify_on_next_frame(() => {
			image_unload(mesh_icon);
		});
		// Project.raw.mesh_icons =
		// 	///if (krom_metal || krom_vulkan)
		// 	[encode(bgraSwap(mesh_icon_pixels)];
		// 	///else
		// 	[encode(mesh_icon_pixels)];
		// 	///end
		krom_write_png(Project.filepath.substr(0, Project.filepath.length - 4) + "_icon.png", mesh_icon_pixels, 256, 256, 0);
		///end

		///if (is_paint || is_sculpt)
		let isPacked: bool = Project.filepath.endsWith("_packed_.arm");
		if (isPacked) { // Pack textures
			ExportArm.pack_assets(Project.raw, Project.assets);
		}
		///end

		let buffer: buffer_t = armpack_encode(Project.raw);
		krom_file_save_bytes(Project.filepath, buffer, buffer.byteLength + 1);

		// Save to recent
		///if krom_ios
		let recent_path: string = Project.filepath.substr(Project.filepath.lastIndexOf("/") + 1);
		///else
		let recent_path: string = Project.filepath;
		///end
		let recent: string[] = Config.raw.recent_projects;
		array_remove(recent, recent_path);
		recent.unshift(recent_path);
		Config.save();

		Console.info(tr("Project saved"));
	}

	static texture_node_name = (): string => {
		///if (is_paint || is_sculpt)
		return "TEX_IMAGE";
		///else
		return "ImageTextureNode";
		///end
	}

	static export_node = (n: zui_node_t, assets: asset_t[] = null) => {
		if (n.type == ExportArm.texture_node_name()) {
			let index: i32 = n.buttons[0].default_value;
			n.buttons[0].data = Base.enum_texts(n.type)[index];

			if (assets != null) {
				let asset: asset_t = Project.assets[index];
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
	static run_material = (path: string) => {
		if (!path.endsWith(".arm")) path += ".arm";
		let mnodes: zui_node_canvas_t[] = [];
		let mgroups: zui_node_canvas_t[] = null;
		let m: SlotMaterialRaw = Context.raw.material;
		let c: zui_node_canvas_t = JSON.parse(JSON.stringify(m.canvas));
		let assets: asset_t[] = [];
		if (UINodes.has_group(c)) {
			mgroups = [];
			UINodes.traverse_group(mgroups, c);
			for (let gc of mgroups) for (let n of gc.nodes) ExportArm.export_node(n, assets);
		}
		for (let n of c.nodes) ExportArm.export_node(n, assets);
		mnodes.push(c);

		let texture_files: string[] = ExportArm.assets_to_files(path, assets);
		let isCloud: bool = path.endsWith("_cloud_.arm");
		if (isCloud) path = string_replace_all(path, "_cloud_", "");
		let packed_assets: packed_asset_t[] = null;
		if (!Context.raw.pack_assets_on_export) {
			packed_assets = ExportArm.get_packed_assets(path, texture_files);
		}

		let raw: project_format_t = {
			version: manifest_version,
			material_nodes: mnodes,
			material_groups: mgroups,
			material_icons: isCloud ? null :
				///if (krom_metal || krom_vulkan)
				[lz4_encode(ExportArm.bgra_swap(image_get_pixels(m.image)))],
				///else
				[lz4_encode(image_get_pixels(m.image))],
				///end
			assets: texture_files,
			packed_assets: packed_assets
		};

		if (Context.raw.write_icon_on_export) { // Separate icon files
			krom_write_png(path.substr(0, path.length - 4) + "_icon.png", image_get_pixels(m.image), m.image.width, m.image.height, 0);
			if (isCloud) {
				krom_write_jpg(path.substr(0, path.length - 4) + "_icon.jpg", image_get_pixels(m.image), m.image.width, m.image.height, 0, 50);
			}
		}

		if (Context.raw.pack_assets_on_export) { // Pack textures
			ExportArm.pack_assets(raw, assets);
		}

		let buffer: buffer_t = armpack_encode(raw);
		krom_file_save_bytes(path, buffer, buffer.byteLength + 1);
	}
	///end

	///if (krom_metal || krom_vulkan)
	static bgra_swap = (buffer: ArrayBuffer) => {
		let view: DataView = new DataView(buffer);
		for (let i: i32 = 0; i < Math.floor(buffer.byteLength / 4); ++i) {
			let r: i32 = view.getUint8(i * 4);
			view.setUint8(i * 4, view.getUint8(i * 4 + 2));
			view.setUint8(i * 4 + 2, r);
		}
		return buffer;
	}
	///end

	///if (is_paint || is_sculpt)
	static run_brush = (path: string) => {
		if (!path.endsWith(".arm")) path += ".arm";
		let bnodes: zui_node_canvas_t[] = [];
		let b: SlotBrushRaw = Context.raw.brush;
		let c: zui_node_canvas_t = JSON.parse(JSON.stringify(b.canvas));
		let assets: asset_t[] = [];
		for (let n of c.nodes) ExportArm.export_node(n, assets);
		bnodes.push(c);

		let texture_files: string[] = ExportArm.assets_to_files(path, assets);
		let isCloud: bool = path.endsWith("_cloud_.arm");
		if (isCloud) path = string_replace_all(path, "_cloud_", "");
		let packed_assets: packed_asset_t[] = null;
		if (!Context.raw.pack_assets_on_export) {
			packed_assets = ExportArm.get_packed_assets(path, texture_files);
		}

		let raw: project_format_t = {
			version: manifest_version,
			brush_nodes: bnodes,
			brush_icons: isCloud ? null :
			///if (krom_metal || krom_vulkan)
			[lz4_encode(ExportArm.bgra_swap(image_get_pixels(b.image)))],
			///else
			[lz4_encode(image_get_pixels(b.image))],
			///end
			assets: texture_files,
			packed_assets: packed_assets
		};

		if (Context.raw.write_icon_on_export) { // Separate icon file
			krom_write_png(path.substr(0, path.length - 4) + "_icon.png", image_get_pixels(b.image), b.image.width, b.image.height, 0);
		}

		if (Context.raw.pack_assets_on_export) { // Pack textures
			ExportArm.pack_assets(raw, assets);
		}

		let buffer: buffer_t = armpack_encode(raw);
		krom_file_save_bytes(path, buffer, buffer.byteLength + 1);
	}
	///end

	static assets_to_files = (projectPath: string, assets: asset_t[]): string[] => {
		let texture_files: string[] = [];
		for (let a of assets) {
			///if krom_ios
			let sameDrive: bool = false;
			///else
			let sameDrive: bool = projectPath.charAt(0) == a.file.charAt(0);
			///end
			// Convert image path from absolute to relative
			if (sameDrive) {
				texture_files.push(Path.to_relative(projectPath, a.file));
			}
			else {
				texture_files.push(a.file);
			}
		}
		return texture_files;
	}

	///if (is_paint || is_sculpt)
	static meshes_to_files = (projectPath: string): string[] => {
		let mesh_files: string[] = [];
		for (let file of Project.mesh_assets) {
			///if krom_ios
			let sameDrive: bool = false;
			///else
			let sameDrive: bool = projectPath.charAt(0) == file.charAt(0);
			///end
			// Convert mesh path from absolute to relative
			if (sameDrive) {
				mesh_files.push(Path.to_relative(projectPath, file));
			}
			else {
				mesh_files.push(file);
			}
		}
		return mesh_files;
	}

	static fonts_to_files = (projectPath: string, fonts: SlotFontRaw[]): string[] => {
		let font_files: string[] = [];
		for (let i = 1; i <fonts.length; ++i) {
			let f: SlotFontRaw = fonts[i];
			///if krom_ios
			let sameDrive: bool = false;
			///else
			let sameDrive: bool = projectPath.charAt(0) == f.file.charAt(0);
			///end
			// Convert font path from absolute to relative
			if (sameDrive) {
				font_files.push(Path.to_relative(projectPath, f.file));
			}
			else {
				font_files.push(f.file);
			}
		}
		return font_files;
	}
	///end

	static get_packed_assets = (projectPath: string, texture_files: string[]): packed_asset_t[] => {
		let packed_assets: packed_asset_t[] = null;
		if (Project.raw.packed_assets != null) {
			for (let pa of Project.raw.packed_assets) {
				///if krom_ios
				let sameDrive: bool = false;
				///else
				let sameDrive: bool = projectPath.charAt(0) == pa.name.charAt(0);
				///end
				// Convert path from absolute to relative
				pa.name = sameDrive ? Path.to_relative(projectPath, pa.name) : pa.name;
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

	static pack_assets = (raw: project_format_t, assets: asset_t[]) => {
		if (raw.packed_assets == null) {
			raw.packed_assets = [];
		}
		let tempImages: image_t[] = [];
		for (let i: i32 = 0; i < assets.length; ++i) {
			if (!Project.packed_asset_exists(raw.packed_assets, assets[i].file)) {
				let image: image_t = Project.get_image(assets[i]);
				let temp: image_t = image_create_render_target(image.width, image.height);
				g2_begin(temp);
				g2_draw_image(image, 0, 0);
				g2_end();
				tempImages.push(temp);
				raw.packed_assets.push({
					name: assets[i].file,
					bytes: assets[i].file.endsWith(".jpg") ?
						krom_encode_jpg(image_get_pixels(temp), temp.width, temp.height, 0, 80) :
						krom_encode_png(image_get_pixels(temp), temp.width, temp.height, 0)
				});
			}
		}
		Base.notify_on_next_frame(() => {
			for (let image of tempImages) image_unload(image);
		});
	}

	static run_swatches = (path: string) => {
		if (!path.endsWith(".arm")) path += ".arm";
		let raw: any = {
			version: manifest_version,
			swatches: Project.raw.swatches
		};
		let buffer: buffer_t = armpack_encode(raw);
		krom_file_save_bytes(path, buffer, buffer.byteLength + 1);
	}

	static vec3f32 = (v: vec4_t): Float32Array => {
		let res: Float32Array = new Float32Array(3);
		res[0] = v.x;
		res[1] = v.y;
		res[2] = v.z;
		return res;
	}
}
