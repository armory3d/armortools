package arm.ui;

import zui.Zui;
import zui.Id;
import iron.math.Vec4;

@:access(zui.Zui)
class TabObjects {

	static var materialId = 0;

	static function roundfp(f: Float, precision = 2): Float {
		f *= std.Math.pow(10, precision);
		return std.Math.round(f) / std.Math.pow(10, precision);
	}

	public static function draw(htab: Handle) {
		var ui = UIBase.inst.ui;
		if (ui.tab(htab, tr("Objects"))) {
			ui.beginSticky();
			ui.row([1 / 4]);
			if (ui.button("Import")) {
				Project.importMesh(false, function() {
					Project.paintObjects.pop().setParent(null);
				});
			}
			ui.endSticky();

			if (ui.panel(Id.handle({selected: true}), "Outliner")) {
				ui.indent();
				ui._y -= ui.ELEMENT_OFFSET();

				var listX = ui._x;
				var listW = ui._w;

				var lineCounter = 0;
				function drawList(listHandle: zui.Zui.Handle, currentObject: iron.object.Object) {
					if (currentObject.name.charAt(0) == ".") return; // Hidden
					var b = false;

					// Highlight every other line
					if (lineCounter % 2 == 0) {
						ui.g.color = ui.t.SEPARATOR_COL;
						ui.g.fillRect(0, ui._y, ui._windowW, ui.ELEMENT_H());
						ui.g.color = 0xffffffff;
					}

					// Highlight selected line
					if (currentObject == Context.raw.selectedObject) {
						ui.g.color = 0xff205d9c;
						ui.g.fillRect(0, ui._y, ui._windowW, ui.ELEMENT_H());
						ui.g.color = 0xffffffff;
					}

					if (currentObject.children.length > 0) {
						ui.row([1 / 13, 12 / 13]);
						b = ui.panel(listHandle.nest(lineCounter, {selected: true}), "", true, false, false);
						ui.text(currentObject.name);
					}
					else {
						ui._x += 18; // Sign offset

						// Draw line that shows parent relations
						ui.g.color = ui.t.ACCENT_COL;
						ui.g.drawLine(ui._x - 10, ui._y + ui.ELEMENT_H() / 2, ui._x, ui._y + ui.ELEMENT_H() / 2);
						ui.g.color = 0xffffffff;

						ui.text(currentObject.name);
						ui._x -= 18;
					}

					lineCounter++;
					// Undo applied offset for row drawing caused by endElement() in Zui.hx
					ui._y -= ui.ELEMENT_OFFSET();

					if (ui.isReleased) {
						Context.raw.selectedObject = currentObject;
					}

					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui: Zui) {
							if (UIMenu.menuButton(ui, "Assign Material")) {
								materialId++;

								for (sh in iron.Scene.active.raw.shader_datas) {
									if (sh.name == "Material_data") {
										var s: iron.data.SceneFormat.TShaderData = haxe.Json.parse(haxe.Json.stringify(sh));
										s.name = "TempMaterial_data" + materialId;
										iron.Scene.active.raw.shader_datas.push(s);
										break;
									}
								}

								for (mat in iron.Scene.active.raw.material_datas) {
									if (mat.name == "Material") {
										var m: iron.data.SceneFormat.TMaterialData = haxe.Json.parse(haxe.Json.stringify(mat));
										m.name = "TempMaterial" + materialId;
										m.shader = "TempMaterial_data" + materialId;
										iron.Scene.active.raw.material_datas.push(m);
										break;
									}
								}

								iron.data.Data.getMaterial("Scene", "TempMaterial" + materialId, function(md: iron.data.MaterialData) {
									var mo: iron.object.MeshObject = cast currentObject;
									mo.materials = haxe.ds.Vector.fromArrayCopy([md]);
									arm.shader.MakeMaterial.parseMeshPreviewMaterial(md);
								});
							}
						}, 1);
					}

					if (b) {
						var currentY = ui._y;
						for (child in currentObject.children) {
							ui.indent();
							drawList(listHandle, child);
							ui.unindent();
						}

						// Draw line that shows parent relations
						ui.g.color = ui.t.ACCENT_COL;
						ui.g.drawLine(ui._x + 14, currentY, ui._x + 14, ui._y - ui.ELEMENT_H() / 2);
						ui.g.color = 0xffffffff;
					}
				}
				for (c in iron.Scene.active.root.children) {
					drawList(Id.handle(), c);
				}

				ui.unindent();
			}

			if (ui.panel(Id.handle({selected: true}), 'Properties')) {
				ui.indent();

				if (Context.raw.selectedObject != null) {
					var h = Id.handle();
					h.selected = Context.raw.selectedObject.visible;
					Context.raw.selectedObject.visible = ui.check(h, "Visible");

					var t = Context.raw.selectedObject.transform;
					var localPos = t.loc;
					var worldPos = new Vec4(t.worldx(), t.worldy(), t.worldz(), 1.0);
					var scale = t.scale;
					var rot = t.rot.getEuler();
					var dim = t.dim;
					rot.mult(180 / 3.141592);
					var f = 0.0;

					ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					ui.text("Loc");

					h = Id.handle();
					h.text = roundfp(localPos.x) + "";
					f = Std.parseFloat(ui.textInput(h, "X"));
					if (h.changed) localPos.x = f;

					h = Id.handle();
					h.text = roundfp(localPos.y) + "";
					f = Std.parseFloat(ui.textInput(h, "Y"));
					if (h.changed) localPos.y = f;

					h = Id.handle();
					h.text = roundfp(localPos.z) + "";
					f = Std.parseFloat(ui.textInput(h, "Z"));
					if (h.changed) localPos.z = f;

					ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					ui.text("Rotation");

					h = Id.handle();
					h.text = roundfp(rot.x) + "";
					f = Std.parseFloat(ui.textInput(h, "X"));
					var changed = false;
					if (h.changed) { changed = true; rot.x = f; }

					h = Id.handle();
					h.text = roundfp(rot.y) + "";
					f = Std.parseFloat(ui.textInput(h, "Y"));
					if (h.changed) { changed = true; rot.y = f; }

					h = Id.handle();
					h.text = roundfp(rot.z) + "";
					f = Std.parseFloat(ui.textInput(h, "Z"));
					if (h.changed) { changed = true; rot.z = f; }

					if (changed && Context.raw.selectedObject.name != "Scene") {
						rot.mult(3.141592 / 180);
						Context.raw.selectedObject.transform.rot.fromEuler(rot.x, rot.y, rot.z);
						Context.raw.selectedObject.transform.buildMatrix();
						// #if arm_physics
						// var rb = Context.raw.selectedObject.getTrait(armory.trait.physics.RigidBody);
						// if (rb != null) rb.syncTransform();
						// #end
					}

					ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					ui.text("Scale");

					h = Id.handle();
					h.text = roundfp(scale.x) + "";
					f = Std.parseFloat(ui.textInput(h, "X"));
					if (h.changed) scale.x = f;

					h = Id.handle();
					h.text = roundfp(scale.y) + "";
					f = Std.parseFloat(ui.textInput(h, "Y"));
					if (h.changed) scale.y = f;

					h = Id.handle();
					h.text = roundfp(scale.z) + "";
					f = Std.parseFloat(ui.textInput(h, "Z"));
					if (h.changed) scale.z = f;

					ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					ui.text("Dimensions");

					h = Id.handle();
					h.text = roundfp(dim.x) + "";
					f = Std.parseFloat(ui.textInput(h, "X"));
					if (h.changed) dim.x = f;

					h = Id.handle();
					h.text = roundfp(dim.y) + "";
					f = Std.parseFloat(ui.textInput(h, "Y"));
					if (h.changed) dim.y = f;

					h = Id.handle();
					h.text = roundfp(dim.z) + "";
					f = Std.parseFloat(ui.textInput(h, "Z"));
					if (h.changed) dim.z = f;

					Context.raw.selectedObject.transform.dirty = true;

					if (Context.raw.selectedObject.name == "Scene") {
						var p = iron.Scene.active.world.probe;
						p.raw.strength = ui.slider(Id.handle({value: p.raw.strength}), "Environment", 0.0, 5.0, true);
					}
					else if (Std.isOfType(Context.raw.selectedObject, iron.object.LightObject)) {
						var light = cast(Context.raw.selectedObject, iron.object.LightObject);
						var lightHandle = Id.handle();
						lightHandle.value = light.data.raw.strength / 10;
						light.data.raw.strength = ui.slider(lightHandle, "Strength", 0.0, 5.0, true) * 10;
					}
					else if (Std.isOfType(Context.raw.selectedObject, iron.object.CameraObject)) {
						var cam = cast(Context.raw.selectedObject, iron.object.CameraObject);
						var fovHandle = Id.handle();
						fovHandle.value = Std.int(cam.data.raw.fov * 100) / 100;
						cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
						if (fovHandle.changed) {
							cam.buildProjection();
						}
					}
				}

				ui.unindent();
			}
		}
	}
}
