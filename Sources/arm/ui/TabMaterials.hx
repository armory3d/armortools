package arm.ui;

import haxe.Json;
import zui.Zui;
import zui.Id;
import iron.system.Time;
import iron.system.Input;
import iron.object.MeshObject;
import iron.data.Data;
import arm.node.MaterialParser;
import arm.data.MaterialSlot;
import arm.util.RenderUtil;
import arm.util.MaterialUtil;
import arm.io.ExportArm;
import arm.Tool;

class TabMaterials {

	@:access(zui.Zui)
	public static function draw() {

		var ui = UITrait.inst.ui;
		var isScene = UITrait.inst.worktab.position == SpaceScene;
		var materials = isScene ? Project.materialsScene : Project.materials;
		var selectMaterial = isScene ? Context.selectMaterialScene : Context.selectMaterial;

		if (ui.tab(UITrait.inst.htab1, "Materials")) {
			ui.row([1 / 4, 1 / 4, 1 / 4]);
			if (ui.button("New")) {
				if (isScene) {
					if (Context.object != Context.paintObject && Std.is(Context.object, MeshObject)) {
						MaterialUtil.removeMaterialCache();
						Data.getMaterial("Scene", "Material2", function(md: iron.data.MaterialData) {
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
					Context.material = new MaterialSlot(materials[0].data);
					materials.push(Context.material);
					updateMaterial();
					ui.g.begin(false);
				}
			}

			if (ui.button("Import")) {
				Project.importMaterial();
			}

			if (ui.button("Nodes")) {
				UITrait.inst.showMaterialNodes();
			}
			else if (ui.isHovered) ui.tooltip("Show Node Editor (" + Config.keymap.toggle_2d_view + ")");

			var slotw = Std.int(51 * ui.SCALE());
			var num = Std.int(UITrait.inst.windowW / slotw);

			for (row in 0...Std.int(Math.ceil(materials.length / num))) {
				ui.row([for (i in 0...num) 1 / num]);

				ui._x += 2;
				if (row > 0) ui._y += 6;

				for (j in 0...num) {
					var imgw = Std.int(50 * ui.SCALE());
					var i = j + row * num;
					if (i >= materials.length) {
						@:privateAccess ui.endElement(imgw);
						continue;
					}
					var img = ui.SCALE() > 1 ? materials[i].image : materials[i].imageIcon;
					var imgFull = materials[i].image;

					if (getSelectedMaterial() == materials[i]) {
						// ui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
						var off = row % 2 == 1 ? 1 : 0;
						var w = 50;
						if (Config.raw.window_scale > 1) w += Std.int(Config.raw.window_scale * 2);
						ui.fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						ui.fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						ui.fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						ui.fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					#if (kha_opengl || kha_webgl)
					ui.imageInvertY = materials[i].previewReady;
					#end

					var uix = ui._x;
					var uiy = ui._y;
					var tile = ui.SCALE() > 1 ? 100 : 50;
					var state = materials[i].previewReady ? ui.image(img) : ui.image(Res.get("icons.k"), -1, null, tile, tile, tile, tile);
					if (state == State.Started && ui.inputY > ui._windowY) {
						if (getSelectedMaterial() != materials[i]) selectMaterial(i);
						var mouse = Input.getMouse();
						App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						App.dragMaterial = getSelectedMaterial();
						if (Time.time() - UITrait.inst.selectTime < 0.25) {
							UITrait.inst.showMaterialNodes();
							App.dragMaterial = null;
							App.isDragging = false;
						}
						UITrait.inst.selectTime = Time.time();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui: Zui) {
							var m = materials[i];
							var add = materials.length > 1 ? 1 : 0;
							ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * (12 + add), ui.t.SEPARATOR_COL);
							ui.text(materials[i].canvas.name, Right, ui.t.HIGHLIGHT_COL);

							if (ui.button("To Fill Layer", Left)) {
								selectMaterial(i);
								Layers.createFillLayer();
							}

							if (ui.button("Export", Left)) {
								selectMaterial(i);
								UIFiles.show("arm", true, function(path: String) {
									var f = UIFiles.filename;
									if (f == "") f = "untitled";
									ExportArm.runMaterial(path + "/" + f);
								});
							}

							if (ui.button("Duplicate", Left)) {
								function dupliMat(_) {
									iron.App.removeRender(dupliMat);
									Context.material = new MaterialSlot(materials[0].data);
									materials.push(Context.material);
									var cloned = Json.parse(Json.stringify(materials[i].canvas));
									Context.material.canvas = cloned;
									updateMaterial();
								}
								iron.App.notifyOnRender(dupliMat);
							}

							if (materials.length > 1 && ui.button("Delete", Left)) {
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
								MaterialParser.parsePaintMaterial();
								UIMenu.keepOpen = true;
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

	static function updateMaterial() {
		UITrait.inst.headerHandle.redraws = 2;
		MaterialParser.parsePaintMaterial();
		RenderUtil.makeMaterialPreview();
		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		if (decal) RenderUtil.makeDecalPreview();
	}

	static function getSelectedMaterial():MaterialSlot {
		return UITrait.inst.worktab.position == SpaceScene ? Context.materialScene : Context.material;
	}
}
