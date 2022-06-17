package arm;

import arm.Project.TNodeGroup;
import zui.Nodes;
import arm.ui.UISidebar;
import arm.ui.UIView2D;
import arm.ui.UIFiles;
import arm.ui.UINodes;
import arm.ui.UIToolbar;
import arm.sys.Path;
import arm.data.LayerSlot;
import arm.data.MaterialSlot;
import arm.node.MakeMaterial;
import arm.Enums;
import haxe.ds.GenericStack;

class History {

	public static var steps: Array<TStep>;
	public static var undoI = 0; // Undo layer
	public static var undos = 0; // Undos available
	public static var redos = 0; // Redos available
	public static var pushUndo = false; // Store undo on next paint
	public static var undoLayers: Array<LayerSlot> = null;
	public static var stepStack: GenericStack<TStep>;

	public static function undo() {
		if (undos == 0)
			return;

		var active = steps.length - 1 - redos;

		undoInternal(active);
	}

	public static function undoInternal(active: Int): Int {
		var step = steps[active];
		var numChildrenTotal = 0;

		if (step.name == tr("New Layer") || step.name == tr("New Black Mask") || step.name == tr("New White Mask") || step.name == tr("New Fill Mask")) {
			Context.layer = LayerSlot.findById(step.layer_id);
			Context.layer.delete();
			Context.layer = Project.layers[step.layer > 0 ? step.layer - 1 : 0];
		}
		else if (step.name == tr("New Group")) {
			Context.layer = LayerSlot.findById(step.layer_id);
			// The layer below is the only layer in the group. Its layer masks are automatically unparented, too.
			Project.layers[step.layer - 1].parent = null;
			Context.layer.delete();
			Context.layer = Project.layers[step.layer > 0 ? step.layer - 1 : 0];
		}
		else if (step.name == tr("Delete Layer")) {
			var parent = step.layer_parent > 0 ? LayerSlot.findById(step.layer_parent) : null;
			var position = step.layer;
			var l = new LayerSlot("", step.layer_type, parent, step.layer_id);
			Project.layers.insert(position, l);
			Context.setLayer(l);
			undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
			var lay = undoLayers[undoI];
			l.swap(lay);
			l.maskOpacity = step.layer_opacity;
			l.blending = step.layer_blending;
			l.objectMask = step.layer_object;
			MakeMaterial.parseMeshMaterial();
		}
		else if (step.name == tr("Clear Layer")) {
			undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
			var lay = undoLayers[undoI];
			Context.layer.swap(lay);
			Context.layerPreviewDirty = true;
		}
		else if (step.name == tr("Duplicate Layer")) {
			var children = Project.layers[step.layer].getRecursiveChildren();
			var position = step.layer + 1;
			if (children != null)
				position += children.length;

			Context.layer = Project.layers[position];
			Context.layer.delete();
		}
		else if (step.name == tr("Order Layers")) {
			var target = Project.layers[step.prev_order];
			Project.layers[step.prev_order] = Project.layers[step.layer];
			Project.layers[step.layer] = target;
		}
		else if (step.name == tr("Merge Layers") || step.name == tr("Merge Group (internal)") || step.name == tr("Apply Mask") || step.name == tr("Apply Masks (internal)")) {
			// Each step can have more than one child and they may not be laid out
			// sequentially since there could be more than one step in between them.
			// This applies to _all_ children on all levels of nesting.
			// So we need to take this into account when iterating over children by
			// skipping all children of the nested steps.
			for (i in 1...step.num_children + 1) {
				numChildrenTotal += undoInternal(active - i - numChildrenTotal);
			}
		}
		else if (step.name == tr("Apply Mask (internal)")) {
			undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
			var lay = undoLayers[undoI];
			Context.setLayer(LayerSlot.findById(step.layer_id));
			Context.layer.swap(lay);
		}
		else if (step.name == tr("Invert Mask")) {
			function _next() {
				Context.layer = Project.layers[step.layer];
				Context.layer.invertMask();
			}
			iron.App.notifyOnInit(_next);
		}
		else if (step.name == "Apply Filter") {
			undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
			var lay = undoLayers[undoI];
			Context.setLayer(Project.layers[step.layer]);
			Context.layer.swap(lay);
			Layers.newMask(false, Context.layer);
			Context.layer.swap(lay);
			Context.layerPreviewDirty = true;
		}
		else if (step.name == tr("To Fill Layer") || step.name == tr("To Fill Mask")) {
			Context.layer.toPaintLayer();
			undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
			var lay = undoLayers[undoI];
			Context.layer.swap(lay);
		}
		else if (step.name == tr("To Paint Layer") || step.name == tr("To Paint Mask")) {
			undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
			var lay = undoLayers[undoI];
			Context.layer.swap(lay);
			Context.layer.fill_layer = Project.materials[step.material];
		}
		else if (step.name == tr("Layer Opacity")) {
			Context.setLayer(Project.layers[step.layer]);
			var t = Context.layer.maskOpacity;
			Context.layer.maskOpacity = step.layer_opacity;
			step.layer_opacity = t;
			MakeMaterial.parseMeshMaterial();
		}
		else if (step.name == tr("Layer Blending")) {
			Context.setLayer(Project.layers[step.layer]);
			var t = Context.layer.blending;
			Context.layer.blending = step.layer_blending;
			step.layer_blending = t;
			MakeMaterial.parseMeshMaterial();
		}
		else if (step.name == tr("Edit Nodes")) {
			swapCanvas(step);
		}
		else if (step.name == tr("Delete Node Group")) {
			Project.materialGroups.insert(step.canvas_group, { canvas: null, nodes: new Nodes() });
			swapCanvas(step);
		}
		else if (step.name == tr("New Material")) {
			Context.material = Project.materials[step.material];
			step.canvas = Context.material.canvas;
			Context.material.delete();
		}
		else if (step.name == tr("Delete Material")) {
			Context.material = new MaterialSlot(Project.materials[0].data);
			Project.materials.insert(step.material, Context.material);
			Context.material.canvas = step.canvas;
			UINodes.inst.canvasChanged();
			@:privateAccess UINodes.inst.getNodes().handle = new zui.Zui.Handle();
			UINodes.inst.hwnd.redraws = 2;
		}
		else if (step.name == tr("Duplicate Material")) {
			Context.material = Project.materials[step.material];
			step.canvas = Context.material.canvas;
			Context.material.delete();
		}
		else { // Paint operation
			undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
			var lay = undoLayers[undoI];
			Context.selectPaintObject(Project.paintObjects[step.object]);
			Context.setLayer(Project.layers[step.layer]);
			Context.layer.swap(lay);
			Context.layerPreviewDirty = true;
		}
		undos--;
		redos++;
		UISidebar.inst.hwnd0.redraws = 2;
		UISidebar.inst.hwnd1.redraws = 2;
		Context.ddirty = 2;
		if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
		// Return total number of steps consumed during execution of this method
		// (including any steps consumed during recursion)
		return numChildrenTotal + step.num_children;
	}

	public static function redo() {
		if (redos > 0) {
			var active = steps.length - redos;
			while (steps[active].internal) {
				active += 1;
				undos++;
				redos--;
			}
			var step = steps[active];

			if (step.name == tr("New Layer") || step.name == tr("New Black Mask") || step.name == tr("New White Mask") || step.name == tr("New Fill Mask")) {
				var parent = step.layer_parent > 0 ? LayerSlot.findById(step.layer_parent) : null;
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
						Context.material = Project.materials[step.material];
						l.toFillLayer();
					});
				}
				Context.layerPreviewDirty = true;
				Context.setLayer(l);
			}
			else if (step.name == tr("New Group")) {
				var l = Project.layers[step.layer - 1];
				var group = Layers.newGroup();
				Project.layers.remove(group);
				Project.layers.insert(step.layer, group);
				l.parent = group;
				Context.setLayer(group);
			}
			else if (step.name == tr("Delete Layer")) {
				Context.layer = Project.layers[step.layer];
				swapActive();
				Context.layer.delete();

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
				Context.layer = Project.layers[step.layer];
				swapActive();
				Context.layer.clear();
				Context.layerPreviewDirty = true;
			}
			else if (step.name == tr("Duplicate Layer")) {
				Context.layer = Project.layers[step.layer];
				function _next() {
					Layers.duplicateLayer(Context.layer);
				}
				App.notifyOnNextFrame(_next);
			}
			else if (step.name == tr("Order Layers")) {
				var target = Project.layers[step.prev_order];
				Project.layers[step.prev_order] = Project.layers[step.layer];
				Project.layers[step.layer] = target;
			}
			else if (step.name == tr("Merge Layers")) {
				Context.layer = Project.layers[step.layer];
				iron.App.notifyOnInit(Layers.mergeDown);
			}
			else if (step.name == tr("Apply Mask")) {
				Context.layer = Project.layers[step.layer];
					if (Context.layer.isGroupMask()) {
						var group = Context.layer.parent;
						var layers = group.getChildren();
						layers.insert(0, Context.layer);
						copyMergingLayers2(layers);	
					}
					else copyMergingLayers2([Context.layer, Context.layer.parent]);

				function _next() {
					Context.layer.applyMask();
					Context.setLayer(Context.layer);
					Context.layersPreviewDirty = true;
				}
				App.notifyOnNextFrame(_next);
			}
			else if (step.name == tr("Invert Mask")) {
				function _next() {
					Context.layer = Project.layers[step.layer];
					Context.layer.invertMask();
				}
				iron.App.notifyOnInit(_next);
			}
			else if (step.name == tr("Apply Filter")) {
				var lay = undoLayers[undoI];
				Context.setLayer(Project.layers[step.layer]);
				Context.layer.swap(lay);
				Layers.newMask(false, lay);
				Context.layer.swap(lay);
				Context.layerPreviewDirty = true;
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Fill Layer") || step.name == tr("To Fill Mask")) {
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				Context.layer.fill_layer = Project.materials[step.material];
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Paint Layer") || step.name == tr("To Paint Mask")) {
				Context.layer.toPaintLayer();
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("Layer Opacity")) {
				Context.setLayer(Project.layers[step.layer]);
				var t = Context.layer.maskOpacity;
				Context.layer.maskOpacity = step.layer_opacity;
				step.layer_opacity = t;
				MakeMaterial.parseMeshMaterial();
			}
			else if (step.name == tr("Layer Blending")) {
				Context.setLayer(Project.layers[step.layer]);
				var t = Context.layer.blending;
				Context.layer.blending = step.layer_blending;
				step.layer_blending = t;
				MakeMaterial.parseMeshMaterial();
			}
			else if (step.name == tr("Edit Nodes")) {
				swapCanvas(step);
			}
			else if (step.name == tr("Delete Node Group")) {
				swapCanvas(step);
				Project.materialGroups.remove(Project.materialGroups[step.canvas_group]);
			}
			else if (step.name == tr("New Material")) {
				Context.material = new MaterialSlot(Project.materials[0].data);
				Project.materials.insert(step.material, Context.material);
				Context.material.canvas = step.canvas;
				UINodes.inst.canvasChanged();
				@:privateAccess UINodes.inst.getNodes().handle = new zui.Zui.Handle();
				UINodes.inst.hwnd.redraws = 2;
			}
			else if (step.name == tr("Delete Material")) {
				Context.material = Project.materials[step.material];
				step.canvas = Context.material.canvas;
				Context.material.delete();
			}
			else if (step.name == tr("Duplicate Material")) {
				Context.material = new MaterialSlot(Project.materials[0].data);
				Project.materials.insert(step.material, Context.material);
				Context.material.canvas = step.canvas;
				UINodes.inst.canvasChanged();
				@:privateAccess UINodes.inst.getNodes().handle = new zui.Zui.Handle();
				UINodes.inst.hwnd.redraws = 2;
			}
			else { // Paint operation
				var lay = undoLayers[undoI];
				Context.selectPaintObject(Project.paintObjects[step.object]);
				Context.setLayer(Project.layers[step.layer]);
				Context.layer.swap(lay);
				Context.layerPreviewDirty = true;
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			undos++;
			redos--;
			UISidebar.inst.hwnd0.redraws = 2;
			UISidebar.inst.hwnd1.redraws = 2;
			Context.ddirty = 2;
			if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
		}
	}

	public static function reset() {
		steps = [{
			name: tr("New"), 
			internal: false, 
			num_children: 0, 
			layer: 0, 
			layer_id: 0,
			layer_type: SlotLayer, 
			layer_parent: -1, 
			object: 0, 
			material: 0, 
			brush: 0
		}];
		undos = 0;
		redos = 0;
		undoI = 0;
		stepStack = new GenericStack<TStep>();
	}

	public static function paint() {
		var isMask = Context.layer.isMask();
		copyToUndo(Context.layer.id, undoI, isMask);

		pushUndo = false;
		push(tr(UIToolbar.inst.toolNames[Context.tool]));
	}

	public static function newLayer() {
		push(tr("New Layer"));
	}

	public static function newLayer2(layer: LayerSlot) {
		push(tr("New Layer"), layer);
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

	public static function deleteLayer2(l: LayerSlot) {
		swapActive2(l);
		push(tr("Delete Layer"), l);
	}

	public static function applyMask(l: LayerSlot) {
		swapActive2(l);
		push(tr("Apply Mask (internal)"), l);
	}

	public static function clearLayer() {
		swapActive();
		push(tr("Clear Layer"));
	}

	public static function orderLayers(prevOrder: Int) {
		var step = push(tr("Order Layers"));
		step.prev_order = prevOrder;
	}

	public static function beginMergeLayers() {
		begin(tr("Merge Layers"));
		// TODO: use undo layer in Layers.mergeDown to save memory
	}

	public static function beginMergeGroup() {
		begin(tr("Merge Group (internal)"));
	}

	public static function beginApplyMask() {
		begin(tr("Apply Mask"));
	}

	public static function beginApplyMasks() {
		begin(tr("Apply Masks (internal)"));
	}

	// public static function applyMask() {
	// 	if (Context.layer.isGroupMask()) {
	// 		var group = Context.layer.parent;
	// 		var layers = group.getChildren();
	// 		layers.insert(0, Context.layer);
	// 		copyMergingLayers2(layers);	
	// 	}
	// 	else copyMergingLayers2([Context.layer, Context.layer.parent]);
	// 	push(tr("Apply Mask"));
	// }

	public static function invertMask() {
		push(tr("Invert Mask"));
	}

	@:keep
	public static function applyFilter() {
		copyToUndo(Context.layer.id, undoI, true);
		push(tr("Apply Filter"));
	}

	public static function toFillLayer() {
		copyToUndo(Context.layer.id, undoI, false);
		push(tr("To Fill Layer"));
	}

	public static function toFillMask() {
		copyToUndo(Context.layer.id, undoI, true);
		push(tr("To Fill Mask"));
	}

	public static function toPaintLayer() {
		copyToUndo(Context.layer.id, undoI, false);
		push(tr("To Paint Layer"));
	}

	public static function toPaintMask() {
		copyToUndo(Context.layer.id, undoI, true);
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
		step.canvas = haxe.Json.parse(haxe.Json.stringify(Context.material.canvas));
	}

	public static function deleteMaterial() {
		var step = push(tr("Delete Material"));
		step.canvas_type = 0;
		step.canvas = haxe.Json.parse(haxe.Json.stringify(Context.material.canvas));
	}

	public static function duplicateMaterial() {
		var step = push(tr("Duplicate Material"));
		step.canvas_type = 0;
		step.canvas = haxe.Json.parse(haxe.Json.stringify(Context.material.canvas));
	}

	public static function editNodes(canvas: TNodeCanvas, canvas_type: Int, canvas_group: Null<Int> = null) {
		var step = push(tr("Edit Nodes"));
		step.canvas_type = canvas_type;
		step.canvas_group = canvas_group;
		step.canvas = haxe.Json.parse(haxe.Json.stringify(canvas));
	}

	public static function deleteMaterialGroup(group: TNodeGroup) {
		var step = push(tr("Delete Node Group"));
		step.canvas_type = CanvasMaterial;
		step.canvas_group = Project.materialGroups.indexOf(group);
		step.canvas = haxe.Json.parse(haxe.Json.stringify(group.canvas));
	}

	static function push(name: String, layer: LayerSlot = null): TStep {
		#if (krom_windows || krom_linux || krom_darwin)
		var filename = Project.filepath == "" ? UIFiles.filename : Project.filepath.substring(Project.filepath.lastIndexOf(Path.sep) + 1, Project.filepath.length - 4);
		kha.Window.get(0).title = filename + "* - " + Main.title;
		#end

		if (undos < Config.raw.undo_steps) undos++;
		if (redos > 0) {
			for (i in 0...redos) steps.pop();
			redos = 0;
		}

		steps.push(prepareStep(name, layer));

		if (!stepStack.isEmpty()) {
			stepStack.first().num_children += 1;
		}

		while (steps.length > Config.raw.undo_steps + 1) steps.shift();
		return steps[steps.length - 1];
	}

	static function push2(step: TStep) {
		#if (krom_windows || krom_linux || krom_darwin)
		var filename = Project.filepath == "" ? UIFiles.filename : Project.filepath.substring(Project.filepath.lastIndexOf(Path.sep) + 1, Project.filepath.length - 4);
		kha.Window.get(0).title = filename + "* - " + Main.title;
		#end

		if (undos < Config.raw.undo_steps) undos++;
		if (redos > 0) {
			for (i in 0...redos) steps.pop();
			redos = 0;
		}

		steps.push(step);

		if (!stepStack.isEmpty()) {
			stepStack.first().num_children += 1;
		}

		while (steps.length > Config.raw.undo_steps + 1) steps.shift();
		return steps[steps.length - 1];
	}

	static function prepareStep(name: String, layer: LayerSlot = null): TStep {
		var internal = !stepStack.isEmpty();
		if (layer == null) {
			layer = Context.layer;
		}
		
		var opos = Project.paintObjects.indexOf(Context.paintObject);
		var lpos = Project.layers.indexOf(layer);
		var mpos = Project.materials.indexOf(Context.material);
		var bpos = Project.brushes.indexOf(Context.brush);
		
		var layer_type = layer.isMask() ? SlotMask : layer.isGroup() ? SlotGroup : SlotLayer;

		var step: TStep = {
			name: name,
			internal: internal,
			num_children: 0,
			layer: lpos,
			layer_id: layer.id,
			layer_type: layer_type,
			layer_parent: layer.parent == null ? -1 : layer.parent.id,
			object: opos,
			material: mpos,
			brush: bpos,
			layer_opacity: layer.maskOpacity,
			layer_object: layer.objectMask,
			layer_blending: layer.blending
		};

		return step;
	}

	static function redoMergeLayers() {
		copyMergingLayers();
	}

	static function copyMergingLayers() {
		var lay = Context.layer;
		copyToUndo(lay.id, undoI, Context.layer.isMask());

		var below = Project.layers.indexOf(lay) - 1;
		lay = Project.layers[below];
		copyToUndo(lay.id, undoI, Context.layer.isMask());
	}

	static function copyMergingLayers2(layers: Array<LayerSlot>) {
		for (layer in layers)
			copyToUndo(layer.id, undoI, layer.isMask());
	}

	static function swapActive() {
		var undoLayer = undoLayers[undoI];
		undoLayer.swap(Context.layer);
		undoI = (undoI + 1) % Config.raw.undo_steps;
	}

	static function swapActive2(l: LayerSlot) {
		var undoLayer = undoLayers[undoI];
		undoLayer.swap(l);
		undoI = (undoI + 1) % Config.raw.undo_steps;
	}

	static function copyToUndo(fromId: Int, toId: Int, isMask: Bool) {
		var path = iron.RenderPath.active;
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

	static function getCanvasOwner(step: TStep): Dynamic {
		return step.canvas_group == null ? Project.materials[step.material] : Project.materialGroups[step.canvas_group];
	}

	static function swapCanvas(step: TStep) {
		if (step.canvas_type == 0) {
			var _canvas = getCanvasOwner(step).canvas;
			getCanvasOwner(step).canvas = step.canvas;
			step.canvas = _canvas;
			Context.material = Project.materials[step.material];
		}
		else {
			var _canvas = Project.brushes[step.brush].canvas;
			Project.brushes[step.brush].canvas = step.canvas;
			step.canvas = _canvas;
			Context.brush = Project.brushes[step.brush];
		}

		UINodes.inst.canvasChanged();
		@:privateAccess UINodes.inst.getNodes().handle = new zui.Zui.Handle();
		UINodes.inst.hwnd.redraws = 2;
	}

	static function begin(name: String, layer: LayerSlot = null) {
		stepStack.add(prepareStep(name, layer));
	}

	public static function end() {
		var step = stepStack.pop();
		push2(step); // The step itself should be added last
	}
}

typedef TStep = {
	public var name: String;
	public var internal: Bool;
	public var num_children: Int;
	public var layer: Int;
	public var layer_id: Int;
	public var layer_type: LayerSlotType;
	public var layer_parent: Int;
	public var object: Int;
	public var material: Int;
	public var brush: Int;
	@:optional public var layer_opacity: Float;
	@:optional public var layer_object: Int;
	@:optional public var layer_blending: Int;
	@:optional public var prev_order: Int; // Previous layer position
	@:optional public var canvas: TNodeCanvas; // Node history
	@:optional public var canvas_type: Int;
	@:optional public var canvas_group: Int;
}
