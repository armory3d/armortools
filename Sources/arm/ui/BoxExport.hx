package arm.ui;

import zui.Zui;
import zui.Id;
import zui.Id;
import arm.util.UVUtil;
import arm.io.Exporter;

class BoxExport {

	public static function showTextures() {
		UIBox.showCustom(function(ui:Zui) {
			var htab = Id.handle();
			if (ui.tab(htab, "Export Textures")) {
				ui.row([1/2, 1/2]);
				ui.combo(UITrait.inst.resHandle, ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"], "Res", true);
				if (UITrait.inst.resHandle.changed) {
					iron.App.notifyOnRender(Layers.resizeLayers);
					UVUtil.uvmap = null;
					UVUtil.uvmapCached = false;
					UVUtil.trianglemap = null;
					UVUtil.trianglemapCached = false;
					#if kha_direct3d12
					arm.render.RenderPathRaytrace.ready = false;
					#end
				}
				ui.combo(UITrait.inst.bitsHandle, ["8bit", "16bit", "32bit"], "Color", true);
				if (UITrait.inst.bitsHandle.changed) {
					iron.App.notifyOnRender(Layers.setLayerBits);
				}

				ui.row([1/2, 1/2]);
				if (UITrait.inst.bitsHandle.position == 0) {
					UITrait.inst.formatType = ui.combo(Id.handle({position: UITrait.inst.formatType}), ["png", "jpg"], "Format", true);
				}
				else {
					ui.combo(Id.handle({position: UITrait.inst.formatType}), ["exr"], "Format", true);
				}
				ui.enabled = UITrait.inst.formatType == 1 && UITrait.inst.bitsHandle.position == 0;
				UITrait.inst.formatQuality = ui.slider(Id.handle({value: UITrait.inst.formatQuality}), "Quality", 0.0, 100.0, true, 1);
				ui.enabled = true;
				ui.row([1/2, 1/2]);
				UITrait.inst.layersExport = ui.combo(Id.handle({position: UITrait.inst.layersExport}), ["Visible", "Selected"], "Layers", true);
				UITrait.inst.outputType = ui.combo(Id.handle({position: UITrait.inst.outputType}), ["Generic", "Unreal 4", "Unity 5"], "Output", true);

				@:privateAccess ui.endElement();

				ui.row([1/2, 1/2]);
				if (ui.button("Cancel")) {
					UIBox.show = false;
				}
				if (ui.button("Export")) {
					UIBox.show = false;
					var filters = UITrait.inst.bitsHandle.position > 0 ? "exr" : UITrait.inst.formatType == 0 ? "png" : "jpg";
					UIFiles.show(filters, true, function(path:String) {
						UITrait.inst.textureExport = true;
						UITrait.inst.textureExportPath = path;
					});
				}
				if (ui.isHovered) ui.tooltip("Export texture files (" + Config.keymap.file_export_textures + ")");
			}
			if (ui.tab(htab, "Channels")) {
				ui.row([1/2, 1/2]);
				UITrait.inst.isBase = ui.check(Id.handle({selected: UITrait.inst.isBase}), "Base Color");
				UITrait.inst.isBaseSpace = ui.combo(Id.handle({position: UITrait.inst.isBaseSpace}), ["linear", "srgb"], "Space");
				ui.row([1/2, 1/2]);
				UITrait.inst.isOpac = ui.check(Id.handle({selected: UITrait.inst.isOpac}), "Opacity");
				UITrait.inst.isOpacSpace = ui.combo(Id.handle({position: UITrait.inst.isOpacSpace}), ["linear", "srgb"], "Space");
				ui.row([1/2, 1/2]);
				UITrait.inst.isOcc = ui.check(Id.handle({selected: UITrait.inst.isOcc}), "Occlusion");
				UITrait.inst.isOccSpace = ui.combo(Id.handle({position: UITrait.inst.isOccSpace}), ["linear", "srgb"], "Space");
				ui.row([1/2, 1/2]);
				UITrait.inst.isRough = ui.check(Id.handle({selected: UITrait.inst.isRough}), "Roughness");
				UITrait.inst.isRoughSpace = ui.combo(Id.handle({position: UITrait.inst.isRoughSpace}), ["linear", "srgb"], "Space");
				ui.row([1/2, 1/2]);
				UITrait.inst.isMet = ui.check(Id.handle({selected: UITrait.inst.isMet}), "Metallic");
				UITrait.inst.isMetSpace = ui.combo(Id.handle({position: UITrait.inst.isMetSpace}), ["linear", "srgb"], "Space");
				ui.row([1/2, 1/2]);
				UITrait.inst.isNor = ui.check(Id.handle({selected: UITrait.inst.isNor}), "Normal");
				UITrait.inst.isNorSpace = ui.combo(Id.handle({position: UITrait.inst.isNorSpace}), ["linear", "srgb"], "Space");
				ui.row([1/2, 1/2]);
				UITrait.inst.isEmis = ui.check(Id.handle({selected: UITrait.inst.isEmis}), "Emission");
				UITrait.inst.isEmisSpace = ui.combo(Id.handle({position: UITrait.inst.isEmisSpace}), ["linear", "srgb"], "Space");
				ui.row([1/2, 1/2]);
				UITrait.inst.isHeight = ui.check(Id.handle({selected: UITrait.inst.isHeight}), "Height");
				UITrait.inst.isHeightSpace = ui.combo(Id.handle({position: UITrait.inst.isHeightSpace}), ["linear", "srgb"], "Space");
				ui.row([1/2, 1/2]);
				UITrait.inst.isSubs = ui.check(Id.handle({selected: UITrait.inst.isSubs}), "Subsurface");
				UITrait.inst.isSubsSpace = ui.combo(Id.handle({position: UITrait.inst.isSubsSpace}), ["linear", "srgb"], "Space");
			}
		}, 500, 310);
	}

	public static function showMesh() {
		UIBox.showCustom(function(ui:Zui) {
			var htab = Id.handle();
			if (ui.tab(htab, "Export Mesh")) {

				UITrait.inst.exportMeshFormat = ui.combo(Id.handle({position: UITrait.inst.exportMeshFormat}), ["obj", "arm"], "Format", true);
				var mesh = Context.paintObject.data.raw;
				var inda = mesh.index_arrays[0].values;
				var tris = Std.int(inda.length / 3);
				ui.text(tris + " triangles");

				@:privateAccess ui.endElement();

				ui.row([1/2, 1/2]);
				if (ui.button("Cancel")) {
					UIBox.show = false;
				}
				if (ui.button("Export")) {
					UIBox.show = false;
					UIFiles.show(UITrait.inst.exportMeshFormat == 0 ? "obj" : "arm", true, function(path:String) {
						var f = UIFiles.filename;
						if (f == "") f = "untitled";
						Exporter.exportMesh(path + "/" + f);
					});
				}
			}
		});
	}
}
