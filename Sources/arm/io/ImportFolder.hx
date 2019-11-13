package arm.io;

import zui.Nodes;
import iron.data.Data;
import iron.data.MaterialData;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.util.RenderUtil;
import arm.util.MaterialUtil;
import arm.sys.Path;
import arm.sys.File;
import arm.nodes.NodesMaterial;
import arm.nodes.MaterialParser;
import arm.data.MaterialSlot;
import arm.Tool;
using StringTools;

class ImportFolder {

	public static function run(path:String) {
		var files = File.readDirectory(path);
		var mapbase = "";
		var mapopac = "";
		var mapnor = "";
		var mapocc = "";
		var maprough = "";
		var mapmet = "";
		var mapheight = "";

		// Import maps
		for (f in files) {
			if (!Path.isTexture(f)) continue;
			f = path + Path.sep + f;

			// TODO: handle -albedo

			var base = f.substr(0, f.lastIndexOf(".")).toLowerCase();
			var valid = false;
			if (mapbase == "" && Path.isBaseTex(base)) {
				mapbase = f;
				valid = true;
			}
			if (mapopac == "" && Path.isOpacTex(base)) {
				mapopac = f;
				valid = true;
			}
			if (mapnor == "" && Path.isNorTex(base)) {
				mapnor = f;
				valid = true;
			}
			if (mapocc == "" && Path.isOccTex(base)) {
				mapocc = f;
				valid = true;
			}
			if (maprough == "" && Path.isRoughTex(base)) {
				maprough = f;
				valid = true;
			}
			if (mapmet == "" && Path.isMetTex(base)) {
				mapmet = f;
				valid = true;
			}
			if (mapheight == "" && Path.isDispTex(base)) {
				mapheight = f;
				valid = true;
			}

			if (valid) ImportTexture.run(f);
		}

		// Create material
		var isScene = UITrait.inst.worktab.position == SpaceScene;
		if (isScene) {
			MaterialUtil.removeMaterialCache();
			Data.getMaterial("Scene", "Material2", function(md:MaterialData) {
				Context.materialScene = new MaterialSlot(md);
				Project.materialsScene.push(Context.materialScene);
			});
		}
		else {
			Context.material = new MaterialSlot(Project.materials[0].data);
			Project.materials.push(Context.material);
		}
		UINodes.inst.updateCanvasMap();
		var nodes = UINodes.inst.nodes;
		var canvas = UINodes.inst.canvas;
		var dirs = path.replace("\\", "/").split("/");
		canvas.name = dirs[dirs.length - 1];
		var nout:TNode = null;
		for (n in canvas.nodes) if (n.type == "OUTPUT_MATERIAL_PBR") { nout = n; break; }
		for (n in canvas.nodes) if (n.name == "RGB") { nodes.removeNode(n, canvas); break; }

		// Place nodes
		var pos = 0;
		var startY = 100;
		var nodeH = 164;
		if (mapbase != "") {
			var n = NodesMaterial.createImageTexture();
			n.buttons[0].default_value = App.getAssetIndex(mapbase);
			n.buttons[0].data = App.mapEnum(App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = startY + nodeH * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 0 };
			canvas.links.push(l);
		}
		if (mapopac != "") {
			var n = NodesMaterial.createImageTexture();
			n.buttons[0].default_value = App.getAssetIndex(mapopac);
			n.buttons[0].data = App.mapEnum(App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = startY + nodeH * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 1 };
			canvas.links.push(l);
		}
		if (mapocc != "") {
			var n = NodesMaterial.createImageTexture();
			n.buttons[0].default_value = App.getAssetIndex(mapocc);
			n.buttons[0].data = App.mapEnum(App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = startY + nodeH * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 2 };
			canvas.links.push(l);
		}
		if (maprough != "") {
			var n = NodesMaterial.createImageTexture();
			n.buttons[0].default_value = App.getAssetIndex(maprough);
			n.buttons[0].data = App.mapEnum(App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = startY + nodeH * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 3 };
			canvas.links.push(l);
		}
		if (mapmet != "") {
			var n = NodesMaterial.createImageTexture();
			n.buttons[0].default_value = App.getAssetIndex(mapmet);
			n.buttons[0].data = App.mapEnum(App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = startY + nodeH * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 4 };
			canvas.links.push(l);
		}
		if (mapnor != "") {
			var n = NodesMaterial.createImageTexture();
			n.buttons[0].default_value = App.getAssetIndex(mapnor);
			n.buttons[0].data = App.mapEnum(App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = startY + nodeH * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 5 };
			canvas.links.push(l);
		}
		if (mapheight != "") {
			var n = NodesMaterial.createImageTexture();
			n.buttons[0].default_value = App.getAssetIndex(mapheight);
			n.buttons[0].data = App.mapEnum(App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = startY + nodeH * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 7 };
			canvas.links.push(l);
		}

		MaterialParser.parsePaintMaterial();
		RenderUtil.makeMaterialPreview();
		UITrait.inst.hwnd1.redraws = 2;
	}
}
