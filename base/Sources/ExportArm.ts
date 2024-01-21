
class ExportArm {

	static runMesh = (path: string, paintObjects: MeshObject[]) => {
		let mesh_datas: TMeshData[] = [];
		for (let p of paintObjects) mesh_datas.push(p.data.raw);
		let raw: TSceneFormat = { mesh_datas: mesh_datas };
		let b = ArmPack.encode(raw);
		if (!path.endsWith(".arm")) path += ".arm";
		Krom.fileSaveBytes(path, b, b.byteLength + 1);
	}

	static runProject = () => {
		///if (is_paint || is_sculpt)
		let mnodes: TNodeCanvas[] = [];
		for (let m of Project.materials) {
			let c: TNodeCanvas = JSON.parse(JSON.stringify(m.canvas));
			for (let n of c.nodes) ExportArm.exportNode(n);
			mnodes.push(c);
		}

		let bnodes: TNodeCanvas[] = [];
		for (let b of Project.brushes) bnodes.push(b.canvas);
		///end

		///if is_lab
		let c: TNodeCanvas = JSON.parse(JSON.stringify(Project.canvas));
		for (let n of c.nodes) ExportArm.exportNode(n);
		///end

		let mgroups: TNodeCanvas[] = null;
		if (Project.materialGroups.length > 0) {
			mgroups = [];
			for (let g of Project.materialGroups) {
				let c: TNodeCanvas = JSON.parse(JSON.stringify(g.canvas));
				for (let n of c.nodes) ExportArm.exportNode(n);
				mgroups.push(c);
			}
		}

		///if (is_paint || is_sculpt)
		let md: TMeshData[] = [];
		for (let p of Project.paintObjects) md.push(p.data.raw);
		///end

		///if is_lab
		let md = Project.paintObjects[0].data.raw;
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
				texpaint: l.texpaint != null ? Lz4.encode(l.texpaint.getPixels()) : null,
				uv_scale: l.scale,
				uv_rot: l.angle,
				uv_type: l.uvType,
				decal_mat: l.uvType == UVType.UVProject ? l.decalMat.toFloat32Array() : null,
				opacity_mask: l.maskOpacity,
				fill_layer: l.fill_layer != null ? Project.materials.indexOf(l.fill_layer) : -1,
				object_mask: l.objectMask,
				blending: l.blending,
				parent: l.parent != null ? Project.layers.indexOf(l.parent) : -1,
				visible: l.visible,
				///if is_paint
				texpaint_nor: l.texpaint_nor != null ? Lz4.encode(l.texpaint_nor.getPixels()) : null,
				texpaint_pack: l.texpaint_pack != null ? Lz4.encode(l.texpaint_pack.getPixels()) : null,
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
			version: Manifest.version,
			material_groups: mgroups,
			assets: texture_files,
			packed_assets: packed_assets,
			swatches: Project.raw.swatches,
			envmap: Project.raw.envmap != null ? (sameDrive ? Path.toRelative(Project.filepath, Project.raw.envmap) : Project.raw.envmap) : null,
			envmap_strength: Scene.active.world.probe.raw.strength,
			camera_world: Scene.active.camera.transform.local.toFloat32Array(),
			camera_origin: ExportArm.vec3f32(Camera.origins[0]),
			camera_fov: Scene.active.camera.data.raw.fov,

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
		let tex = RenderPath.active.renderTargets.get(Context.raw.renderMode == RenderMode.RenderForward ? "buf" : "tex").image;
		let mesh_icon = Image.createRenderTarget(256, 256);
		let r = App.w() / App.h();
		mesh_icon.g2.begin(false);
		///if krom_opengl
		mesh_icon.g2.drawScaledImage(tex, -(256 * r - 256) / 2, 256, 256 * r, -256);
		///else
		mesh_icon.g2.drawScaledImage(tex, -(256 * r - 256) / 2, 0, 256 * r, 256);
		///end
		mesh_icon.g2.end();
		///if krom_metal
		// Flush command list
		mesh_icon.g2.begin(false);
		mesh_icon.g2.end();
		///end
		let mesh_icon_pixels = mesh_icon.getPixels();
		let u8a = new Uint8Array(mesh_icon_pixels);
		for (let i = 0; i < 256 * 256 * 4; ++i) {
			u8a[i] = Math.floor(Math.pow(u8a[i] / 255, 1.0 / 2.2) * 255);
		}
		///if (krom_metal || krom_vulkan)
		ExportArm.bgraSwap(mesh_icon_pixels);
		///end
		Base.notifyOnNextFrame(() => {
			mesh_icon.unload();
		});
		// Project.raw.mesh_icons =
		// 	///if (krom_metal || krom_vulkan)
		// 	[Lz4.encode(bgraSwap(mesh_icon_pixels)];
		// 	///else
		// 	[Lz4.encode(mesh_icon_pixels)];
		// 	///end
		Krom.writePng(Project.filepath.substr(0, Project.filepath.length - 4) + "_icon.png", mesh_icon_pixels, 256, 256, 0);
		///end

		///if (is_paint || is_sculpt)
		let isPacked = Project.filepath.endsWith("_packed_.arm");
		if (isPacked) { // Pack textures
			ExportArm.packAssets(Project.raw, Project.assets);
		}
		///end

		let buffer = ArmPack.encode(Project.raw);
		Krom.fileSaveBytes(Project.filepath, buffer, buffer.byteLength + 1);

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

	static exportNode = (n: TNode, assets: TAsset[] = null) => {
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
		let mnodes: TNodeCanvas[] = [];
		let mgroups: TNodeCanvas[] = null;
		let m = Context.raw.material;
		let c: TNodeCanvas = JSON.parse(JSON.stringify(m.canvas));
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
		if (isCloud) path = path.replace("_cloud_", "");
		let packed_assets: TPackedAsset[] = null;
		if (!Context.raw.packAssetsOnExport) {
			packed_assets = ExportArm.getPackedAssets(path, texture_files);
		}

		let raw: TProjectFormat = {
			version: Manifest.version,
			material_nodes: mnodes,
			material_groups: mgroups,
			material_icons: isCloud ? null :
				///if (krom_metal || krom_vulkan)
				[Lz4.encode(ExportArm.bgraSwap(m.image.getPixels()))],
				///else
				[Lz4.encode(m.image.getPixels())],
				///end
			assets: texture_files,
			packed_assets: packed_assets
		};

		if (Context.raw.writeIconOnExport) { // Separate icon files
			Krom.writePng(path.substr(0, path.length - 4) + "_icon.png", m.image.getPixels(), m.image.width, m.image.height, 0);
			if (isCloud) {
				Krom.writeJpg(path.substr(0, path.length - 4) + "_icon.jpg", m.image.getPixels(), m.image.width, m.image.height, 0, 50);
			}
		}

		if (Context.raw.packAssetsOnExport) { // Pack textures
			ExportArm.packAssets(raw, assets);
		}

		let buffer = ArmPack.encode(raw);
		Krom.fileSaveBytes(path, buffer, buffer.byteLength + 1);
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
		let bnodes: TNodeCanvas[] = [];
		let b = Context.raw.brush;
		let c: TNodeCanvas = JSON.parse(JSON.stringify(b.canvas));
		let assets: TAsset[] = [];
		for (let n of c.nodes) ExportArm.exportNode(n, assets);
		bnodes.push(c);

		let texture_files = ExportArm.assetsToFiles(path, assets);
		let isCloud = path.endsWith("_cloud_.arm");
		if (isCloud) path = path.replace("_cloud_", "");
		let packed_assets: TPackedAsset[] = null;
		if (!Context.raw.packAssetsOnExport) {
			packed_assets = ExportArm.getPackedAssets(path, texture_files);
		}

		let raw: TProjectFormat = {
			version: Manifest.version,
			brush_nodes: bnodes,
			brush_icons: isCloud ? null :
			///if (krom_metal || krom_vulkan)
			[Lz4.encode(ExportArm.bgraSwap(b.image.getPixels()))],
			///else
			[Lz4.encode(b.image.getPixels())],
			///end
			assets: texture_files,
			packed_assets: packed_assets
		};

		if (Context.raw.writeIconOnExport) { // Separate icon file
			Krom.writePng(path.substr(0, path.length - 4) + "_icon.png", b.image.getPixels(), b.image.width, b.image.height, 0);
		}

		if (Context.raw.packAssetsOnExport) { // Pack textures
			ExportArm.packAssets(raw, assets);
		}

		let buffer = ArmPack.encode(raw);
		Krom.fileSaveBytes(path, buffer, buffer.byteLength + 1);
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

	static fontsToFiles = (projectPath: string, fonts: SlotFont[]): string[] => {
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
		let tempImages: Image[] = [];
		for (let i = 0; i < assets.length; ++i) {
			if (!Project.packedAssetExists(raw.packed_assets, assets[i].file)) {
				let image = Project.getImage(assets[i]);
				let temp = Image.createRenderTarget(image.width, image.height);
				temp.g2.begin(false);
				temp.g2.drawImage(image, 0, 0);
				temp.g2.end();
				tempImages.push(temp);
				raw.packed_assets.push({
					name: assets[i].file,
					bytes: assets[i].file.endsWith(".jpg") ?
						Krom.encodeJpg(temp.getPixels(), temp.width, temp.height, 0, 80) :
						Krom.encodePng(temp.getPixels(), temp.width, temp.height, 0)
				});
			}
		}
		Base.notifyOnNextFrame(() => {
			for (let image of tempImages) image.unload();
		});
	}

	static runSwatches = (path: string) => {
		if (!path.endsWith(".arm")) path += ".arm";
		let raw = {
			version: Manifest.version,
			swatches: Project.raw.swatches
		};
		let buffer = ArmPack.encode(raw);
		Krom.fileSaveBytes(path, buffer, buffer.byteLength + 1);
	}

	static vec3f32 = (v: Vec4): Float32Array => {
		let res = new Float32Array(3);
		res[0] = v.x;
		res[1] = v.y;
		res[2] = v.z;
		return res;
	}
}
