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
using StringTools;

class ExportArm {

	public static function run(path:String) {
		var raw:TSceneFormat = { mesh_datas: [ Context.paintObject.data.raw ] };
		var b = ArmPack.encode(raw);
		if (!path.endsWith(".arm")) path += ".arm";
		Krom.fileSaveBytes(path, b.getData());
	}

	public static function runProject() {
		var mnodes:Array<TNodeCanvas> = [];
		var bnodes:Array<TNodeCanvas> = [];

		for (m in Project.materials) {
			var c:TNodeCanvas = Json.parse(Json.stringify(UINodes.inst.canvasMap.get(m)));
			for (n in c.nodes) {
				if (n.type == "TEX_IMAGE") {  // Convert image path from absolute to relative
					var sameDrive = Project.filepath.charAt(0) == n.buttons[0].data.charAt(0);
					if (sameDrive) {
						n.buttons[0].data = Path.toRelative(Project.filepath, n.buttons[0].data);
					}
				}
			}
			mnodes.push(c);
		}
		for (b in Project.brushes) bnodes.push(UINodes.inst.canvasBrushMap.get(b));

		var md:Array<TMeshData> = [];
		for (p in Project.paintObjects) md.push(p.data.raw);

		var asset_files:Array<String> = [];
		for (a in Project.assets) {
			// Convert image path from absolute to relative
			var sameDrive = Project.filepath.charAt(0) == a.file.charAt(0);
			if (sameDrive) {
				asset_files.push(Path.toRelative(Project.filepath, a.file));
			}
			else {
				asset_files.push(a.file);
			}
		}

		var bitsPos = UITrait.inst.bitsHandle.position;
		var bpp = bitsPos == 0 ? 8 : bitsPos == 1 ? 16 : 32;

		var ld:Array<TLayerData> = [];
		for (l in Project.layers) {
			ld.push({
				res: l.texpaint.width,
				bpp: bpp,
				texpaint: Lz4.encode(l.texpaint.getPixels()),
				texpaint_nor: Lz4.encode(l.texpaint_nor.getPixels()),
				texpaint_pack: Lz4.encode(l.texpaint_pack.getPixels()),
				texpaint_mask: l.texpaint_mask != null ? Lz4.encode(l.texpaint_mask.getPixels()) : null,
				opacity_mask: l.maskOpacity,
				material_mask: l.material_mask != null ? Project.materials.indexOf(l.material_mask) : -1,
				object_mask: l.objectMask
			});
		}

		Project.raw = {
			version: App.version,
			material_nodes: mnodes,
			brush_nodes: bnodes,
			mesh_datas: md,
			layer_datas: ld,
			assets: asset_files
		};
		
		var bytes = ArmPack.encode(Project.raw);
		Krom.fileSaveBytes(Project.filepath, bytes.getData());
	}
}
