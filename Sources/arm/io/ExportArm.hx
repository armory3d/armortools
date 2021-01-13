package arm.io;

import haxe.Json;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.system.ArmPack;
import iron.system.Lz4;
import arm.data.FontSlot;
import arm.ui.UISidebar;
import arm.sys.Path;
import arm.ProjectFormat;
import arm.Enums;

class ExportArm {

	public static function run(path: String) {
		var raw: TSceneFormat = { mesh_datas: [ Context.paintObject.data.raw ] };
		var b = ArmPack.encode(raw);
		if (!path.endsWith(".arm")) path += ".arm";
		Krom.fileSaveBytes(path, b.getData());
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

		var texture_files = assetsToFiles(Project.assets);
		var font_files = fontsToFiles(Project.fonts);
		var mesh_files = meshesToFiles();

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
				texpaint_mask: l.texpaint_mask != null ? Lz4.encode(l.texpaint_mask.getPixels()) : null,
				uv_scale: l.scale,
				uv_rot: l.angle,
				uv_type: l.uvType,
				decal_mat: l.uvType == UVProject ? l.decalMat.toFloat32Array() : null,
				opacity_mask: l.maskOpacity,
				fill_layer: l.fill_layer != null ? Project.materials.indexOf(l.fill_layer) : -1,
				fill_mask: l.fill_mask != null ? Project.materials.indexOf(l.fill_mask) : -1,
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
				paint_height: l.paintHeight,
				paint_emis: l.paintEmis,
				paint_subs: l.paintSubs
			});
		}

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
			#if (kha_metal || kha_vulkan)
			is_bgra: true
			#else
			is_bgra: false
			#end
		};

		var bytes = ArmPack.encode(Project.raw);
		Krom.fileSaveBytes(Project.filepath, bytes.getData());

		// Save to recent
		var recent = Config.raw.recent_projects;
		recent.remove(Project.filepath);
		recent.unshift(Project.filepath);
		Config.save();

		Log.info("Project saved.");
	}

	static function getGroup(canvases: Array<TNodeCanvas>, name: String): TNodeCanvas {
		for (c in canvases) if (c.name == name) return c;
		return null;
	}

	static function hasGroup(c: TNodeCanvas): Bool {
		for (n in c.nodes) if (n.type == "GROUP") return true;
		return false;
	}

	static function traverseGroup(mgroups: Array<TNodeCanvas>, c: TNodeCanvas) {
		for (n in c.nodes) {
			if (n.type == "GROUP") {
				if (getGroup(mgroups, n.name) == null) {
					var canvases: Array<TNodeCanvas> = [];
					for (g in Project.materialGroups) canvases.push(g.canvas);
					var group = getGroup(canvases, n.name);
					mgroups.push(Json.parse(Json.stringify(group)));
					traverseGroup(mgroups, group);
				}
			}
		}
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
	}

	public static function runMaterial(path: String) {
		if (!path.endsWith(".arm")) path += ".arm";
		var mnodes: Array<TNodeCanvas> = [];
		var mgroups: Array<TNodeCanvas> = null;
		var m = Context.material;
		var c: TNodeCanvas = Json.parse(Json.stringify(m.canvas));
		var assets: Array<TAsset> = [];
		if (hasGroup(c)) {
			mgroups = [];
			traverseGroup(mgroups, c);
			for (gc in mgroups) for (n in gc.nodes) exportNode(n, assets);
		}
		for (n in c.nodes) exportNode(n, assets);
		mnodes.push(c);

		var texture_files = assetsToFiles(assets);

		var isCloud = path.endsWith("_cloud_.arm");
		if (isCloud) {
			path = path.replace("_cloud_", "");
			// Separate icon files
			var out = new haxe.io.BytesOutput();
			var writer = new arm.format.JpgWriter(out);
			writer.write(
				{
					width: m.image.width,
					height: m.image.height,
					quality: 50,
					pixels: m.image.getPixels()
				}, 1
			);
			Krom.fileSaveBytes(path.substr(0, path.length - 4) + "_icon.jpg", out.getBytes().getData());
			var out = new haxe.io.BytesOutput();
			var writer = new arm.format.PngWriter(out);
			var data = arm.format.PngTools.build32RGBA(m.image.width, m.image.height, m.image.getPixels());
			writer.write(data);
			Krom.fileSaveBytes(path.substr(0, path.length - 4) + "_icon.png", out.getBytes().getData());
		}

		var raw = {
			version: Main.version,
			material_nodes: mnodes,
			material_groups: mgroups,
			material_icons: isCloud ? null :
				#if (kha_metal || kha_vulkan)
				[Lz4.encode(bgraSwap(m.image.getPixels()))],
				#else
				[Lz4.encode(m.image.getPixels())],
				#end
			assets: texture_files
		};

		var bytes = ArmPack.encode(raw);
		Krom.fileSaveBytes(path, bytes.getData());
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
		var bnodes: Array<TNodeCanvas> = [];
		var b = Context.brush;
		var c: TNodeCanvas = Json.parse(Json.stringify(b.canvas));
		var assets: Array<TAsset> = [];
		for (n in c.nodes) exportNode(n, assets);
		bnodes.push(c);

		var texture_files = assetsToFiles(assets);

		var raw = {
			version: Main.version,
			brush_nodes: bnodes,
			brush_icons: [Lz4.encode(b.image.getPixels())],
			assets: texture_files
		};

		var bytes = ArmPack.encode(raw);
		if (!path.endsWith(".arm")) path += ".arm";
		Krom.fileSaveBytes(path, bytes.getData());
	}

	static function assetsToFiles(assets: Array<TAsset>): Array<String> {
		var texture_files: Array<String> = [];
		for (a in assets) {
			// Convert image path from absolute to relative
			var sameDrive = Project.filepath.charAt(0) == a.file.charAt(0);
			if (sameDrive) {
				texture_files.push(Path.toRelative(Project.filepath, a.file));
			}
			else {
				texture_files.push(a.file);
			}
		}
		return texture_files;
	}

	static function meshesToFiles(): Array<String> {
		var mesh_files: Array<String> = [];
		for (file in Project.meshAssets) {
			// Convert mesh path from absolute to relative
			var sameDrive = Project.filepath.charAt(0) == file.charAt(0);
			if (sameDrive) {
				mesh_files.push(Path.toRelative(Project.filepath, file));
			}
			else {
				mesh_files.push(file);
			}
		}
		return mesh_files;
	}

	static function fontsToFiles(fonts: Array<FontSlot>): Array<String> {
		var font_files: Array<String> = [];
		for (i in 1...fonts.length) {
			var f = fonts[i];
			// Convert font path from absolute to relative
			var sameDrive = Project.filepath.charAt(0) == f.file.charAt(0);
			if (sameDrive) {
				font_files.push(Path.toRelative(Project.filepath, f.file));
			}
			else {
				font_files.push(f.file);
			}
		}
		return font_files;
	}
}
