package arm.io;

import haxe.Json;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.system.ArmPack;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.util.Lz4;
import arm.util.Path;
import arm.Project;
import arm.App;

class ExportArm {

	public static function run(path:String) {
		var raw:TSceneFormat = { mesh_datas: [ UITrait.inst.paintObject.data.raw ] };
		var b = ArmPack.encode(raw);
		if (!StringTools.endsWith(path, ".arm")) path += ".arm";
		#if kha_krom
		Krom.fileSaveBytes(path, b.getData());
		#end
	}

	public static function runProject() {
		var mnodes:Array<TNodeCanvas> = [];
		var bnodes:Array<TNodeCanvas> = [];

		for (m in UITrait.inst.materials) {
			var c:TNodeCanvas = Json.parse(Json.stringify(UINodes.inst.canvasMap.get(m)));
			for (n in c.nodes) {
				if (n.type == "TEX_IMAGE") {  // Convert image path from absolute to relative
					var sameDrive = UITrait.inst.projectPath.charAt(0) == n.buttons[0].data.charAt(0);
					if (sameDrive) {
						n.buttons[0].data = Path.toRelative(UITrait.inst.projectPath, n.buttons[0].data);
					}
				}
			}
			mnodes.push(c);
		}
		for (b in UITrait.inst.brushes) bnodes.push(UINodes.inst.canvasBrushMap.get(b));

		var md:Array<TMeshData> = [];
		for (p in UITrait.inst.paintObjects) md.push(p.data.raw);

		var asset_files:Array<String> = [];
		for (a in UITrait.inst.assets) {
			// Convert image path from absolute to relative
			var sameDrive = UITrait.inst.projectPath.charAt(0) == a.file.charAt(0);
			if (sameDrive) {
				asset_files.push(Path.toRelative(UITrait.inst.projectPath, a.file));
			}
			else {
				asset_files.push(a.file);
			}
		}

		var ld:Array<TLayerData> = [];
		for (l in UITrait.inst.layers) {
			ld.push({
				res: l.texpaint.width,
				texpaint: Lz4.encode(l.texpaint.getPixels()),
				texpaint_nor: Lz4.encode(l.texpaint_nor.getPixels()),
				texpaint_pack: Lz4.encode(l.texpaint_pack.getPixels()),
				texpaint_mask: l.texpaint_mask != null ? Lz4.encode(l.texpaint_mask.getPixels()) : null,
				opacity_mask: l.maskOpacity,
				material_mask: l.material_mask != null ? UITrait.inst.materials.indexOf(l.material_mask) : -1,
				object_mask: l.objectMask
			});
		}

		UITrait.inst.project = {
			version: App.version,
			material_nodes: mnodes,
			brush_nodes: bnodes,
			mesh_datas: md,
			layer_datas: ld,
			assets: asset_files
		};
		
		var bytes = ArmPack.encode(UITrait.inst.project);

		#if kha_krom
		Krom.fileSaveBytes(UITrait.inst.projectPath, bytes.getData());
		#end
	}
}
