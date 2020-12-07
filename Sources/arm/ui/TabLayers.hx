package arm.ui;

import zui.Zui;
import zui.Id;
import zui.Nodes;
import iron.system.Time;
import iron.system.Input;
import arm.data.LayerSlot;
import arm.node.MakeMaterial;
import arm.util.UVUtil;
import arm.sys.Path;
import arm.Enums;

class TabLayers {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab0, tr("Layers"))) {

			ui.beginSticky();

			ui.row([1 / 4, 1 / 4, 1 / 2]);
			if (ui.button(tr("New"))) {

				// UIMenu.draw(function(ui:Zui) {
				// 	ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.SEPARATOR_COL);
				// 	ui.text("New", Right, ui.t.HIGHLIGHT_COL);
				// 	if (ui.button("Paint Layer", Left)) {}
				// 	if (ui.button("Fill Layer", Left, "Material 1")) {}
				// 	if (ui.button("Black Mask", Left, "Layer 1")) {}
				// 	if (ui.button("White Mask", Left, "Layer 1")) {}
				// 	if (ui.button("Folder", Left)) {}
				// }, 6);

				Layers.newLayer();
				History.newLayer();
			}
			if (ui.button(tr("2D View"))) UISidebar.inst.show2DView(View2DLayer);
			else if (ui.isHovered) ui.tooltip(tr("Show 2D View") + ' (${Config.keymap.toggle_node_editor})');

			var ar = [tr("All")];
			for (p in Project.paintObjects) ar.push(p.name);
			var filterHandle = Id.handle();
			filterHandle.position = Context.layerFilter;
			Context.layerFilter = ui.combo(filterHandle, ar, tr("Filter"), false, Left, 16);
			if (filterHandle.changed) {
				for (p in Project.paintObjects) {
					p.visible = Context.layerFilter == 0 || p.name == ar[Context.layerFilter];
					Layers.setObjectMask();
				}
				UVUtil.uvmapCached = false;
				Context.ddirty = 2;
				#if (kha_direct3d12 || kha_vulkan)
				arm.render.RenderPathRaytrace.ready = false;
				#end
			}

			ui.endSticky();

			function drawList(l: LayerSlot, i: Int) {

				if (Context.layerFilter > 0 &&
					l.objectMask > 0 &&
					l.objectMask != Context.layerFilter) {
					return;
				}

				if (l.parent != null && !l.parent.show_panel) { // Group closed
					return;
				}

				var off = ui.t.ELEMENT_OFFSET;
				var step = ui.t.ELEMENT_H;
				var checkw = (ui._windowW / 100 * 8) / ui.SCALE();

				if (l.show_panel && l.getChildren() == null) {
					var mult = l.fill_layer != null ? 2 : 1;
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

				// Highlight drag destination
				var mouse = Input.getMouse();
				var mx = mouse.x;
				var my = mouse.y;
				var inLayers = mx > UISidebar.inst.tabx && my < Config.raw.layout[LayoutSidebarH0];
				if (App.isDragging && App.dragLayer != null && inLayers) {
					if (my > ui._y - step && my < ui._y + step) {
						ui.fill(checkw, 0, (ui._windowW / ui.SCALE() - 2) - checkw, 2 * ui.SCALE(), ui.t.HIGHLIGHT_COL);
						Context.dragDestination = Project.layers.indexOf(App.dragLayer) < i ? i : i + 1;
					}
					else if (i == 0 && my > ui._y + step) {
						ui.fill(checkw, step * 2, (ui._windowW / ui.SCALE() - 2) - checkw, 2 * ui.SCALE(), ui.t.HIGHLIGHT_COL);
						Context.dragDestination = 0;
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
				var col = ui.t.ACCENT_SELECT_COL;
				var groupVisible = l.parent == null || l.parent.visible;
				if (!groupVisible) col -= 0x99000000;

				if (ui.image(icons, col, null, r.x, r.y, r.w, r.h) == Released) {
					l.visible = !l.visible;
					MakeMaterial.parseMeshMaterial();
				}
				ui._x -= 2;
				ui._y -= 3;
				ui._y -= center;

				var contextMenu = false;

				#if kha_opengl
				ui.imageInvertY = l.fill_layer != null;
				#end

				var uix = ui._x;
				var uiy = ui._y;
				ui._x += 2;
				ui._y += 3;
				if (l.parent != null) ui._x += 10 * ui.SCALE();

				var state = State.Idle;
				var iconH = (ui.ELEMENT_H() - 3) * 2;

				if (l.getChildren() == null) {
					var icon = l.fill_layer == null ? l.texpaint_preview : l.fill_layer.imageIcon;
					if (l.fill_layer == null) {
						// Checker
						var r = Res.tile50(icons, 4, 1);
						var _x = ui._x;
						var _y = ui._y;
						ui.curRatio--;
						ui.image(icons, 0xffffffff, iconH, r.x, r.y, r.w, r.h);
						ui._x = _x;
						ui._y = _y;
					}
					state = ui.image(icon, 0xffffffff, iconH);
				}
				else { // Group
					var folderClosed = Res.tile50(icons, 2, 1);
					var folderOpen = Res.tile50(icons, 8, 1);
					var folder = l.show_panel ? folderOpen : folderClosed;
					state = ui.image(icons, ui.t.LABEL_COL - 0x00202020, iconH, folder.x, folder.y, folder.w, folder.h);
				}

				ui._x -= 2;
				ui._y -= 3;

				#if kha_opengl
				ui.imageInvertY = false;
				#end

				if (ui.isHovered && l.texpaint_preview != null) {
					ui.tooltipImage(l.texpaint_preview);
				}
				if (ui.isHovered && ui.inputReleasedR) {
					contextMenu = true;
				}
				if (state == State.Started) {
					Context.setLayer(l);
					if (Time.time() - Context.selectTime < 0.25) UISidebar.inst.show2DView(View2DLayer);
					Context.selectTime = Time.time();
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

					// var r = Res.tile50(icons, 4, 1);
					// var _x = ui._x;
					// var _y = ui._y;
					// ui.curRatio--;
					// ui.image(icons, 0xffffffff, null, r.x, r.y, r.w, r.h);
					// ui._x = _x;
					// ui._y = _y;

					var state = State.Idle;
					if (l.fill_mask != null) {
						state = ui.image(l.fill_mask.imageIcon, 0xffffffff, (ui.ELEMENT_H() - 3) * 2);
					}
					else {
						ui.g.pipeline = UIView2D.pipe;
						#if kha_opengl
						ui.currentWindow.texture.g4.setPipeline(UIView2D.pipe);
						#end
						ui.currentWindow.texture.g4.setInt(UIView2D.channelLocation, 1);
						state = ui.image(l.texpaint_mask_preview, 0xffffffff, (ui.ELEMENT_H() - 3) * 2);
						ui.g.pipeline = null;
					}

					ui._x -= Std.int(4 * ui.SCALE());
					ui._y -= 3;
					if (ui.isHovered) {
						ui.tooltipImage(l.texpaint_mask_preview);
					}
					if (ui.isHovered && ui.inputReleasedR) {
						var add = l.fill_mask == null ? 2 : 0;
						UIMenu.draw(function(ui: Zui) {
							ui.text('${l.name} ' + tr("Mask"), Right, ui.t.HIGHLIGHT_COL);
							if (ui.button(tr("Export"), Left)) {
								UIFiles.show("png", true, function(path: String) {
									var f = UIFiles.filename;
									if (f == "") f = tr("untitled");
									if (!f.endsWith(".png")) f += ".png";
									var out = new haxe.io.BytesOutput();
									var writer = new arm.format.PngWriter(out);
									var data = arm.format.PngTools.buildGrey(l.texpaint_mask.width, l.texpaint_mask.height, l.texpaint_mask.getPixels());
									writer.write(data);
									Krom.fileSaveBytes(path + Path.sep + f, out.getBytes().getData());
								});
							}
							if (l.fill_mask == null && ui.button(tr("To Fill Mask"), Left)) {
								function _init() {
									History.toFillMask();
									l.toFillMask();
								}
								iron.App.notifyOnInit(_init);
							}
							if (l.fill_mask != null && ui.button(tr("To Paint Mask"), Left)) {
								function _init() {
									History.toPaintMask();
									l.toPaintMask();
								}
								iron.App.notifyOnInit(_init);
							}
							if (l.fill_mask != null && ui.button(tr("Select Material"), Left)) {
								Context.setMaterial(l.fill_mask);
							}
							if (ui.button(tr("Delete"), Left)) {
								Context.setLayer(l, true);
								History.deleteMask();
								l.deleteMask();
								Context.setLayer(l, false);
							}
							if (l.fill_mask == null && ui.button(tr("Clear to Black"), Left)) {
								function _init() {
									l.clearMask(0x00000000);
								}
								iron.App.notifyOnInit(_init);
							}
							if (l.fill_mask == null && ui.button(tr("Clear to White"), Left)) {
								function _init() {
									l.clearMask(0xffffffff);
								}
								iron.App.notifyOnInit(_init);
							}
							if (l.fill_mask == null && ui.button(tr("Invert"), Left)) {
								function _init() {
									l.invertMask();
								}
								iron.App.notifyOnInit(_init);
							}
							if (ui.button(tr("Apply"), Left)) {
								function _init() {
									Context.setLayer(l);
									History.applyMask();
									l.applyMask();
									MakeMaterial.parseMeshMaterial();
								}
								iron.App.notifyOnInit(_init);
							}
						}, 6 + add);
					}
					if (state == State.Started) {
						Context.setLayer(l, true);
						if (Time.time() - Context.selectTime < 0.25) UISidebar.inst.show2DView(View2DLayer);
						Context.selectTime = Time.time();
						var mouse = Input.getMouse();
						App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						App.dragLayer = Context.layer;
					}
				}

				ui._y += center;
				var state = ui.text(l.name);
				ui._y -= center;

				if (l.parent != null) ui._x -= 10 * ui.SCALE();

				if (state == State.Started) {
					Context.setLayer(l);
					if (Time.time() - Context.selectTime < 0.25) UISidebar.inst.show2DView(View2DLayer);
					Context.selectTime = Time.time();
					var mouse = Input.getMouse();
					App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
					App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
					App.dragLayer = Context.layer;
				}

				if (ui.isHovered && ui.inputReleasedR) {
					contextMenu = true;
				}

				if (contextMenu) {

					var add = l.fill_layer != null ? 1 : 0;
					var menuElements = l.getChildren() != null ? 6 : (19 + add);

					UIMenu.draw(function(ui: Zui) {
						ui.text(l.name, Right, ui.t.HIGHLIGHT_COL);

						if (ui.button(tr("Export"), Left)) {
							Context.layersExport = ExportSelected;
							BoxExport.showTextures();
						}

						if (l.getChildren() == null && l.fill_layer == null && ui.button(tr("To Fill Layer"), Left)) {
							function _init() {
								History.toFillLayer();
								l.toFillLayer();
							}
							iron.App.notifyOnInit(_init);
						}
						if (l.getChildren() == null && l.fill_layer != null && ui.button(tr("To Paint Layer"), Left)) {
							function _init() {
								History.toPaintLayer();
								l.toPaintLayer();
							}
							iron.App.notifyOnInit(_init);
						}

						if (l.getChildren() == null && ui.button(tr("To Group"), Left)) {
							if (l.parent == null) { // 1-level nesting only
								var pointers = initLayerMap();
								Context.setLayer(l);
								var group = Layers.newGroup();
								Project.layers.remove(group);
								Project.layers.insert(Project.layers.indexOf(l) + 1, group);
								group.show_panel = true;
								l.parent = group;
								Context.setLayer(l);
								// History.newGroup();
								for (m in Project.materials) remapLayerPointers(m.canvas.nodes, fillLayerMap(pointers));
							}
						}
						if (ui.button(tr("Delete"), Left)) {
							if (arm.Project.layers.length > 1) {
								var pointers = initLayerMap();
								Context.layer = l;
								if (l.getChildren() == null) {
									History.deleteLayer();
								}
								else {
									for (c in l.getChildren()) {
										Context.layer = c;
										History.deleteLayer();
										c.delete();
									}
								}
								l.delete();

								// Remove empty group
								if (l.parent != null && l.parent.getChildren() == null) {
									l.parent.delete();
								}
								Context.ddirty = 2;
								for (m in Project.materials) remapLayerPointers(m.canvas.nodes, fillLayerMap(pointers));
							}
						}
						if (ui.button(tr("Clear"), Left)) {
							Context.setLayer(l);
							function _init() {
								if (l.getChildren() == null) {
									History.clearLayer();
									l.clearLayer();
								}
								else {
									for (c in l.getChildren()) {
										Context.layer = c;
										History.clearLayer();
										c.clearLayer();
									}
									Context.layer = l;
								}
							}
							iron.App.notifyOnInit(_init);
						}
						if (l.getChildren() != null && ui.button(tr("Merge Group"), Left)) {
							function _init() {
								var children = l.getChildren();
								for (i in 0...children.length - 1) {
									Context.setLayer(children[children.length - 1 - i]);
									History.mergeLayers();
									Layers.mergeDown();
								}
								children[0].parent = null;
								children[0].name = l.name;
								l.delete();
							}
							iron.App.notifyOnInit(_init);
						}
						if (l.getChildren() == null && ui.button(tr("Merge Down"), Left)) {
							Context.setLayer(l);
							iron.App.notifyOnInit(History.mergeLayers);
							iron.App.notifyOnInit(Layers.mergeDown);
						}
						if (ui.button(tr("Duplicate"), Left)) {
							function _init() {
								if (l.getChildren() == null) {
									Context.setLayer(l);
									History.duplicateLayer();
									l = l.duplicate();
									Context.setLayer(l);
								}
								else {
									var group = Layers.newGroup();
									Project.layers.remove(group);
									Project.layers.insert(Project.layers.indexOf(l) + 1, group);
									// group.show_panel = true;
									for (c in l.getChildren()) {
										Context.setLayer(c);
										History.duplicateLayer();
										c = c.duplicate();
										c.parent = group;
										Project.layers.remove(c);
										Project.layers.insert(Project.layers.indexOf(group), c);
									}
									Context.setLayer(group);
								}
							}
							iron.App.notifyOnInit(_init);
						}
						if (l.getChildren() == null && ui.button(tr("Black Mask"), Left)) {
							l.createMask(0x00000000);
							Context.setLayer(l, true);
							Context.layerPreviewDirty = true;
							History.newMask();
						}
						if (l.getChildren() == null && ui.button(tr("White Mask"), Left)) {
							l.createMask(0xffffffff);
							Context.setLayer(l, true);
							Context.layerPreviewDirty = true;
							History.newMask();
						}

						if (l.fill_layer != null && ui.button(tr("Select Material"), Left)) {
							Context.setMaterial(l.fill_layer);
						}

						if (l.getChildren() == null) {
							var baseHandle = Id.handle().nest(l.id);
							var opacHandle = Id.handle().nest(l.id);
							var norHandle = Id.handle().nest(l.id);
							var occHandle = Id.handle().nest(l.id);
							var roughHandle = Id.handle().nest(l.id);
							var metHandle = Id.handle().nest(l.id);
							var heightHandle = Id.handle().nest(l.id);
							var emisHandle = Id.handle().nest(l.id);
							var subsHandle = Id.handle().nest(l.id);
							baseHandle.selected = l.paintBase;
							opacHandle.selected = l.paintOpac;
							norHandle.selected = l.paintNor;
							occHandle.selected = l.paintOcc;
							roughHandle.selected = l.paintRough;
							metHandle.selected = l.paintMet;
							heightHandle.selected = l.paintHeight;
							emisHandle.selected = l.paintEmis;
							subsHandle.selected = l.paintSubs;
							l.paintBase = ui.check(baseHandle, tr("Base Color"));
							l.paintOpac = ui.check(opacHandle, tr("Opacity"));
							l.paintNor = ui.check(norHandle, tr("Normal"));
							l.paintOcc = ui.check(occHandle, tr("Occlusion"));
							l.paintRough = ui.check(roughHandle, tr("Roughness"));
							l.paintMet = ui.check(metHandle, tr("Metallic"));
							l.paintHeight = ui.check(heightHandle, tr("Height"));
							l.paintEmis = ui.check(emisHandle, tr("Emission"));
							l.paintSubs = ui.check(subsHandle, tr("Subsurface"));
							if (baseHandle.changed ||
								opacHandle.changed ||
								norHandle.changed ||
								occHandle.changed ||
								roughHandle.changed ||
								metHandle.changed ||
								heightHandle.changed ||
								emisHandle.changed ||
								subsHandle.changed) {
								MakeMaterial.parseMeshMaterial();
								UIMenu.keepOpen = true;
							}
						}
					}, menuElements);
				}

				if (l.getChildren() != null) {
					@:privateAccess ui.endElement();
				}
				else {
					var blendingHandle = Id.handle().nest(l.id);
					blendingHandle.position = l.blending;
					ui.combo(blendingHandle, [
						tr("Mix"),
						tr("Darken"),
						tr("Multiply"),
						tr("Burn"),
						tr("Lighten"),
						tr("Screen"),
						tr("Dodge"),
						tr("Add"),
						tr("Overlay"),
						tr("Soft Light"),
						tr("Linear Light"),
						tr("Difference"),
						tr("Subtract"),
						tr("Divide"),
						tr("Hue"),
						tr("Saturation"),
						tr("Color"),
						tr("Value"),
					], tr("Blending"));
					if (blendingHandle.changed) {
						Context.setLayer(l);
						History.layerBlending();
						l.blending = blendingHandle.position;
						MakeMaterial.parseMeshMaterial();
					}
				}

				ui._y += center;
				var layerPanel = Id.handle().nest(l.id);
				layerPanel.selected = l.show_panel;
				l.show_panel = ui.panel(layerPanel, "", true, false, false);
				ui._y -= center;

				if (l.getChildren() != null) {
					ui._y -= ui.t.ELEMENT_OFFSET;
					@:privateAccess ui.endElement();
				}
				else {
					ui._y -= ui.t.ELEMENT_OFFSET;
					ui.row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
					@:privateAccess ui.endElement();
					@:privateAccess ui.endElement();
					@:privateAccess ui.endElement();

					var ar = [tr("Shared")];
					for (p in Project.paintObjects) ar.push(p.name);
					var objectHandle = Id.handle().nest(l.id);
					objectHandle.position = l.objectMask == null ? 0 : l.objectMask; // TODO: deprecated
					l.objectMask = ui.combo(objectHandle, ar, tr("Object"), false, Left, 16);
					if (objectHandle.changed) {
						Context.setLayer(l);
						MakeMaterial.parseMeshMaterial();
						if (l.fill_layer != null) { // Fill layer
							iron.App.notifyOnInit(l.clear);
							function _init() {
								Layers.updateFillLayers();
							}
							iron.App.notifyOnInit(_init);
						}
						else {
							Layers.setObjectMask();
						}
					}
					@:privateAccess ui.endElement();
				}
				ui._y -= ui.t.ELEMENT_OFFSET;

				if (l.show_panel && l.getChildren() == null) {
					ui.row([8 / 100, 92 / 100 / 3, 92 / 100 / 3, 92 / 100 / 3]);
					@:privateAccess ui.endElement();
					ui._x += 1;
					ui._y += 2;

					var layerOpacHandle = Id.handle().nest(l.id);
					layerOpacHandle.value = l.maskOpacity;
					ui.slider(layerOpacHandle, tr("Opacity"), 0.0, 1.0, true);
					if (layerOpacHandle.changed) {
						Context.setLayer(l);
						if (ui.inputStarted) History.layerOpacity();
						l.maskOpacity = layerOpacHandle.value;
						MakeMaterial.parseMeshMaterial();
					}

					ui.combo(App.resHandle, ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"], tr("Res"), true);
					if (App.resHandle.changed) {
						iron.App.notifyOnInit(Layers.resizeLayers);
						UVUtil.uvmap = null;
						UVUtil.uvmapCached = false;
						UVUtil.trianglemap = null;
						UVUtil.trianglemapCached = false;
						UVUtil.dilatemapCached = false;
						#if (kha_direct3d12 || kha_vulkan)
						arm.render.RenderPathRaytrace.ready = false;
						#end
					}
					ui.combo(App.bitsHandle, ["8bit", "16bit", "32bit"], tr("Color"), true);
					if (App.bitsHandle.changed) {
						iron.App.notifyOnInit(Layers.setLayerBits);
					}

					if (l.fill_layer != null) {
						ui.row([8 / 100, 92 / 100 / 3, 92 / 100 / 3, 92 / 100 / 3]);
						@:privateAccess ui.endElement();

						var scaleHandle = Id.handle().nest(l.id);
						scaleHandle.value = l.scale;
						l.scale = ui.slider(scaleHandle, tr("UV Scale"), 0.0, 5.0, true);
						if (scaleHandle.changed) {
							Context.setMaterial(l.fill_layer);
							Context.setLayer(l);
							function _init() {
								Layers.updateFillLayers();
							}
							iron.App.notifyOnInit(_init);
						}

						var angleHandle = Id.handle().nest(l.id);
						angleHandle.value = l.angle;
						l.angle = ui.slider(angleHandle, tr("Angle"), 0.0, 360, true, 1);
						if (angleHandle.changed) {
							Context.setMaterial(l.fill_layer);
							Context.setLayer(l);
							MakeMaterial.parsePaintMaterial();
							function _init() {
								Layers.updateFillLayers();
							}
							iron.App.notifyOnInit(_init);
						}

						var uvTypeHandle = Id.handle().nest(l.id);
						uvTypeHandle.position = l.uvType;
						l.uvType = ui.combo(uvTypeHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], tr("TexCoord"));
						if (uvTypeHandle.changed) {
							Context.setMaterial(l.fill_layer);
							Context.setLayer(l);
							MakeMaterial.parsePaintMaterial();
							function _init() {
								Layers.updateFillLayers();
							}
							iron.App.notifyOnInit(_init);
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

	public static function remapLayerPointers(nodes: Array<TNode>, pointerMap: Map<Int, Int>) {
		for (n in nodes) {
			if (n.type == "LAYER" || n.type == "LAYER_MASK") {
				var i = n.buttons[0].default_value;
				if (pointerMap.exists(i)) {
					n.buttons[0].default_value = pointerMap.get(i);
				}
			}
		}
	}

	public static function initLayerMap(): Map<LayerSlot, Int> {
		var res: Map<LayerSlot, Int> = [];
		for (i in 0...Project.layers.length) res.set(Project.layers[i], i);
		return res;
	}

	public static function fillLayerMap(map: Map<LayerSlot, Int>): Map<Int, Int> {
		var res: Map<Int, Int> = [];
		for (l in map.keys()) res.set(map.get(l), Project.layers.indexOf(l) > -1 ? Project.layers.indexOf(l) : 9999);
		return res;
	}
}
