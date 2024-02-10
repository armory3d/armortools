
class ExportArm {

	static runMesh = (path: string, paintObjects: mesh_object_t[]) => {
		let mesh_datas: mesh_data_t[] = [];
		for (let p of paintObjects) mesh_datas.push(p.data);
		let raw: scene_t = { mesh_datas: mesh_datas };
		let b = armpack_encode(raw);
		if (!path.endsWith(".arm")) path += ".arm";
		krom_file_save_bytes(path, b, b.byteLength + 1);
	}

	static runProject = () => {
		///if (is_paint || is_sculpt)
		let mnodes: zui_node_canvas_t[] = [];
		for (let m of Project.materials) {
			let c: zui_node_canvas_t = JSON.parse(JSON.stringify(m.canvas));
			for (let n of c.nodes) ExportArm.exportNode(n);
			mnodes.push(c);
		}

		let bnodes: zui_node_canvas_t[] = [];
		for (let b of Project.brushes) bnodes.push(b.canvas);
		///end

		///if is_lab
		let c: zui_node_canvas_t = JSON.parse(JSON.stringify(Project.canvas));
		for (let n of c.nodes) ExportArm.exportNode(n);
		///end

		let mgroups: zui_node_canvas_t[] = null;
		if (Project.materialGroups.length > 0) {
			mgroups = [];
			for (let g of Project.materialGroups) {
				let c: zui_node_canvas_t = JSON.parse(JSON.stringify(g.canvas));
				for (let n of c.nodes) ExportArm.exportNode(n);
				mgroups.push(c);
			}
		}

		///if (is_paint || is_sculpt)
		let md: mesh_data_t[] = [];
		for (let p of Project.paintObjects) md.push(p.data);
		///end

		///if is_lab
		let md = Project.paintObjects[0].data;
		///end

		let texture_files = ExportArm.assetsToFiles(Project.filepath, Project.assets);

		///if (is_paint || is_sculpt)
		let font_files = ExportArm.fontsToFiles(Project.filepath, Project.fonts);
		let mesh_files = ExportArm.meshesToFiles(Project.filepath);

		let bitsPos = Base.bitsHandle.position;
		let bpp = bitsPos == TextureBits.Bits8 ? 8 : bitsPos == TextureBits.Bits16 ? 16 : 32;

		let ld: TLayerData[] = [];
		for (let l of Project.layers) {
			ld.push({
				name: l.name,
				res: l.texpaint != null ? l.texpaint.width : Project.layers[0].texpaint.width,
				bpp: bpp,
				texpaint: l.texpaint != null ? lz4_encode(image_get_pixels(l.texpaint)) : null,
				uv_scale: l.scale,
				uv_rot: l.angle,
				uv_type: l.uvType,
				decal_mat: l.uvType == UVType.UVProject ? mat4_to_f32_array(l.decalMat) : null,
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

		let packed_assets = (Project.raw.packed_assets == null || Project.raw.packed_assets.length == 0) ? null : Project.raw.packed_assets;
		///if krom_ios
		let sameDrive = false;
		///else
		let sameDrive = Project.raw.envmap != null ? Project.filepath.charAt(0) == Project.raw.envmap.charAt(0) : true;
		///end

		Project.raw = {
			version: manifest_version,
			material_groups: mgroups,
			assets: texture_files,
			packed_assets: packed_assets,
			swatches: Project.raw.swatches,
			envmap: Project.raw.envmap != null ? (sameDrive ? Path.toRelative(Project.filepath, Project.raw.envmap) : Project.raw.envmap) : null,
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
			atlas_objects: Project.atlasObjects,
			atlas_names: Project.atlasNames,
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
		let tex = render_path_render_targets.get(Context.raw.renderMode == RenderMode.RenderForward ? "buf" : "tex").image;
		let mesh_icon = image_create_render_target(256, 256);
		let r = app_w() / app_h();
		g2_begin(mesh_icon, false);
		///if krom_opengl
		g2_draw_scaled_image(tex, -(256 * r - 256) / 2, 256, 256 * r, -256);
		///else
		g2_draw_scaled_image(tex, -(256 * r - 256) / 2, 0, 256 * r, 256);
		///end
		g2_end();
		///if krom_metal
		// Flush command list
		g2_begin(mesh_icon, false);
		g2_end();
		///end
		let mesh_icon_pixels = image_get_pixels(mesh_icon);
		let u8a = new Uint8Array(mesh_icon_pixels);
		for (let i = 0; i < 256 * 256 * 4; ++i) {
			u8a[i] = Math.floor(Math.pow(u8a[i] / 255, 1.0 / 2.2) * 255);
		}
		///if (krom_metal || krom_vulkan)
		ExportArm.bgraSwap(mesh_icon_pixels);
		///end
		Base.notifyOnNextFrame(() => {
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
		let isPacked = Project.filepath.endsWith("_packed_.arm");
		if (isPacked) { // Pack textures
			ExportArm.packAssets(Project.raw, Project.assets);
		}
		///end

		let buffer = armpack_encode(Project.raw);
		krom_file_save_bytes(Project.filepath, buffer, buffer.byteLength + 1);

		// Save to recent
		///if krom_ios
		let recent_path = Project.filepath.substr(Project.filepath.lastIndexOf("/") + 1);
		///else
		let recent_path = Project.filepath;
		///end
		let recent = Config.raw.recent_projects;
		array_remove(recent, recent_path);
		recent.unshift(recent_path);
		Config.save();

		Console.info(tr("Project saved"));
	}

	static textureNodeName = (): string => {
		///if (is_paint || is_sculpt)
		return "TEX_IMAGE";
		///else
		return "ImageTextureNode";
		///end
	}

	static exportNode = (n: zui_node_t, assets: TAsset[] = null) => {
		if (n.type == ExportArm.textureNodeName()) {
			let index = n.buttons[0].default_value;
			n.buttons[0].data = Base.enumTexts(n.type)[index];

			if (assets != null) {
				let asset = Project.assets[index];
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
	static runMaterial = (path: string) => {
		if (!path.endsWith(".arm")) path += ".arm";
		let mnodes: zui_node_canvas_t[] = [];
		let mgroups: zui_node_canvas_t[] = null;
		let m = Context.raw.material;
		let c: zui_node_canvas_t = JSON.parse(JSON.stringify(m.canvas));
		let assets: TAsset[] = [];
		if (UINodes.hasGroup(c)) {
			mgroups = [];
			UINodes.traverseGroup(mgroups, c);
			for (let gc of mgroups) for (let n of gc.nodes) ExportArm.exportNode(n, assets);
		}
		for (let n of c.nodes) ExportArm.exportNode(n, assets);
		mnodes.push(c);

		let texture_files = ExportArm.assetsToFiles(path, assets);
		let isCloud = path.endsWith("_cloud_.arm");
		if (isCloud) path = path.replaceAll("_cloud_", "");
		let packed_assets: TPackedAsset[] = null;
		if (!Context.raw.packAssetsOnExport) {
			packed_assets = ExportArm.getPackedAssets(path, texture_files);
		}

		let raw: TProjectFormat = {
			version: manifest_version,
			material_nodes: mnodes,
			material_groups: mgroups,
			material_icons: isCloud ? null :
				///if (krom_metal || krom_vulkan)
				[lz4_encode(ExportArm.bgraSwap(image_get_pixels(m.image)))],
				///else
				[lz4_encode(image_get_pixels(m.image))],
				///end
			assets: texture_files,
			packed_assets: packed_assets
		};

		if (Context.raw.writeIconOnExport) { // Separate icon files
			krom_write_png(path.substr(0, path.length - 4) + "_icon.png", image_get_pixels(m.image), m.image.width, m.image.height, 0);
			if (isCloud) {
				krom_write_jpg(path.substr(0, path.length - 4) + "_icon.jpg", image_get_pixels(m.image), m.image.width, m.image.height, 0, 50);
			}
		}

		if (Context.raw.packAssetsOnExport) { // Pack textures
			ExportArm.packAssets(raw, assets);
		}

		let buffer = armpack_encode(raw);
		krom_file_save_bytes(path, buffer, buffer.byteLength + 1);
	}
	///end

	///if (krom_metal || krom_vulkan)
	static bgraSwap = (buffer: ArrayBuffer) => {
		let view = new DataView(buffer);
		for (let i = 0; i < Math.floor(buffer.byteLength / 4); ++i) {
			let r = view.getUint8(i * 4);
			view.setUint8(i * 4, view.getUint8(i * 4 + 2));
			view.setUint8(i * 4 + 2, r);
		}
		return buffer;
	}
	///end

	///if (is_paint || is_sculpt)
	static runBrush = (path: string) => {
		if (!path.endsWith(".arm")) path += ".arm";
		let bnodes: zui_node_canvas_t[] = [];
		let b = Context.raw.brush;
		let c: zui_node_canvas_t = JSON.parse(JSON.stringify(b.canvas));
		let assets: TAsset[] = [];
		for (let n of c.nodes) ExportArm.exportNode(n, assets);
		bnodes.push(c);

		let texture_files = ExportArm.assetsToFiles(path, assets);
		let isCloud = path.endsWith("_cloud_.arm");
		if (isCloud) path = path.replaceAll("_cloud_", "");
		let packed_assets: TPackedAsset[] = null;
		if (!Context.raw.packAssetsOnExport) {
			packed_assets = ExportArm.getPackedAssets(path, texture_files);
		}

		let raw: TProjectFormat = {
			version: manifest_version,
			brush_nodes: bnodes,
			brush_icons: isCloud ? null :
			///if (krom_metal || krom_vulkan)
			[lz4_encode(ExportArm.bgraSwap(image_get_pixels(b.image)))],
			///else
			[lz4_encode(image_get_pixels(b.image))],
			///end
			assets: texture_files,
			packed_assets: packed_assets
		};

		if (Context.raw.writeIconOnExport) { // Separate icon file
			krom_write_png(path.substr(0, path.length - 4) + "_icon.png", image_get_pixels(b.image), b.image.width, b.image.height, 0);
		}

		if (Context.raw.packAssetsOnExport) { // Pack textures
			ExportArm.packAssets(raw, assets);
		}

		let buffer = armpack_encode(raw);
		krom_file_save_bytes(path, buffer, buffer.byteLength + 1);
	}
	///end

	static assetsToFiles = (projectPath: string, assets: TAsset[]): string[] => {
		let texture_files: string[] = [];
		for (let a of assets) {
			///if krom_ios
			let sameDrive = false;
			///else
			let sameDrive = projectPath.charAt(0) == a.file.charAt(0);
			///end
			// Convert image path from absolute to relative
			if (sameDrive) {
				texture_files.push(Path.toRelative(projectPath, a.file));
			}
			else {
				texture_files.push(a.file);
			}
		}
		return texture_files;
	}

	///if (is_paint || is_sculpt)
	static meshesToFiles = (projectPath: string): string[] => {
		let mesh_files: string[] = [];
		for (let file of Project.meshAssets) {
			///if krom_ios
			let sameDrive = false;
			///else
			let sameDrive = projectPath.charAt(0) == file.charAt(0);
			///end
			// Convert mesh path from absolute to relative
			if (sameDrive) {
				mesh_files.push(Path.toRelative(projectPath, file));
			}
			else {
				mesh_files.push(file);
			}
		}
		return mesh_files;
	}

	static fontsToFiles = (projectPath: string, fonts: SlotFontRaw[]): string[] => {
		let font_files: string[] = [];
		for (let i = 1; i <fonts.length; ++i) {
			let f = fonts[i];
			///if krom_ios
			let sameDrive = false;
			///else
			let sameDrive = projectPath.charAt(0) == f.file.charAt(0);
			///end
			// Convert font path from absolute to relative
			if (sameDrive) {
				font_files.push(Path.toRelative(projectPath, f.file));
			}
			else {
				font_files.push(f.file);
			}
		}
		return font_files;
	}
	///end

	static getPackedAssets = (projectPath: string, texture_files: string[]): TPackedAsset[] => {
		let packed_assets: TPackedAsset[] = null;
		if (Project.raw.packed_assets != null) {
			for (let pa of Project.raw.packed_assets) {
				///if krom_ios
				let sameDrive = false;
				///else
				let sameDrive = projectPath.charAt(0) == pa.name.charAt(0);
				///end
				// Convert path from absolute to relative
				pa.name = sameDrive ? Path.toRelative(projectPath, pa.name) : pa.name;
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

	static packAssets = (raw: TProjectFormat, assets: TAsset[]) => {
		if (raw.packed_assets == null) {
			raw.packed_assets = [];
		}
		let tempImages: image_t[] = [];
		for (let i = 0; i < assets.length; ++i) {
			if (!Project.packedAssetExists(raw.packed_assets, assets[i].file)) {
				let image = Project.getImage(assets[i]);
				let temp = image_create_render_target(image.width, image.height);
				g2_begin(temp, false);
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
		Base.notifyOnNextFrame(() => {
			for (let image of tempImages) image_unload(image);
		});
	}

	static runSwatches = (path: string) => {
		if (!path.endsWith(".arm")) path += ".arm";
		let raw = {
			version: manifest_version,
			swatches: Project.raw.swatches
		};
		let buffer = armpack_encode(raw);
		krom_file_save_bytes(path, buffer, buffer.byteLength + 1);
	}

	static vec3f32 = (v: vec4_t): Float32Array => {
		let res = new Float32Array(3);
		res[0] = v.x;
		res[1] = v.y;
		res[2] = v.z;
		return res;
	}
}
