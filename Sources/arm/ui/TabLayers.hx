package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Time;
import iron.system.Input;
import arm.data.LayerSlot;
import arm.node.MaterialParser;
import arm.util.UVUtil;

class TabLayers {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, "Layers")) {
			ui.row([1 / 4, 1 / 4, 1 / 2]);
			if (ui.button("New")) {

				// UIMenu.draw(function(ui:Zui) {
				// 	ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 5, ui.t.SEPARATOR_COL);
				// 	ui.text("New", Right, ui.t.HIGHLIGHT_COL);
				// 	if (ui.button("Paint Layer", Left)) {}
				// 	if (ui.button("Fill Layer", Left)) {}
				// 	if (ui.button("Black Mask", Left)) {}
				// 	if (ui.button("White Mask", Left)) {}
				// });

				Layers.newLayer();
				History.newLayer();
			}
			if (ui.button("2D View")) UITrait.inst.show2DView();
			else if (ui.isHovered) ui.tooltip("Show 2D View (" + Config.keymap.toggle_node_editor + ")");

			var ar = ["All"];
			for (p in Project.paintObjects) ar.push(p.name);
			var filterHandle = Id.handle();
			UITrait.inst.layerFilter = ui.combo(filterHandle, ar, "Filter");
			if (filterHandle.changed) {
				for (p in Project.paintObjects) {
					p.visible = UITrait.inst.layerFilter == 0 || p.name == ar[UITrait.inst.layerFilter];
					Layers.setObjectMask();
				}
				Context.ddirty = 2;
			}

			function drawList(l: LayerSlot, i: Int) {

				if (UITrait.inst.layerFilter > 0 &&
					l.objectMask > 0 &&
					l.objectMask != UITrait.inst.layerFilter) {
					return;
				}

				var h = Id.handle().nest(l.id, {selected: l.visible});
				var layerPanel = h.nest(0, {selected: false});
				var off = ui.t.ELEMENT_OFFSET;
				var step = ui.t.ELEMENT_H;
				var checkw = (ui._windowW / 100 * 8) / ui.SCALE();

				if (layerPanel.selected) {
					var mult = l.material_mask != null ? 2 : 1;
					var ph = (step + off) * mult;
					ui.fill(checkw, step * 2, (ui._windowW / ui.SCALE() - 2) - checkw, ph, ui.t.SEPARATOR_COL);
				}

				if (Context.layer == l) {
					if (Context.layerIsMask) {
						ui.rect((ui._windowW / 100 * 24) / ui.SCALE() + Std.int(1 * ui.SCALE()), 0, step * 2, step * 2, ui.t.HIGHLIGHT_COL, 2);
					}
					else {
						ui.fill(checkw, 0, (ui._windowW / ui.SCALE() - 2) - checkw, step * 2, ui.t.HIGHLIGHT_COL);
					}
				}

				if (l.texpaint_mask != null) {
					ui.row([8 / 100, 16 / 100, 16 / 100, 20 / 100, 30 / 100, 10 / 100]);
				}
				else {
					ui.row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
				}

				var center = (step / 2) * ui.SCALE();
				ui._y += center;
				var icons = Res.get("icons.k");
				var r = Res.tile18(icons, l.visible ? 0 : 1, 0);
				ui._x += 2;
				ui._y += 3;
				if (ui.image(icons, ui.t.ACCENT_SELECT_COL, null, r.x, r.y, r.w, r.h) == Released) {
					l.visible = !l.visible;
					MaterialParser.parseMeshMaterial();
				}
				ui._x -= 2;
				ui._y -= 3;
				ui._y -= center;

				var contextMenu = false;

				#if (kha_opengl || kha_webgl)
				ui.imageInvertY = l.material_mask != null;
				#end

				var uix = ui._x;
				var uiy = ui._y;
				ui._x += 2;
				ui._y += 3;
				var state = ui.image(l.material_mask == null ? l.texpaint_preview : l.material_mask.imageIcon, 0xffffffff, (ui.ELEMENT_H() - 3) * 2);
				ui._x -= 2;
				ui._y -= 3;

				#if (kha_opengl || kha_webgl)
				ui.imageInvertY = false;
				#end

				if (ui.isHovered) {
					ui.tooltipImage(l.texpaint_preview);
				}
				if (ui.isHovered && ui.inputReleasedR) {
					contextMenu = true;
				}
				if (state == State.Started) {
					Context.setLayer(l);
					if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.show2DView();
					UITrait.inst.selectTime = Time.time();
					var mouse = Input.getMouse();
					App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
					App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
					App.dragLayer = Context.layer;
				}

				if (l.texpaint_mask != null) {
					var uix = ui._x;
					var uiy = ui._y;
					ui._x += Std.int(4 * ui.SCALE());
					ui._y += 3;
					var state = ui.image(l.texpaint_mask_preview, 0xffffffff, (ui.ELEMENT_H() - 3) * 2);
					ui._x -= Std.int(4 * ui.SCALE());
					ui._y -= 3;
					if (ui.isHovered) {
						ui.tooltipImage(l.texpaint_mask_preview);
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui: Zui) {
							ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 3, ui.t.SEPARATOR_COL);
							ui.text(l.name + " Mask", Right, ui.t.HIGHLIGHT_COL);
							if (ui.button("Delete", Left)) {
								Context.setLayer(l);
								History.deleteMask();
								l.deleteMask();
								Context.setLayer(l);
							}
							if (ui.button("Apply", Left)) {
								function makeApply(g: kha.graphics4.Graphics) {
									g.end();
									Context.setLayer(l);
									History.applyMask();
									l.applyMask();
									MaterialParser.parseMeshMaterial();
									g.begin();
									iron.App.removeRender(makeApply);
								}
								iron.App.notifyOnRender(makeApply);
							}
						});
					}
					if (state == State.Started) {
						Context.setLayer(l, true);
						if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.show2DView();
						UITrait.inst.selectTime = Time.time();
						var mouse = Input.getMouse();
						App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						App.dragLayer = Context.layer;
					}
				}

				ui._y += center;
				var state = ui.text(l.name);
				ui._y -= center;

				if (state == State.Started) {
					Context.setLayer(l);
					if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.show2DView();
					UITrait.inst.selectTime = Time.time();
				}

				if (ui.isHovered && ui.inputReleasedR) {
					contextMenu = true;
				}

				if (contextMenu) {
					UIMenu.draw(function(ui: Zui) {
						var add = l.material_mask != null ? 1 : 0;
						if (l == Project.layers[0]) {
							ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * (11 + add), ui.t.SEPARATOR_COL);
						}
						else {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * (18 + add), ui.t.SEPARATOR_COL);
						}
						ui.text(l.name, Right, ui.t.HIGHLIGHT_COL);

						if (ui.button("Export", Left)) BoxExport.showTextures();

						if (l.material_mask == null && ui.button("To Fill Layer", Left)) {
							function makeFill(g: kha.graphics4.Graphics) {
								g.end();
								History.toFillLayer();
								l.toFillLayer();
								g.begin();
								iron.App.removeRender(makeFill);
							}
							iron.App.notifyOnRender(makeFill);
						}
						if (l.material_mask != null && ui.button("To Paint Layer", Left)) {
							function makePaint(g: kha.graphics4.Graphics) {
								g.end();
								History.toPaintLayer();
								l.toPaintLayer();
								g.begin();
								iron.App.removeRender(makePaint);
							}
							iron.App.notifyOnRender(makePaint);
						}

						if (l == Project.layers[0]) {
						}
						else {
							if (ui.button("Delete", Left)) {
								Context.layer = l;
								History.deleteLayer();
								l.delete();
							}
							if (ui.button("Move Up", Left)) {
								if (i < Project.layers.length - 1) {
									Context.setLayer(l);
									History.orderLayers(i + 1);
									var target = Project.layers[i + 1];
									Project.layers[i + 1] = Project.layers[i];
									Project.layers[i] = target;
									UITrait.inst.hwnd.redraws = 2;
								}
							}
							if (ui.button("Move Down", Left)) {
								if (i > 1) {
									Context.setLayer(l);
									History.orderLayers(i - 1);
									var target = Project.layers[i - 1];
									Project.layers[i - 1] = Project.layers[i];
									Project.layers[i] = target;
									UITrait.inst.hwnd.redraws = 2;
								}
							}
							if (ui.button("Merge Down", Left)) {
								Context.setLayer(l);
								iron.App.notifyOnRender(History.mergeLayers);
								iron.App.notifyOnRender(Layers.mergeSelectedLayer);
							}
							if (ui.button("Duplicate", Left)) {
								Context.setLayer(l);
								History.duplicateLayer();
								function makeDupli(g: kha.graphics4.Graphics) {
									g.end();
									l = l.duplicate();
									Context.setLayer(l);
									g.begin();
									iron.App.removeRender(makeDupli);
								}
								iron.App.notifyOnRender(makeDupli);
							}
							if (ui.button("Black Mask", Left)) {
								l.createMask(0x00000000);
								Context.setLayer(l, true);
								Context.layerPreviewDirty = true;
								History.newMask();
							}
							if (ui.button("White Mask", Left)) {
								l.createMask(0xffffffff);
								Context.setLayer(l, true);
								Context.layerPreviewDirty = true;
								History.newMask();
							}
						}
						if (l.material_mask != null) {
							if (ui.button("Select Material", Left)) {
								Context.setMaterial(l.material_mask);
							}
						}

						var baseHandle = Id.handle().nest(l.id, {selected: l.paintBase});
						var norHandle = Id.handle().nest(l.id, {selected: l.paintNor});
						var occHandle = Id.handle().nest(l.id, {selected: l.paintOcc});
						var roughHandle = Id.handle().nest(l.id, {selected: l.paintRough});
						var metHandle = Id.handle().nest(l.id, {selected: l.paintMet});
						var heightHandle = Id.handle().nest(l.id, {selected: l.paintHeight});
						var emisHandle = Id.handle().nest(l.id, {selected: l.paintEmis});
						var subsHandle = Id.handle().nest(l.id, {selected: l.paintSubs});
						l.paintBase = ui.check(baseHandle, "Base Color");
						l.paintNor = ui.check(norHandle, "Normal");
						l.paintOcc = ui.check(occHandle, "Occlusion");
						l.paintRough = ui.check(roughHandle, "Roughness");
						l.paintMet = ui.check(metHandle, "Metallic");
						l.paintHeight = ui.check(heightHandle, "Height");
						l.paintEmis = ui.check(emisHandle, "Emission");
						l.paintSubs = ui.check(subsHandle, "Subsurface");
						if (baseHandle.changed ||
							norHandle.changed ||
							occHandle.changed ||
							roughHandle.changed ||
							metHandle.changed ||
							heightHandle.changed ||
							emisHandle.changed ||
							subsHandle.changed) {
							MaterialParser.parseMeshMaterial();
							UIMenu.keepOpen = true;
						}
					});
				}

				if (i == 0) {
					@:privateAccess ui.endElement();
				}
				else {
					var blendingHandle = Id.handle().nest(l.id);
					blendingHandle.position = l.blending;
					ui.combo(blendingHandle, ["Mix", "Darken", "Multiply", "Burn", "Lighten", "Screen", "Dodge", "Add", "Overlay", "Soft Light", "Linear Light", "Difference", "Subtract", "Divide", "Hue", "Saturation", "Color", "Value"], "Blending");
					if (blendingHandle.changed) {
						Context.setLayer(l);
						History.layerBlending();
						l.blending = blendingHandle.position;
						MaterialParser.parseMeshMaterial();
					}
				}

				ui._y += center;
				var showPanel = ui.panel(layerPanel, "", true);
				ui._y -= center;

				if (i == 0) {
					ui._y -= ui.t.ELEMENT_OFFSET;
					@:privateAccess ui.endElement();
				}
				else {
					ui._y -= ui.t.ELEMENT_OFFSET;
					ui.row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
					@:privateAccess ui.endElement();
					@:privateAccess ui.endElement();
					@:privateAccess ui.endElement();

					var ar = ["Shared"];
					for (p in Project.paintObjects) ar.push(p.name);
					var h = Id.handle().nest(l.id);
					h.position = l.objectMask;
					l.objectMask = ui.combo(h, ar, "Object");
					if (h.changed) {
						Context.setLayer(l);
						if (l.material_mask != null) { // Fill layer
							iron.App.notifyOnRender(l.clear);
							iron.App.notifyOnRender(function(_){
								Layers.updateFillLayers(4);
							});
						}
						else {
							MaterialParser.parseMeshMaterial();
							Layers.setObjectMask();
						}
					}
					@:privateAccess ui.endElement();
				}
				ui._y -= ui.t.ELEMENT_OFFSET;

				if (showPanel) {
					ui.row([8 / 100, 92 / 100 / 3, 92 / 100 / 3, 92 / 100 / 3]);
					@:privateAccess ui.endElement();
					ui._x += 1;
					ui._y += 2;

					var opacHandle = Id.handle().nest(l.id);

					opacHandle.value = l.maskOpacity;
					ui.slider(opacHandle, "Opacity", 0.0, 1.0, true);
					if (opacHandle.changed) {
						Context.setLayer(l);
						if (ui.inputStarted) History.layerOpacity();
						l.maskOpacity = opacHandle.value;
						MaterialParser.parseMeshMaterial();
					}

					ui.combo(App.resHandle, ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"], "Res", true);
					if (App.resHandle.changed) {
						iron.App.notifyOnRender(Layers.resizeLayers);
						UVUtil.uvmap = null;
						UVUtil.uvmapCached = false;
						UVUtil.trianglemap = null;
						UVUtil.trianglemapCached = false;
						#if kha_direct3d12
						arm.render.RenderPathRaytrace.ready = false;
						#end
					}
					ui.combo(App.bitsHandle, ["8bit", "16bit", "32bit"], "Color", true);
					if (App.bitsHandle.changed) {
						iron.App.notifyOnRender(Layers.setLayerBits);
					}

					if (l.material_mask != null) {
						ui.row([8 / 100, 92 / 100 / 3, 92 / 100 / 3, 92 / 100 / 3]);
						@:privateAccess ui.endElement();

						var uvScaleHandle = Id.handle().nest(l.id, {value: l.uvScale});
						l.uvScale = ui.slider(uvScaleHandle, "UV Scale", 0.0, 5.0, true);
						if (uvScaleHandle.changed) {
							Context.setMaterial(l.material_mask);
							Context.setLayer(l);
							Layers.updateFillLayers();
						}

						var uvRotHandle = Id.handle().nest(l.id, {value: l.uvRot});
						l.uvRot = ui.slider(uvRotHandle, "UV Rotate", 0.0, 360, true, 1);
						if (uvRotHandle.changed) {
							Context.setMaterial(l.material_mask);
							Context.setLayer(l);
							MaterialParser.parsePaintMaterial();
							Layers.updateFillLayers();
						}

						var uvTypeHandle = Id.handle().nest(l.id, {position: l.uvType});
						l.uvType = ui.combo(uvTypeHandle, ["UV Map", "Triplanar"], "TexCoord");
						if (uvTypeHandle.changed) {
							Context.setMaterial(l.material_mask);
							Context.setLayer(l);
							MaterialParser.parsePaintMaterial();
							Layers.updateFillLayers();
						}
					}
				}
			}

			for (i in 0...Project.layers.length) {
				if (i >= Project.layers.length) break; // Layer was deleted
				var j = Project.layers.length - 1 - i;
				var l = Project.layers[j];
				drawList(l, j);
			}
		}
	}
}
