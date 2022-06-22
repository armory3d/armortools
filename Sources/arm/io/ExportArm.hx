package arm.io;

import haxe.Json;
import haxe.io.Bytes;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.object.MeshObject;
import iron.system.ArmPack;
import iron.system.Lz4;
import arm.data.FontSlot;
import arm.ui.UISidebar;
import arm.ui.UINodes;
import arm.sys.Path;
import arm.ProjectFormat;
import arm.Enums;

class ExportArm {

	public static function runMesh(path: String, paintObjects: Array<MeshObject>) {
		var mesh_datas: Array<TMeshData> = [];
		for (p in paintObjects) mesh_datas.push(p.data.raw);
		var raw: TSceneFormat = { mesh_datas: mesh_datas };
		var b = ArmPack.encode(raw);
		if (!path.endsWith(".arm")) path += ".arm";
		Krom.fileSaveBytes(path, b.getData(), b.length + 1);
	}

	public static function runProject() {
		var mnodes: Array<TNodeCanvas> = [];
		for (m in Project.materials) {
			var c: TNodeCanvas = Json.parse(Json.stringify(m.canvas));
			for (n in c.nodes) exportNode(n);
			mnodes.push(c);
		}

		var bnodes: Array<TNodeCanvas> = [];
		for (b in Project.brushes) bnodes.push(b.canvas);

		var mgroups: Array<TNodeCanvas> = null;
		if (Project.materialGroups.length > 0) {
			mgroups = [];
			for (g in Project.materialGroups) {
				var c: TNodeCanvas = Json.parse(Json.stringify(g.canvas));
				for (n in c.nodes) exportNode(n);
				mgroups.push(c);
			}
		}

		var md: Array<TMeshData> = [];
		for (p in Project.paintObjects) md.push(p.data.raw);

		var texture_files = assetsToFiles(Project.filepath, Project.assets);
		var font_files = fontsToFiles(Project.filepath, Project.fonts);
		var mesh_files = meshesToFiles(Project.filepath);

		var bitsPos = App.bitsHandle.position;
		var bpp = bitsPos == Bits8 ? 8 : bitsPos == Bits16 ? 16 : 32;

		var ld: Array<TLayerData> = [];
		for (l in Project.layers) {
			ld.push({
				name: l.name,
				res: l.texpaint != null ? l.texpaint.width : Project.layers[0].texpaint.width,
				bpp: bpp,
				texpaint: l.texpaint != null ? Lz4.encode(l.texpaint.getPixels()) : null,
				texpaint_nor: l.texpaint_nor != null ? Lz4.encode(l.texpaint_nor.getPixels()) : null,
				texpaint_pack: l.texpaint_pack != null ? Lz4.encode(l.texpaint_pack.getPixels()) : null,
				uv_scale: l.scale,
				uv_rot: l.angle,
				uv_type: l.uvType,
				decal_mat: l.uvType == UVProject ? l.decalMat.toFloat32Array() : null,
				opacity_mask: l.maskOpacity,
				fill_layer: l.fill_layer != null ? Project.materials.indexOf(l.fill_layer) : -1,
				object_mask: l.objectMask,
				blending: l.blending,
				parent: l.parent != null ? Project.layers.indexOf(l.parent) : -1,
				visible: l.visible,
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
			});
		}

		var packed_assets = Project.raw.packed_assets == null || Project.raw.packed_assets.length == 0 ? null : Project.raw.packed_assets;
		#if krom_ios
		var sameDrive = false;
		#else
		var sameDrive = Project.raw.envmap != null ? Project.filepath.charAt(0) == Project.raw.envmap.charAt(0) : true;
		#end

		Project.raw = {
			version: Main.version,
			material_nodes: mnodes,
			material_groups: mgroups,
			brush_nodes: bnodes,
			mesh_datas: md,
			layer_datas: ld,
			assets: texture_files,
			font_assets: font_files,
			mesh_assets: mesh_files,
			atlas_objects: Project.atlasObjects,
			atlas_names: Project.atlasNames,
			swatches: Project.raw.swatches,
			packed_assets: packed_assets,
			envmap: Project.raw.envmap != null ? (sameDrive ? Path.toRelative(Project.filepath, Project.raw.envmap) : Project.raw.envmap) : null,
			envmap_strength: iron.Scene.active.world.probe.raw.strength,
			camera_world: iron.Scene.active.camera.transform.local.toFloat32Array(),
			camera_origin: vec3f32(arm.Camera.inst.origins[0]),
			camera_fov: iron.Scene.active.camera.data.raw.fov,
			#if (kha_metal || kha_vulkan)
			is_bgra: true
			#else
			is_bgra: false
			#end
		};

		#if (krom_android || krom_ios)
		var tex = iron.RenderPath.active.renderTargets.get(Context.renderMode == RenderForward ? "buf" : "tex").image;
		var mesh_icon = kha.Image.createRenderTarget(256, 256);
		var r = App.w() / App.h();
		mesh_icon.g2.begin(false);
		#if kha_opengl
		mesh_icon.g2.drawScaledImage(tex, -(256 * r - 256) / 2, 256, 256 * r, -256);
		#else
		mesh_icon.g2.drawScaledImage(tex, -(256 * r - 256) / 2, 0, 256 * r, 256);
		#end
		mesh_icon.g2.end();
		#if kha_metal
		// Flush command list
		mesh_icon.g2.begin(false);
		mesh_icon.g2.end();
		#end
		var mesh_icon_pixels = mesh_icon.getPixels();
		for (i in 0...256 * 256 * 4) {
			mesh_icon_pixels.set(i, Std.int(Math.pow(mesh_icon_pixels.get(i) / 255, 1.0 / 2.2) * 255));
		}
		#if (kha_metal || kha_vulkan)
		bgraSwap(mesh_icon_pixels);
		#end
		App.notifyOnNextFrame(function() {
			mesh_icon.unload();
		});
		// Project.raw.mesh_icons =
		// 	#if (kha_metal || kha_vulkan)
		// 	[Lz4.encode(bgraSwap(mesh_icon_pixels)];
		// 	#else
		// 	[Lz4.encode(mesh_icon_pixels)];
		// 	#end
		Krom.writePng(Project.filepath.substr(0, Project.filepath.length - 4) + "_icon.png", mesh_icon_pixels.getData(), 256, 256, 0);
		#end

		var isPacked = Project.filepath.endsWith("_packed_.arm");
		if (isPacked) { // Pack textures
			packAssets(Project.raw, Project.assets);
		}

		var bytes = ArmPack.encode(Project.raw);
		Krom.fileSaveBytes(Project.filepath, bytes.getData(), bytes.length + 1);

		// Save to recent
		#if krom_ios
		var recent_path = Project.filepath.substr(Project.filepath.lastIndexOf("/") + 1);
		#else
		var recent_path = Project.filepath;
		#end
		var recent = Config.raw.recent_projects;
		recent.remove(recent_path);
		recent.unshift(recent_path);
		Config.save();

		Console.info(tr("Project saved"));
	}

	static function exportNode(n: TNode, assets: Array<TAsset> = null) {
		if (n.type == "TEX_IMAGE") {
			var index = n.buttons[0].default_value;
			n.buttons[0].data = App.enumTexts(n.type)[index];

			if (assets != null) {
				var asset = Project.assets[index];
				if (assets.indexOf(asset) == -1) {
					assets.push(asset);
				}
			}
		}
		// Pack colors
		if (n.color > 0) n.color -= untyped 4294967296;
		for (inp in n.inputs) if (inp.color > 0) inp.color -= untyped 4294967296;
		for (out in n.outputs) if (out.color > 0) out.color -= untyped 4294967296;
	}

	public static function runMaterial(path: String) {
		if (!path.endsWith(".arm")) path += ".arm";
		var mnodes: Array<TNodeCanvas> = [];
		var mgroups: Array<TNodeCanvas> = null;
		var m = Context.material;
		var c: TNodeCanvas = Json.parse(Json.stringify(m.canvas));
		var assets: Array<TAsset> = [];
		if (UINodes.hasGroup(c)) {
			mgroups = [];
			UINodes.traverseGroup(mgroups, c);
			for (gc in mgroups) for (n in gc.nodes) exportNode(n, assets);
		}
		for (n in c.nodes) exportNode(n, assets);
		mnodes.push(c);

		var texture_files = assetsToFiles(path, assets);
		var isCloud = path.endsWith("_cloud_.arm");
		if (isCloud) path = path.replace("_cloud_", "");
		var packed_assets: Array<TPackedAsset> = null;
		if (!Context.packAssetsOnExport) {
			packed_assets = getPackedAssets(path, texture_files);
		}

		var raw: TProjectFormat = {
			version: Main.version,
			material_nodes: mnodes,
			material_groups: mgroups,
			material_icons: isCloud ? null :
				#if (kha_metal || kha_vulkan)
				[Lz4.encode(bgraSwap(m.image.getPixels()))],
				#else
				[Lz4.encode(m.image.getPixels())],
				#end
			assets: texture_files,
			packed_assets: packed_assets
		};

		if (Context.writeIconOnExport) { // Separate icon files
			Krom.writePng(path.substr(0, path.length - 4) + "_icon.png", m.image.getPixels().getData(), m.image.width, m.image.height, 0);
			if (isCloud) {
				Krom.writeJpg(path.substr(0, path.length - 4) + "_icon.jpg", m.image.getPixels().getData(), m.image.width, m.image.height, 0, 50);
			}
		}

		if (Context.packAssetsOnExport) { // Pack textures
			packAssets(raw, assets);
		}

		var bytes = ArmPack.encode(raw);
		Krom.fileSaveBytes(path, bytes.getData(), bytes.length + 1);
	}

	#if (kha_metal || kha_vulkan)
	static function bgraSwap(bytes: haxe.io.Bytes) {
		for (i in 0...Std.int(bytes.length / 4)) {
			var r = bytes.get(i * 4);
			bytes.set(i * 4, bytes.get(i * 4 + 2));
			bytes.set(i * 4 + 2, r);
		}
		return bytes;
	}
	#end

	public static function runBrush(path: String) {
		if (!path.endsWith(".arm")) path += ".arm";
		var bnodes: Array<TNodeCanvas> = [];
		var b = Context.brush;
		var c: TNodeCanvas = Json.parse(Json.stringify(b.canvas));
		var assets: Array<TAsset> = [];
		for (n in c.nodes) exportNode(n, assets);
		bnodes.push(c);

		var texture_files = assetsToFiles(path, assets);
		var isCloud = path.endsWith("_cloud_.arm");
		if (isCloud) path = path.replace("_cloud_", "");
		var packed_assets: Array<TPackedAsset> = null;
		if (!Context.packAssetsOnExport) {
			packed_assets = getPackedAssets(path, texture_files);
		}

		var raw = {
			version: Main.version,
			brush_nodes: bnodes,
			brush_icons: isCloud ? null :
			#if (kha_metal || kha_vulkan)
			[Lz4.encode(bgraSwap(b.image.getPixels()))],
			#else
			[Lz4.encode(b.image.getPixels())],
			#end
			assets: texture_files,
			packed_assets: packed_assets
		};

		if (Context.writeIconOnExport) { // Separate icon file
			Krom.writePng(path.substr(0, path.length - 4) + "_icon.png", b.image.getPixels().getData(), b.image.width, b.image.height, 0);
		}

		if (Context.packAssetsOnExport) { // Pack textures
			packAssets(raw, assets);
		}

		var bytes = ArmPack.encode(raw);
		Krom.fileSaveBytes(path, bytes.getData(), bytes.length + 1);
	}

	static function assetsToFiles(projectPath: String, assets: Array<TAsset>): Array<String> {
		var texture_files: Array<String> = [];
		for (a in assets) {
			#if krom_ios
			var sameDrive = false;
			#else
			var sameDrive = projectPath.charAt(0) == a.file.charAt(0);
			#end
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

	static function meshesToFiles(projectPath: String): Array<String> {
		var mesh_files: Array<String> = [];
		for (file in Project.meshAssets) {
			#if krom_ios
			var sameDrive = false;
			#else
			var sameDrive = projectPath.charAt(0) == file.charAt(0);
			#end
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

	static function fontsToFiles(projectPath: String, fonts: Array<FontSlot>): Array<String> {
		var font_files: Array<String> = [];
		for (i in 1...fonts.length) {
			var f = fonts[i];
			#if krom_ios
			var sameDrive = false;
			#else
			var sameDrive = projectPath.charAt(0) == f.file.charAt(0);
			#end
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

	static function getPackedAssets(projectPath: String, texture_files: Array<String>): Array<TPackedAsset> {
		var packed_assets: Array<TPackedAsset> = null;
		if (Project.raw.packed_assets != null) {
			for (pa in Project.raw.packed_assets) {
				#if krom_ios
				var sameDrive = false;
				#else
				var sameDrive = projectPath.charAt(0) == pa.name.charAt(0);
				#end
				// Convert path from absolute to relative
				pa.name = sameDrive ? Path.toRelative(projectPath, pa.name) : pa.name;
				for (tf in texture_files) {
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

	static function packAssets(raw: TProjectFormat, assets: Array<TAsset>) {
		if (raw.packed_assets == null) {
			raw.packed_assets = [];
		}
		var tempImages: Array<kha.Image> = [];
		for (i in 0...assets.length) {
			if (!Project.packedAssetExists(raw.packed_assets, assets[i].file)) {
				var image = Project.getImage(assets[i]);
				var temp = kha.Image.createRenderTarget(image.width, image.height);
				temp.g2.begin(false);
				temp.g2.drawImage(image, 0, 0);
				temp.g2.end();
				tempImages.push(temp);
				raw.packed_assets.push({
					name: assets[i].file,
					bytes: Bytes.ofData(assets[i].file.endsWith(".jpg") ?
						Krom.encodeJpg(temp.getPixels().getData(), temp.width, temp.height, 0, 80) :
						Krom.encodePng(temp.getPixels().getData(), temp.width, temp.height, 0)
					)
				});
			}
		}
		App.notifyOnNextFrame(function() {
			for (image in tempImages) image.unload();
		});
	}

	public static function runSwatches(path: String) {
		if (!path.endsWith(".arm")) path += ".arm";
		var raw = {
			version: Main.version,
			swatches: Project.raw.swatches
		};
		var bytes = ArmPack.encode(raw);
		Krom.fileSaveBytes(path, bytes.getData(), bytes.length + 1);
	}

	static function vec3f32(v: iron.math.Vec4): kha.arrays.Float32Array {
		var res = new kha.arrays.Float32Array(3);
		res[0] = v.x;
		res[1] = v.y;
		res[2] = v.z;
		return res;
	}
}
