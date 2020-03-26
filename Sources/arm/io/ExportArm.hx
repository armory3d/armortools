package arm.io;

import haxe.Json;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.system.ArmPack;
import arm.ui.UISidebar;
import arm.format.Lz4;
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
		var bnodes: Array<TNodeCanvas> = [];

		for (m in Project.materials) {
			var c: TNodeCanvas = Json.parse(Json.stringify(m.canvas));
			for (n in c.nodes) {
				if (n.type == "TEX_IMAGE") {
					n.buttons[0].data = App.enumTexts(n.type)[n.buttons[0].default_value];
				}
			}
			mnodes.push(c);
		}
		for (b in Project.brushes) bnodes.push(b.canvas);

		var md: Array<TMeshData> = [];
		for (p in Project.paintObjects) md.push(p.data.raw);

		var texture_files = assetsToFiles(Project.assets);
		var mesh_files = meshesToFiles();

		var bitsPos = App.bitsHandle.position;
		var bpp = bitsPos == Bits8 ? 8 : bitsPos == Bits16 ? 16 : 32;

		var ld: Array<TLayerData> = [];
		for (l in Project.layers) {
			ld.push({
				res: l.texpaint != null ? l.texpaint.width : Project.layers[0].texpaint.width,
				bpp: bpp,
				texpaint: l.texpaint != null ? Lz4.encode(l.texpaint.getPixels()) : null,
				texpaint_nor: l.texpaint_nor != null ? Lz4.encode(l.texpaint_nor.getPixels()) : null,
				texpaint_pack: l.texpaint_pack != null ? Lz4.encode(l.texpaint_pack.getPixels()) : null,
				texpaint_mask: l.texpaint_mask != null ? Lz4.encode(l.texpaint_mask.getPixels()) : null,
				uv_scale: l.scale,
				uv_rot: l.angle,
				uv_type: l.uvType,
				opacity_mask: l.maskOpacity,
				material_mask: l.material_mask != null ? Project.materials.indexOf(l.material_mask) : -1,
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
			brush_nodes: bnodes,
			mesh_datas: md,
			layer_datas: ld,
			assets: texture_files,
			mesh_assets: mesh_files
		};

		var bytes = ArmPack.encode(Project.raw);
		Krom.fileSaveBytes(Project.filepath, bytes.getData());

		Log.info("Project saved.");
	}

	public static function runMaterial(path: String) {
		var mnodes: Array<TNodeCanvas> = [];
		var m = Context.material;
		var c: TNodeCanvas = Json.parse(Json.stringify(m.canvas));
		var assets: Array<TAsset> = [];
		for (n in c.nodes) {
			if (n.type == "TEX_IMAGE") {
				var index = n.buttons[0].default_value;
				n.buttons[0].data = App.enumTexts(n.type)[index];

				var asset = Project.assets[index];
				if (assets.indexOf(asset) == -1) {
					assets.push(asset);
				}
			}
		}
		mnodes.push(c);

		var texture_files = assetsToFiles(assets);

		var raw = {
			version: Main.version,
			material_nodes: mnodes,
			material_icons: [Lz4.encode(m.image.getPixels())],
			assets: texture_files
		};

		var bytes = ArmPack.encode(raw);
		if (!path.endsWith(".arm")) path += ".arm";
		Krom.fileSaveBytes(path, bytes.getData());
	}

	public static function runBrush(path: String) {
		var bnodes: Array<TNodeCanvas> = [];
		var b = Context.brush;
		var c: TNodeCanvas = Json.parse(Json.stringify(b.canvas));
		var assets: Array<TAsset> = [];
		for (n in c.nodes) {
			if (n.type == "TEX_IMAGE") {
				var index = n.buttons[0].default_value;
				n.buttons[0].data = App.enumTexts(n.type)[index];

				var asset = Project.assets[index];
				if (assets.indexOf(asset) == -1) {
					assets.push(asset);
				}
			}
		}
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
}
