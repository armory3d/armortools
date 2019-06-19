package arm.io;

import zui.Nodes;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.util.RenderUtil;
import arm.creator.NodeCreator;
import arm.Tool;

class ImportFolder {

	public static function run(path:String) {
		#if kha_krom
		#if krom_windows
		var cmd = "dir /b ";
		var sep = "\\";
		#else
		var cmd = "ls ";
		var sep = "/";
		#end
		#if krom_linux
		var save = "/tmp";
		#else
		var save = Krom.savePath();
		#end
		save += sep + "dir.txt";
		Krom.sysCommand(cmd + '"' + path + '"' + ' > ' + '"' + save + '"');
		var str = haxe.io.Bytes.ofData(Krom.loadBlob(save)).toString();
		var files = str.split("\n");
		var mapbase = "";
		var mapopac = "";
		var mapnor = "";
		var mapocc = "";
		var maprough = "";
		var mapmet = "";
		var mapheight = "";
		// Import maps
		for (f in files) {
			if (f.length == 0) continue;
			f = StringTools.rtrim(f);
			if (!Format.checkTextureFormat(f)) continue;
			
			f = path + sep + f;
			#if krom_windows
			f = StringTools.replace(f, "/", "\\");
			#end

			// TODO: handle -albedo
			
			var base = f.substr(0, f.lastIndexOf(".")).toLowerCase();
			var valid = false;
			if (mapbase == "" && Format.checkBaseTex(base)) {
				mapbase = f;
				valid = true;
			}
			if (mapopac == "" && Format.checkOpacTex(base)) {
				mapopac = f;
				valid = true;
			}
			if (mapnor == "" && Format.checkNorTex(base)) {
				mapnor = f;
				valid = true;
			}
			if (mapocc == "" && Format.checkOccTex(base)) {
				mapocc = f;
				valid = true;
			}
			if (maprough == "" && Format.checkRoughTex(base)) {
				maprough = f;
				valid = true;
			}
			if (mapmet == "" && Format.checkMetTex(base)) {
				mapmet = f;
				valid = true;
			}
			if (mapheight == "" && Format.checkDispTex(base)) {
				mapheight = f;
				valid = true;
			}

			if (valid) arm.io.ImportTexture.run(f);
		}
		// Create material
		var isScene = UITrait.inst.worktab.position == SpaceScene;
		if (isScene) {
			UITrait.inst.removeMaterialCache();
			iron.data.Data.getMaterial("Scene", "Material2", function(md:iron.data.MaterialData) {
				UITrait.inst.selectedMaterialScene = new MaterialSlot(md);
				UITrait.inst.materialsScene.push(UITrait.inst.selectedMaterialScene);
			});
		}
		else {
			UITrait.inst.selectedMaterial = new MaterialSlot();
			UITrait.inst.materials.push(UITrait.inst.selectedMaterial);
		}
		UINodes.inst.updateCanvasMap();
		var nodes = UINodes.inst.nodes;
		var canvas = UINodes.inst.canvas;
		var dirs = StringTools.replace(path, "\\", "/").split("/");
		canvas.name = dirs[dirs.length - 1];
		var nout:TNode = null;
		for (n in canvas.nodes) if (n.type == "OUTPUT_MATERIAL_PBR") { nout = n; break; }
		for (n in canvas.nodes) if (n.name == "RGB") { nodes.removeNode(n, canvas); break; }
		
		var pos = 0;
		if (mapbase != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapbase);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 0 };
			canvas.links.push(l);
		}
		if (mapopac != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapopac);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 1 };
			canvas.links.push(l);
		}
		if (mapocc != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapocc);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 2 };
			canvas.links.push(l);
		}
		if (maprough != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(maprough);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 3 };
			canvas.links.push(l);
		}
		if (mapmet != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapmet);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 4 };
			canvas.links.push(l);
		}
		if (mapnor != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapnor);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 5 };
			canvas.links.push(l);
		}
		if (mapheight != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapheight);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 7 };
			canvas.links.push(l);
		}
		arm.MaterialParser.parsePaintMaterial();
		RenderUtil.makeMaterialPreview();
		UITrait.inst.hwnd1.redraws = 2;
		#end
	}
}
