package arm;

import zui.Nodes;
import arm.sys.Path;
import arm.ui.UIFiles;
import arm.ui.UINodes;
#if (is_paint || is_sculpt)
import arm.ui.UIBase;
import arm.ui.UIView2D;
import arm.ui.UIToolbar;
import arm.data.LayerSlot;
import arm.data.MaterialSlot;
import arm.shader.MakeMaterial;
import arm.Project;
#end

class History {

	public static var steps: Array<TStep>;
	public static var undoI = 0; // Undo layer
	public static var undos = 0; // Undos available
	public static var redos = 0; // Redos available
	#if (is_paint || is_sculpt)
	public static var pushUndo = false; // Store undo on next paint
	public static var undoLayers: Array<LayerSlot> = null;
	#end
	#if is_sculpt
	public static var pushUndo2 = false;
	#end

	public static function undo() {
		if (undos > 0) {
			var active = steps.length - 1 - redos;
			var step = steps[active];

			if (step.name == tr("Edit Nodes")) {
				swapCanvas(step);
			}

			#if (is_paint || is_sculpt)
			else if (step.name == tr("New Layer") || step.name == tr("New Black Mask") || step.name == tr("New White Mask") || step.name == tr("New Fill Mask")) {
				Context.raw.layer = Project.layers[step.layer];
				Context.raw.layer.delete();
				Context.raw.layer = Project.layers[step.layer > 0 ? step.layer - 1 : 0];
			}
			else if (step.name == tr("New Group")) {
				Context.raw.layer = Project.layers[step.layer];
				// The layer below is the only layer in the group. Its layer masks are automatically unparented, too.
				Project.layers[step.layer - 1].parent = null;
				Context.raw.layer.delete();
				Context.raw.layer = Project.layers[step.layer > 0 ? step.layer - 1 : 0];
			}
			else if (step.name == tr("Delete Layer")) {
				var parent = step.layer_parent > 0 ? Project.layers[step.layer_parent - 1] : null;
				var l = new LayerSlot("", step.layer_type, parent);
				Project.layers.insert(step.layer, l);
				Context.setLayer(l);
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				l.swap(lay);
				l.maskOpacity = step.layer_opacity;
				l.blending = step.layer_blending;
				l.objectMask = step.layer_object;
				MakeMaterial.parseMeshMaterial();

				// Undo at least second time in order to avoid empty groups
				if (step.layer_type == LayerSlotType.SlotGroup) {
					App.notifyOnNextFrame(function() {
						// 1. Undo deleting group masks
						var n = 1;
						while (steps[active - n].layer_type == LayerSlotType.SlotMask) {
							undo();
							++n;
						}
						// 2. Undo a mask to have a non empty group
						undo();
					});
				}
			}
			else if (step.name == tr("Clear Layer")) {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.raw.layer.swap(lay);
				Context.raw.layerPreviewDirty = true;
			}
			else if (step.name == tr("Duplicate Layer")) {
				var children = Project.layers[step.layer].getRecursiveChildren();
				var position = step.layer + 1;
				if (children != null)
					position += children.length;

				Context.raw.layer = Project.layers[position];
				Context.raw.layer.delete();
			}
			else if (step.name == tr("Order Layers")) {
				var target = Project.layers[step.prev_order];
				Project.layers[step.prev_order] = Project.layers[step.layer];
				Project.layers[step.layer] = target;
			}
			else if (step.name == tr("Merge Layers")) {
				Context.raw.layer = Project.layers[step.layer];
				Context.raw.layer.delete();

				var parent = step.layer_parent > 0 ? Project.layers[step.layer_parent - 2] : null;
				var l = new LayerSlot("", step.layer_type, parent);
				Project.layers.insert(step.layer, l);
				Context.setLayer(l);

				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.raw.layer.swap(lay);

				var l = new LayerSlot("", step.layer_type, parent);
				Project.layers.insert(step.layer + 1, l);
				Context.setLayer(l);

				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.raw.layer.swap(lay);

				Context.raw.layer.maskOpacity = step.layer_opacity;
				Context.raw.layer.blending = step.layer_blending;
				Context.raw.layer.objectMask = step.layer_object;
				Context.raw.layersPreviewDirty = true;
				MakeMaterial.parseMeshMaterial();
			}
			else if (step.name == tr("Apply Mask")) {
				// First restore the layer(s)
				var maskPosition = step.layer;
				var currentLayer = null;
				// The layer at the old mask position is a mask, i.e. the layer had multiple masks before.
				if (Project.layers[maskPosition].isMask())
					currentLayer = Project.layers[maskPosition].parent;
				else if (Project.layers[maskPosition].isLayer() || Project.layers[maskPosition].isGroup())
					currentLayer = Project.layers[maskPosition];

				var layersToRestore = currentLayer.isGroup() ? currentLayer.getChildren() : [currentLayer];
				layersToRestore.reverse();

				for (layer in layersToRestore) {
					// Replace the current layer's content with the old one
					Context.raw.layer = layer;
					undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
					var oldLayer = undoLayers[undoI];
					Context.raw.layer.swap(oldLayer);
				}

				// Now restore the applied mask
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var mask = undoLayers[undoI];
				App.newMask(false, currentLayer, maskPosition);
				Context.raw.layer.swap(mask);
				Context.raw.layersPreviewDirty = true;
				Context.setLayer(Context.raw.layer);
			}
			else if (step.name == tr("Invert Mask")) {
				function _next() {
					Context.raw.layer = Project.layers[step.layer];
					Context.raw.layer.invertMask();
				}
				iron.App.notifyOnInit(_next);
			}
			else if (step.name == "Apply Filter") {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.setLayer(Project.layers[step.layer]);
				Context.raw.layer.swap(lay);
				App.newMask(false, Context.raw.layer);
				Context.raw.layer.swap(lay);
				Context.raw.layerPreviewDirty = true;
			}
			else if (step.name == tr("To Fill Layer") || step.name == tr("To Fill Mask")) {
				Context.raw.layer.toPaintLayer();
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.raw.layer.swap(lay);
			}
			else if (step.name == tr("To Paint Layer") || step.name == tr("To Paint Mask")) {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.raw.layer.swap(lay);
				Context.raw.layer.fill_layer = Project.materials[step.material];
			}
			else if (step.name == tr("Layer Opacity")) {
				Context.setLayer(Project.layers[step.layer]);
				var t = Context.raw.layer.maskOpacity;
				Context.raw.layer.maskOpacity = step.layer_opacity;
				step.layer_opacity = t;
				MakeMaterial.parseMeshMaterial();
			}
			else if (step.name == tr("Layer Blending")) {
				Context.setLayer(Project.layers[step.layer]);
				var t = Context.raw.layer.blending;
				Context.raw.layer.blending = step.layer_blending;
				step.layer_blending = t;
				MakeMaterial.parseMeshMaterial();
			}
			else if (step.name == tr("Delete Node Group")) {
				Project.materialGroups.insert(step.canvas_group, { canvas: null, nodes: new Nodes() });
				swapCanvas(step);
			}
			else if (step.name == tr("New Material")) {
				Context.raw.material = Project.materials[step.material];
				step.canvas = Context.raw.material.canvas;
				Context.raw.material.delete();
			}
			else if (step.name == tr("Delete Material")) {
				Context.raw.material = new MaterialSlot(Project.materials[0].data);
				Project.materials.insert(step.material, Context.raw.material);
				Context.raw.material.canvas = step.canvas;
				UINodes.inst.canvasChanged();
				@:privateAccess UINodes.inst.getNodes().handle = new zui.Zui.Handle();
				UINodes.inst.hwnd.redraws = 2;
			}
			else if (step.name == tr("Duplicate Material")) {
				Context.raw.material = Project.materials[step.material];
				step.canvas = Context.raw.material.canvas;
				Context.raw.material.delete();
			}
			else { // Paint operation
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.selectPaintObject(Project.paintObjects[step.object]);
				Context.setLayer(Project.layers[step.layer]);
				Context.raw.layer.swap(lay);
				Context.raw.layerPreviewDirty = true;
			}
			#end

			undos--;
			redos++;
			Context.raw.ddirty = 2;

			#if (is_paint || is_sculpt)
			UIBase.inst.hwnds[TabSidebar0].redraws = 2;
			UIBase.inst.hwnds[TabSidebar1].redraws = 2;
			if (UIView2D.inst.show) {
				UIView2D.inst.hwnd.redraws = 2;
			}
			#end
		}
	}

	public static function redo() {
		if (redos > 0) {
			var active = steps.length - redos;
			var step = steps[active];

			if (step.name == tr("Edit Nodes")) {
				swapCanvas(step);
			}

			#if (is_paint || is_sculpt)
			else if (step.name == tr("New Layer") || step.name == tr("New Black Mask") || step.name == tr("New White Mask") || step.name == tr("New Fill Mask")) {
				var parent = step.layer_parent > 0 ? Project.layers[step.layer_parent - 1] : null;
				var l = new LayerSlot("", step.layer_type, parent);
				Project.layers.insert(step.layer, l);
				if (step.name == tr("New Black Mask")) {
					App.notifyOnNextFrame(function() {
						l.clear(0x00000000);
					});
				}
				else if (step.name == tr("New White Mask")) {
					App.notifyOnNextFrame(function() {
						l.clear(0xffffffff);
					});
				}
				else if (step.name == tr("New Fill Mask")) {
					App.notifyOnNextFrame(function() {
						Context.raw.material = Project.materials[step.material];
						l.toFillLayer();
					});
				}
				Context.raw.layerPreviewDirty = true;
				Context.setLayer(l);
			}
			else if (step.name == tr("New Group")) {
				var l = Project.layers[step.layer - 1];
				var group = App.newGroup();
				Project.layers.remove(group);
				Project.layers.insert(step.layer, group);
				l.parent = group;
				Context.setLayer(group);
			}
			else if (step.name == tr("Delete Layer")) {
				Context.raw.layer = Project.layers[step.layer];
				swapActive();
				Context.raw.layer.delete();

				// Redoing the last delete would result in an empty group
				// Redo deleting all group masks + the group itself
				if (step.layer_type == LayerSlotType.SlotLayer && steps.length >= active + 2 && (steps[active + 1].layer_type == LayerSlotType.SlotGroup || steps[active + 1].layer_type == LayerSlotType.SlotMask)) {
					var n = 1;
					while (steps[active + n].layer_type == LayerSlotType.SlotMask) {
						++n;
					}
					App.notifyOnNextFrame(function() {
						for (i in 0...n) redo();
					});
				}
			}
			else if (step.name == tr("Clear Layer")) {
				Context.raw.layer = Project.layers[step.layer];
				swapActive();
				Context.raw.layer.clear();
				Context.raw.layerPreviewDirty = true;
			}
			else if (step.name == tr("Duplicate Layer")) {
				Context.raw.layer = Project.layers[step.layer];
				function _next() {
					App.duplicateLayer(Context.raw.layer);
				}
				App.notifyOnNextFrame(_next);
			}
			else if (step.name == tr("Order Layers")) {
				var target = Project.layers[step.prev_order];
				Project.layers[step.prev_order] = Project.layers[step.layer];
				Project.layers[step.layer] = target;
			}
			else if (step.name == tr("Merge Layers")) {
				Context.raw.layer = Project.layers[step.layer + 1];
				iron.App.notifyOnInit(redoMergeLayers);
				iron.App.notifyOnInit(App.mergeDown);
			}
			else if (step.name == tr("Apply Mask")) {
				Context.raw.layer = Project.layers[step.layer];
					if (Context.raw.layer.isGroupMask()) {
						var group = Context.raw.layer.parent;
						var layers = group.getChildren();
						layers.insert(0, Context.raw.layer);
						copyMergingLayers2(layers);
					}
					else copyMergingLayers2([Context.raw.layer, Context.raw.layer.parent]);

				function _next() {
					Context.raw.layer.applyMask();
					Context.setLayer(Context.raw.layer);
					Context.raw.layersPreviewDirty = true;
				}
				App.notifyOnNextFrame(_next);
			}
			else if (step.name == tr("Invert Mask")) {
				function _next() {
					Context.raw.layer = Project.layers[step.layer];
					Context.raw.layer.invertMask();
				}
				iron.App.notifyOnInit(_next);
			}
			else if (step.name == tr("Apply Filter")) {
				var lay = undoLayers[undoI];
				Context.setLayer(Project.layers[step.layer]);
				Context.raw.layer.swap(lay);
				App.newMask(false, lay);
				Context.raw.layer.swap(lay);
				Context.raw.layerPreviewDirty = true;
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Fill Layer") || step.name == tr("To Fill Mask")) {
				var lay = undoLayers[undoI];
				Context.raw.layer.swap(lay);
				Context.raw.layer.fill_layer = Project.materials[step.material];
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Paint Layer") || step.name == tr("To Paint Mask")) {
				Context.raw.layer.toPaintLayer();
				var lay = undoLayers[undoI];
				Context.raw.layer.swap(lay);
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("Layer Opacity")) {
				Context.setLayer(Project.layers[step.layer]);
				var t = Context.raw.layer.maskOpacity;
				Context.raw.layer.maskOpacity = step.layer_opacity;
				step.layer_opacity = t;
				MakeMaterial.parseMeshMaterial();
			}
			else if (step.name == tr("Layer Blending")) {
				Context.setLayer(Project.layers[step.layer]);
				var t = Context.raw.layer.blending;
				Context.raw.layer.blending = step.layer_blending;
				step.layer_blending = t;
				MakeMaterial.parseMeshMaterial();
			}
			else if (step.name == tr("Delete Node Group")) {
				swapCanvas(step);
				Project.materialGroups.remove(Project.materialGroups[step.canvas_group]);
			}
			else if (step.name == tr("New Material")) {
				Context.raw.material = new MaterialSlot(Project.materials[0].data);
				Project.materials.insert(step.material, Context.raw.material);
				Context.raw.material.canvas = step.canvas;
				UINodes.inst.canvasChanged();
				@:privateAccess UINodes.inst.getNodes().handle = new zui.Zui.Handle();
				UINodes.inst.hwnd.redraws = 2;
			}
			else if (step.name == tr("Delete Material")) {
				Context.raw.material = Project.materials[step.material];
				step.canvas = Context.raw.material.canvas;
				Context.raw.material.delete();
			}
			else if (step.name == tr("Duplicate Material")) {
				Context.raw.material = new MaterialSlot(Project.materials[0].data);
				Project.materials.insert(step.material, Context.raw.material);
				Context.raw.material.canvas = step.canvas;
				UINodes.inst.canvasChanged();
				@:privateAccess UINodes.inst.getNodes().handle = new zui.Zui.Handle();
				UINodes.inst.hwnd.redraws = 2;
			}
			else { // Paint operation
				var lay = undoLayers[undoI];
				Context.selectPaintObject(Project.paintObjects[step.object]);
				Context.setLayer(Project.layers[step.layer]);
				Context.raw.layer.swap(lay);
				Context.raw.layerPreviewDirty = true;
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			#end

			undos++;
			redos--;
			Context.raw.ddirty = 2;

			#if (is_paint || is_sculpt)
			UIBase.inst.hwnds[TabSidebar0].redraws = 2;
			UIBase.inst.hwnds[TabSidebar1].redraws = 2;
			if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
			#end
		}
	}

	public static function reset() {
		#if (is_paint || is_sculpt)
		steps = [{name: tr("New"), layer: 0, layer_type: SlotLayer, layer_parent: -1, object: 0, material: 0, brush: 0}];
		#end
		#if is_lab
		steps = [{name: tr("New")}];
		#end

		undos = 0;
		redos = 0;
		undoI = 0;
	}

	#if (is_paint || is_sculpt)
	public static function editNodes(canvas: TNodeCanvas, canvas_type: Int, canvas_group: Null<Int> = null) {
	#end
	#if is_lab
	public static function editNodes(canvas: TNodeCanvas, canvas_group: Null<Int> = null) {
	#end
		var step = push(tr("Edit Nodes"));
		step.canvas_group = canvas_group;
		#if (is_paint || is_sculpt)
		step.canvas_type = canvas_type;
		#end
		step.canvas = haxe.Json.parse(haxe.Json.stringify(canvas));
	}

	#if (is_paint || is_sculpt)
	public static function paint() {
		var isMask = Context.raw.layer.isMask();
		copyToUndo(Context.raw.layer.id, undoI, isMask);

		pushUndo = false;
		push(tr(UIToolbar.inst.toolNames[Context.raw.tool]));
	}

	public static function newLayer() {
		push(tr("New Layer"));
	}

	public static function newBlackMask() {
		push(tr("New Black Mask"));
	}

	public static function newWhiteMask() {
		push(tr("New White Mask"));
	}

	public static function newFillMask() {
		push(tr("New Fill Mask"));
	}

	public static function newGroup() {
		push(tr("New Group"));
	}

	public static function duplicateLayer() {
		push(tr("Duplicate Layer"));
	}

	public static function deleteLayer() {
		swapActive();
		push(tr("Delete Layer"));
	}

	public static function clearLayer() {
		swapActive();
		push(tr("Clear Layer"));
	}

	public static function orderLayers(prevOrder: Int) {
		var step = push(tr("Order Layers"));
		step.prev_order = prevOrder;
	}

	public static function mergeLayers() {
		copyMergingLayers();

		var step = push(tr("Merge Layers"));
		step.layer -= 1; // Merge down
		if (Context.raw.layer.hasMasks()) {
			step.layer -= Context.raw.layer.getMasks().length;
		}
		steps.shift(); // Merge consumes 2 steps
		undos--;
		// TODO: use undo layer in App.mergeDown to save memory
	}

	public static function applyMask() {
		if (Context.raw.layer.isGroupMask()) {
			var group = Context.raw.layer.parent;
			var layers = group.getChildren();
			layers.insert(0, Context.raw.layer);
			copyMergingLayers2(layers);
		}
		else copyMergingLayers2([Context.raw.layer, Context.raw.layer.parent]);
		push(tr("Apply Mask"));
	}


	public static function invertMask() {
		push(tr("Invert Mask"));
	}

	@:keep
	public static function applyFilter() {
		copyToUndo(Context.raw.layer.id, undoI, true);
		push(tr("Apply Filter"));
	}

	public static function toFillLayer() {
		copyToUndo(Context.raw.layer.id, undoI, false);
		push(tr("To Fill Layer"));
	}

	public static function toFillMask() {
		copyToUndo(Context.raw.layer.id, undoI, true);
		push(tr("To Fill Mask"));
	}

	public static function toPaintLayer() {
		copyToUndo(Context.raw.layer.id, undoI, false);
		push(tr("To Paint Layer"));
	}

	public static function toPaintMask() {
		copyToUndo(Context.raw.layer.id, undoI, true);
		push(tr("To Paint Mask"));
	}

	public static function layerOpacity() {
		push(tr("Layer Opacity"));
	}

	// public static function layerObject() {
	// 	push("Layer Object");
	// }

	public static function layerBlending() {
		push(tr("Layer Blending"));
	}

	public static function newMaterial() {
		var step = push(tr("New Material"));
		step.canvas_type = 0;
		step.canvas = haxe.Json.parse(haxe.Json.stringify(Context.raw.material.canvas));
	}

	public static function deleteMaterial() {
		var step = push(tr("Delete Material"));
		step.canvas_type = 0;
		step.canvas = haxe.Json.parse(haxe.Json.stringify(Context.raw.material.canvas));
	}

	public static function duplicateMaterial() {
		var step = push(tr("Duplicate Material"));
		step.canvas_type = 0;
		step.canvas = haxe.Json.parse(haxe.Json.stringify(Context.raw.material.canvas));
	}

	public static function deleteMaterialGroup(group: TNodeGroup) {
		var step = push(tr("Delete Node Group"));
		step.canvas_type = CanvasMaterial;
		step.canvas_group = Project.materialGroups.indexOf(group);
		step.canvas = haxe.Json.parse(haxe.Json.stringify(group.canvas));
	}
	#end

	static function push(name: String): TStep {
		#if (krom_windows || krom_linux || krom_darwin)
		var filename = Project.filepath == "" ? UIFiles.filename : Project.filepath.substring(Project.filepath.lastIndexOf(Path.sep) + 1, Project.filepath.length - 4);
		kha.Window.get(0).title = filename + "* - " + Manifest.title;
		#end

		if (Config.raw.touch_ui) {
			// Refresh undo & redo buttons
			arm.ui.UIMenubar.inst.menuHandle.redraws = 2;
		}

		if (undos < Config.raw.undo_steps) undos++;
		if (redos > 0) {
			for (i in 0...redos) steps.pop();
			redos = 0;
		}

		#if (is_paint || is_sculpt)
		var opos = Project.paintObjects.indexOf(Context.raw.paintObject);
		var lpos = Project.layers.indexOf(Context.raw.layer);
		var mpos = Project.materials.indexOf(Context.raw.material);
		var bpos = Project.brushes.indexOf(Context.raw.brush);

		steps.push({
			name: name,
			layer: lpos,
			layer_type: Context.raw.layer.isMask() ? SlotMask : Context.raw.layer.isGroup() ? SlotGroup : SlotLayer,
			layer_parent: Context.raw.layer.parent == null ? -1 : Project.layers.indexOf(Context.raw.layer.parent),
			object: opos,
			material: mpos,
			brush: bpos,
			layer_opacity: Context.raw.layer.maskOpacity,
			layer_object: Context.raw.layer.objectMask,
			layer_blending: Context.raw.layer.blending
		});
		#end

		#if is_lab
		steps.push({
			name: name
		});
		#end

		while (steps.length > Config.raw.undo_steps + 1) steps.shift();
		return steps[steps.length - 1];
	}

	#if (is_paint || is_sculpt)
	static function redoMergeLayers() {
		copyMergingLayers();
	}

	static function copyMergingLayers() {
		var lay = Context.raw.layer;
		copyToUndo(lay.id, undoI, Context.raw.layer.isMask());

		var below = Project.layers.indexOf(lay) - 1;
		lay = Project.layers[below];
		copyToUndo(lay.id, undoI, Context.raw.layer.isMask());
	}

	static function copyMergingLayers2(layers: Array<LayerSlot>) {
		for (layer in layers)
			copyToUndo(layer.id, undoI, layer.isMask());
	}

	static function swapActive() {
		var undoLayer = undoLayers[undoI];
		undoLayer.swap(Context.raw.layer);
		undoI = (undoI + 1) % Config.raw.undo_steps;
	}

	static function copyToUndo(fromId: Int, toId: Int, isMask: Bool) {
		var path = iron.RenderPath.active;

		#if is_sculpt
		isMask = true;
		#end

		if (isMask) {
			path.setTarget("texpaint_undo" + toId);
			path.bindTarget("texpaint" + fromId, "tex");
			// path.drawShader("shader_datas/copy_pass/copyR8_pass");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		else {
			path.setTarget("texpaint_undo" + toId, ["texpaint_nor_undo" + toId, "texpaint_pack_undo" + toId]);
			path.bindTarget("texpaint" + fromId, "tex0");
			path.bindTarget("texpaint_nor" + fromId, "tex1");
			path.bindTarget("texpaint_pack" + fromId, "tex2");
			path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		}
		undoI = (undoI + 1) % Config.raw.undo_steps;
	}
	#end

	static function getCanvasOwner(step: TStep): Dynamic {
		#if (is_paint || is_sculpt)
		return step.canvas_group == null ?
			Project.materials[step.material] :
			Project.materialGroups[step.canvas_group];
		#end

		#if is_lab
		return null;
		#end
	}

	static function swapCanvas(step: TStep) {
		#if (is_paint || is_sculpt)
		if (step.canvas_type == 0) {
			var _canvas = getCanvasOwner(step).canvas;
			getCanvasOwner(step).canvas = step.canvas;
			step.canvas = _canvas;
			Context.raw.material = Project.materials[step.material];
		}
		else {
			var _canvas = Project.brushes[step.brush].canvas;
			Project.brushes[step.brush].canvas = step.canvas;
			step.canvas = _canvas;
			Context.raw.brush = Project.brushes[step.brush];
		}
		#end

		#if is_lab
		var _canvas = getCanvasOwner(step).canvas;
		getCanvasOwner(step).canvas = step.canvas;
		step.canvas = _canvas;
		#end

		UINodes.inst.canvasChanged();
		@:privateAccess UINodes.inst.getNodes().handle = new zui.Zui.Handle();
		UINodes.inst.hwnd.redraws = 2;
	}
}

typedef TStep = {
	public var name: String;
	@:optional public var canvas: TNodeCanvas; // Node history
	@:optional public var canvas_group: Int;
	#if (is_paint || is_sculpt)
	public var layer: Int;
	public var layer_type: LayerSlotType;
	public var layer_parent: Int;
	public var object: Int;
	public var material: Int;
	public var brush: Int;
	@:optional public var layer_opacity: Float;
	@:optional public var layer_object: Int;
	@:optional public var layer_blending: Int;
	@:optional public var prev_order: Int; // Previous layer position
	@:optional public var canvas_type: Int;
	#end
}
