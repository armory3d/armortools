package arm.ui;

import zui.Id;
import arm.util.UVUtil;
import arm.io.Exporter;

class TabExport {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "Export")) {
			if (ui.panel(Id.handle({selected: true}), "Export Textures", 1)) {
				if (ui.button("Export")) {
					UIFiles.show = true;
					UIFiles.isSave = true;
					// var path = 'C:\\Users\\lubos\\Documents\\';
					UIFiles.filters = UITrait.inst.bitsHandle.position > 0 ? "exr" : UITrait.inst.formatType == 0 ? "png" : "jpg";
					UIFiles.filesDone = function(path:String) {
						UITrait.inst.textureExport = true;
						UITrait.inst.textureExportPath = path;
					}
				}
				if (ui.isHovered) ui.tooltip("Export texture files (" + Config.keymap.export_textures + ")");

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
				UITrait.inst.outputType = ui.combo(Id.handle(), ["Generic", "Unreal 4", "Unity 5"], "Output", true);

				if (ui.panel(Id.handle({selected: false}), "Channels")) {
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
			}

			ui.separator();
			if (ui.panel(Id.handle({selected: false}), "Export Mesh", 1)) {
				if (ui.button("Export")) {
					UIFiles.show = true;
					UIFiles.isSave = true;
					UIFiles.filters = UITrait.inst.exportMeshFormat == 0 ? "obj" : "arm";
					UIFiles.filesDone = function(path:String) {
						var f = UIFiles.filename;
						if (f == "") f = "untitled";
						Exporter.exportMesh(path + "/" + f);
					};
				}
				UITrait.inst.exportMeshFormat = ui.combo(Id.handle({position: UITrait.inst.exportMeshFormat}), ["obj", "arm"], "Format", true);
				var mesh = Context.paintObject.data.raw;
				var inda = mesh.index_arrays[0].values;
				var tris = Std.int(inda.length / 3);
				ui.text(tris + " triangles");
			}
		}
	}
}
