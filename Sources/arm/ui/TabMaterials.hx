package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Time;
import iron.system.Input;
import iron.object.MeshObject;
import iron.data.Data;
import arm.nodes.MaterialParser;
import arm.data.MaterialSlot;
import arm.util.RenderUtil;
import arm.io.ImportBlend;
import arm.io.ImportArm;
import arm.io.Exporter;
import arm.Tool;
using StringTools;

class TabMaterials {

	@:access(zui.Zui)
	public static function draw() {

		var ui = UITrait.inst.ui;
		var isScene = UITrait.inst.worktab.position == SpaceScene;
		var materials = isScene ? Project.materialsScene : Project.materials;
		var selectMaterial = isScene ? Context.selectMaterialScene : Context.selectMaterial;

		if (ui.tab(UITrait.inst.htab1, "Materials")) {
			ui.row([1/4,1/4,1/4]);
			if (ui.button("New")) {
				if (isScene) {
					if (Context.object != Context.paintObject && Std.is(Context.object, MeshObject)) {
						Context.removeMaterialCache();
						Data.getMaterial("Scene", "Material2", function(md:iron.data.MaterialData) {
							ui.g.end();
							md.name = "Material2." + materials.length;
							Context.materialScene = new MaterialSlot(md);
							materials.push(Context.materialScene);
							selectMaterial(materials.length - 1);
							RenderUtil.makeMaterialPreview();
							ui.g.begin(false);
						});
					}
				}
				else {
					ui.g.end();
					UITrait.inst.headerHandle.redraws = 2;
					Context.material = new MaterialSlot(materials[0].data);
					materials.push(Context.material);
					UINodes.inst.updateCanvasMap();
					MaterialParser.parsePaintMaterial();
					RenderUtil.makeMaterialPreview();
					var decal = Context.tool == ToolDecal || Context.tool == ToolText;
					if (decal) RenderUtil.makeDecalPreview();
					ui.g.begin(false);
				}
			}

			if (ui.button("Import")) {
				UIFiles.show = true;
				UIFiles.isSave = false;
				UIFiles.filters = "arm,blend";
				UIFiles.filesDone = function(path:String) {
					path.endsWith(".blend") ?
						ImportBlend.runMaterial(path) :
						ImportArm.runMaterial(path);
				}
			}

			if (ui.button("Nodes")) {
				UITrait.inst.showMaterialNodes();
			}
			else if (ui.isHovered) ui.tooltip("Show Node Editor (TAB)");

			var slotw = Std.int(51 * ui.SCALE);
			var num = Std.int(UITrait.inst.windowW / slotw);

			for (row in 0...Std.int(Math.ceil(materials.length / num))) {
				ui.row([for (i in 0...num) 1/num]);

				if (row > 0) ui._y += 6;

				for (j in 0...num) {
					var imgw = Std.int(50 * ui.SCALE);
					var i = j + row * num;
					if (i >= materials.length) {
						@:privateAccess ui.endElement(imgw);
						continue;
					}
					var img = ui.SCALE > 1 ? materials[i].image : materials[i].imageIcon;
					var imgFull = materials[i].image;

					if (getSelectedMaterial() == materials[i]) {
						// ui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
						var off = row % 2 == 1 ? 1 : 0;
						var w = 51 - Config.raw.window_scale;
						ui.fill(1,          -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						ui.fill(1,     w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						ui.fill(1,          -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						ui.fill(w + 3,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					#if (kha_opengl || kha_webgl)
					ui.imageInvertY = materials[i].previewReady;
					#end

					var uix = ui._x;
					var uiy = ui._y;
					var state = materials[i].previewReady ? ui.image(img) : ui.image(Res.get('icons.png'), -1, null, imgw, imgw, imgw, imgw);
					if (state == State.Started) {
						if (getSelectedMaterial() != materials[i]) selectMaterial(i);
						if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.showMaterialNodes();
						UITrait.inst.selectTime = Time.time();
						var mouse = Input.getMouse();
						App.dragOffX = -(mouse.x - uix - ui._windowX + iron.App.x() - 3);
						App.dragOffY = -(mouse.y - uiy - ui._windowY + iron.App.y() + 1);
						App.dragMaterial = getSelectedMaterial();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui:Zui) {
							var m = materials[i];
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 11, ui.t.SEPARATOR_COL);
							ui.text(UINodes.inst.canvasMap.get(materials[i]).name, Right);
							
							if (ui.button("To Fill Layer", Left)) {
								selectMaterial(i);
								Layers.createFillLayer();
							}

							if (ui.button("Export", Left)) {
								selectMaterial(i);
								UIFiles.show = true;
								UIFiles.isSave = true;
								UIFiles.filters = "arm";
								UIFiles.filesDone = function(path:String) {
									var f = UIFiles.filename;
									if (f == "") f = "untitled";
									Exporter.exportMaterial(path + "/" + f);
								};
							}

							if (ui.button("Delete", Left) && materials.length > 1) {
								selectMaterial(i == 0 ? 1 : 0);
								materials.splice(i, 1);
								UITrait.inst.hwnd1.redraws = 2;
							}
							
							var baseHandle = Id.handle().nest(m.id, {selected: m.paintBase});
							var norHandle = Id.handle().nest(m.id, {selected: m.paintNor});
							var occHandle = Id.handle().nest(m.id, {selected: m.paintOcc});
							var roughHandle = Id.handle().nest(m.id, {selected: m.paintRough});
							var metHandle = Id.handle().nest(m.id, {selected: m.paintMet});
							var heightHandle = Id.handle().nest(m.id, {selected: m.paintHeight});
							var emisHandle = Id.handle().nest(m.id, {selected: m.paintEmis});
							var subsHandle = Id.handle().nest(m.id, {selected: m.paintSubs});
							m.paintBase = ui.check(baseHandle, "Base Color");
							m.paintNor = ui.check(norHandle, "Normal");
							m.paintOcc = ui.check(occHandle, "Occlusion");
							m.paintRough = ui.check(roughHandle, "Roughness");
							m.paintMet = ui.check(metHandle, "Metallic");
							m.paintHeight = ui.check(heightHandle, "Height");
							m.paintEmis = ui.check(emisHandle, "Emission");
							m.paintSubs = ui.check(subsHandle, "Subsurface");
							if (baseHandle.changed ||
								norHandle.changed ||
								occHandle.changed ||
								roughHandle.changed ||
								metHandle.changed ||
								heightHandle.changed ||
								emisHandle.changed ||
								subsHandle.changed) {
								UINodes.inst.updateCanvasMap();
								MaterialParser.parsePaintMaterial();
								UIMenu.propChanged = true;
							}
						});
					}
					if (ui.isHovered) ui.tooltipImage(imgFull);
				}

				ui._y += 6;

				#if (kha_opengl || kha_webgl)
				ui.imageInvertY = false; // Material preview
				#end
			}
		}
	}

	static function getSelectedMaterial() { return UITrait.inst.worktab.position == SpaceScene ? Context.materialScene : Context.material; }
}
