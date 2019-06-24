package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Time;
import arm.data.LayerSlot;
import arm.nodes.MaterialParser;

class TabLayers {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, "Layers")) {
			ui.row([1/4,1/4,1/2]);
			if (ui.button("New")) {
				Layers.newLayer();
				History.newLayer();
			}
			if (ui.button("2D View")) UITrait.inst.show2DView();
			else if (ui.isHovered) ui.tooltip("Show 2D View (SHIFT+TAB)");
			
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

			function drawList(l:LayerSlot, i:Int) {

				if (UITrait.inst.layerFilter > 0 &&
					l.objectMask > 0 &&
					l.objectMask != UITrait.inst.layerFilter) return;

				var h = Id.handle().nest(l.id, {selected: l.visible});
				var layerPanel = h.nest(0, {selected: false});
				var off = ui.t.ELEMENT_OFFSET;
				var step = ui.t.ELEMENT_H;
				var checkw = (ui._windowW / 100 * 8) / ui.SCALE;

				if (layerPanel.selected) {
					ui.fill(checkw, step * 2, (ui._windowW / ui.SCALE - 2) - checkw, step + off, ui.t.SEPARATOR_COL);
				}

				if (Context.layer == l) {
					if (Context.layerIsMask) {
						ui.rect(ui._windowW / 100 * 24 - 2, 0, ui._windowW / 100 * 16, step * 2, ui.t.HIGHLIGHT_COL, 2);
					}
					else {
						ui.fill(checkw, 0, (ui._windowW / ui.SCALE - 2) - checkw, step * 2, ui.t.HIGHLIGHT_COL);
					}
				}

				if (l.texpaint_mask != null) {
					ui.row([8/100, 16/100, 16/100, 20/100, 30/100, 10/100]);
				}
				else {
					ui.row([8/100, 16/100, 36/100, 30/100, 10/100]);
				}
				
				var center = (step / 2) * ui.SCALE;
				ui._y += center;
				l.visible = ui.check(h, "");
				if (h.changed) {
					MaterialParser.parseMeshMaterial();
				}
				ui._y -= center;

				var contextMenu = false;

				#if (kha_opengl || kha_webgl)
				ui.imageInvertY = l.material_mask != null;
				#end

				ui._y += 3;
				var state = ui.image(l.material_mask == null ? l.texpaint_preview : l.material_mask.imageIcon);
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
				if (ui.isReleased) {
					Context.setLayer(l);
				}
				if (state == State.Started) {
					if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.show2DView();
					UITrait.inst.selectTime = Time.time();
				}

				if (l.texpaint_mask != null) {
					ui._y += 3;
					var state = ui.image(l.texpaint_mask_preview);
					ui._y -= 3;
					if (ui.isHovered) {
						ui.tooltipImage(l.texpaint_mask_preview);
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui:Zui) {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 3, ui.t.SEPARATOR_COL);
							ui.text(l.name + " Mask", Right);
							if (ui.button("Delete", Left)) {
								l.deleteMask();
								Context.setLayer(l);
								History.deleteMask();
							}
							if (ui.button("Apply", Left)) {
								Context.setLayer(l);
								l.applyMask();
								Context.setLayer(l); // Parse mesh material
								History.applyMask();
							}
						});
					}
					if (ui.isReleased) {
						Context.setLayer(l, true);
					}
					if (state == State.Started) {
						if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.show2DView();
						UITrait.inst.selectTime = Time.time();
					}
				}

				ui._y += center;
				ui.text(l.name);
				ui._y -= center;

				if (ui.isReleased) {
					Context.setLayer(l);
				}

				if (ui.isHovered && ui.inputReleasedR) {
					contextMenu = true;
				}

				if (contextMenu) {
					UIMenu.draw(function(ui:Zui) {
						if (l == Project.layers[0]) {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 10, ui.t.SEPARATOR_COL);
							ui.text(l.name, Right);
						}
						else {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 17, ui.t.SEPARATOR_COL);
							ui.text(l.name, Right);
						}

						if (l.material_mask == null && ui.button("To Fill Layer", Left)) {
							Layers.toFillLayer(l);
						}
						if (l.material_mask != null && ui.button("To Paint Layer", Left)) {
							Layers.toPaintLayer(l);
						}

						if (l == Project.layers[0]) {
						}
						else {
							if (ui.button("Delete", Left)) {
								Context.layer = l;
								History.deleteLayer();
								Layers.deleteSelectedLayer();
							}
							if (ui.button("Move Up", Left)) {
								if (i < Project.layers.length - 1) {
									Context.setLayer(l);
									var target = Project.layers[i + 1];
									Project.layers[i + 1] = Project.layers[i];
									Project.layers[i] = target;
									UITrait.inst.hwnd.redraws = 2;
									History.orderLayers(i);
								}
							}
							if (ui.button("Move Down", Left)) {
								if (i > 1) {
									Context.setLayer(l);
									var target = Project.layers[i - 1];
									Project.layers[i - 1] = Project.layers[i];
									Project.layers[i] = target;
									UITrait.inst.hwnd.redraws = 2;
									History.orderLayers(i);
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
								function makeDupli(g:kha.graphics4.Graphics) {
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
							UIMenu.propChanged = true;
						}
					});
				}

				if (i == 0) {
					@:privateAccess ui.endElement();
				}
				else {
					var blend = ui.combo(Id.handle(), ["Add"], "Blending");
				}

				ui._y += center;
				var showPanel = ui.panel(layerPanel, "", 0, true);
				ui._y -= center;

				if (i == 0) {
					ui._y -= ui.t.ELEMENT_OFFSET;
					@:privateAccess ui.endElement();
				}
				else {
					ui._y -= ui.t.ELEMENT_OFFSET;
					ui.row([8/100, 16/100, 36/100, 30/100, 10/100]);
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
						Layers.updateFillLayers(4);
					}
					@:privateAccess ui.endElement();
				}
				ui._y -= ui.t.ELEMENT_OFFSET;

				if (showPanel) {
					ui.row([8/100,92/100]);
					@:privateAccess ui.endElement();
					var opacHandle = Id.handle().nest(l.id, {value: l.maskOpacity});
					l.maskOpacity = ui.slider(opacHandle, "Opacity", 0.0, 1.0, true);
					if (opacHandle.changed) {
						MaterialParser.parseMeshMaterial();
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
