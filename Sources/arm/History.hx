package arm;

import iron.RenderPath;
import arm.ui.UITrait;
import arm.ui.UIView2D;
import arm.data.LayerSlot;

class History {

	public static var steps:Array<TStep>;
	public static var undoI = 0; // Undo layer
	public static var undos = 0; // Undos available
	public static var redos = 0; // Redos available
	public static var pushUndo = false; // Store undo on next paint
	public static var undoLayers:Array<LayerSlot> = null;

	public static function undo() {
		if (undos > 0) {
			var active = History.steps.length - 1 - History.redos;
			var step = steps[active];

			if (step.name == "New Layer") {
				Context.layer = Project.layers[step.layer];
				Layers.deleteSelectedLayer();
			}
			else if (step.name == "Delete Layer") {
				var l = Layers.newLayer(false);
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				l.swap(lay);
				if (step.has_mask) {
					l.createMask(0, false);
					l.swapMask(lay);
					Context.layersPreviewDirty = true;
				}
			}
			else if (step.name == "Duplicate Layer") {
				Context.layer = Project.layers[step.layer + 1];
				Layers.deleteSelectedLayer();
			}
			else if (step.name == "Order Layers") {
				var target = Project.layers[step.prev_order];
				Project.layers[step.prev_order] = Project.layers[step.layer];
				Project.layers[step.layer] = target;
			}
			else if (step.name == "Merge Layers") {
				Context.layer = Project.layers[step.layer];
				Layers.deleteSelectedLayer();

				Context.layer = Layers.newLayer(false);
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				// if (step.has_mask) {
				// 	l.createMask(0, false);
				// 	l.swapMask(lay);
				// }

				Context.layer = Layers.newLayer(false);
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				// if (step.has_mask) {
				// 	l.createMask(0, false);
				// 	l.swapMask(lay);
				// }

				Context.layersPreviewDirty = true;
			}
			else { // Paint operation
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				var opos = lay.targetObject;
				var lpos = lay.targetLayer;
				Context.selectPaintObject(Project.paintObjects[opos]);
				Context.setLayer(Project.layers[lpos], lay.targetIsMask);
				lay.targetIsMask ? Context.layer.swapMask(lay) : Context.layer.swap(lay);
				Context.layerPreviewDirty = true;
			}
			undos--;
			redos++;
			UITrait.inst.hwnd.redraws = 2;
			if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
		}
	}

	public static function redo() {
		if (redos > 0) {
			var active = History.steps.length - History.redos;
			var step = steps[active];

			if (step.name == "New Layer") {
				Layers.newLayer();
			}
			else if (step.name == "Delete Layer") {
				Context.layer = Project.layers[step.layer];
				swapActive();
				Layers.deleteSelectedLayer();
			}
			else if (step.name == "Duplicate Layer") {
				Context.layer = Project.layers[step.layer];
				Context.layer = Context.layer.duplicate();
			}
			else if (step.name == "Order Layers") {
				var target = Project.layers[step.prev_order];
				Project.layers[step.prev_order] = Project.layers[step.layer];
				Project.layers[step.layer] = target;
			}
			else if (step.name == "Merge Layers") {
				Context.layer = Project.layers[step.layer + 1];
				copyMergingLayers();
				iron.App.notifyOnRender(Layers.mergeSelectedLayer);
			}
			else { // Paint operation
				var lay = undoLayers[undoI];
				var opos = lay.targetObject;
				var lpos = lay.targetLayer;
				Context.selectPaintObject(Project.paintObjects[opos]);
				Context.setLayer(Project.layers[lpos], lay.targetIsMask);
				lay.targetIsMask ? Context.layer.swapMask(lay) : Context.layer.swap(lay);
				Context.layerPreviewDirty = true;
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			undos++;
			redos--;
			UITrait.inst.hwnd.redraws = 2;
			if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
		}
	}

	public static function reset() {
		steps = [{name: "New", layer: 0, object: 0}];
		undos = 0;
		redos = 0;
		undoI = 0;
	}

	public static function paint() {
		var isMask = Context.layerIsMask;
		copyLayer(Context.layer.id, undoI, isMask);
		
		pushUndo = false;
		push(UITrait.inst.toolNames[Context.tool]);
	}

	public static function newLayer() {
		push("New Layer");
	}

	public static function duplicateLayer() {
		push("Duplicate Layer");
	}

	public static function deleteLayer() {
		swapActive();
		var step = push("Delete Layer");
		step.has_mask = Context.layer.texpaint_mask != null;
	}

	public static function orderLayers(prevOrder:Int) {
		var step = push("Order Layers");
		step.prev_order = prevOrder;
	}

	public static function mergeLayers(g:kha.graphics4.Graphics) {
		copyMergingLayers();

		var step = push("Merge Layers");
		step.layer -= 1; // Merge down
		steps.shift(); // Merge consumes 2 steps
		iron.App.removeRender(mergeLayers);
		// TODO: use undo layer in Layers.mergeSelectedLayer to save memory
	}

	static function copyMergingLayers() {
		var lay = Context.layer;
		// if (lay.texpaint_mask != null) copyLayer(lay.id, undoI, true);
		copyLayer(lay.id, undoI, false);

		var below = Project.layers.indexOf(lay) - 1;
		lay = Project.layers[below];
		// if (lay.texpaint_mask != null) copyLayer(lay.id, undoI, true);
		copyLayer(lay.id, undoI, false);
	}

	public static function newMask() {
		push("New Mask");
	}

	public static function deleteMask() {
		push("Delete Mask");
	}

	public static function applyMask() {
		push("Apply Mask");
	}

	static function push(name:String):TStep {
		if (undos < Config.raw.undo_steps) undos++;
		if (redos > 0) {
			for (i in 0...redos) steps.pop();
			redos = 0;
		}

		var opos = 0;
		for (i in 0...Project.paintObjects.length) if (Context.object == Project.paintObjects[i]) { opos = i; break; }
		
		var lpos = 0;
		for (i in 0...Project.layers.length) if (Context.layer == Project.layers[i]) { lpos = i; break; }
		
		steps.push({
			name: name,
			layer: lpos,
			object: opos,
		});
		
		while (steps.length > Config.raw.undo_steps + 1) steps.shift();
		return steps[steps.length - 1];
	}

	static function swapActive() {
		var undoLayer = undoLayers[undoI];
		undoLayer.swap(Context.layer);
		if (undoLayer.texpaint_mask != null) {
			undoLayer.swapMask(Context.layer);
		}
		undoLayer.targetObject = Project.paintObjects.indexOf(Context.paintObject);
		undoLayer.targetLayer = Project.layers.indexOf(Context.layer);
		undoLayer.targetIsMask = false;
		undoI = (undoI + 1) % Config.raw.undo_steps;
	}

	static function copyLayer(fromId:Int, toId:Int, isMask:Bool) {
		var path = iron.RenderPath.active;
		if (isMask) {
			path.setTarget("texpaint_mask_undo" + toId);
			path.bindTarget("texpaint_mask" + fromId, "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		else {
			path.setTarget("texpaint_undo" + toId, ["texpaint_nor_undo" + toId, "texpaint_pack_undo" + toId]);
			path.bindTarget((isMask ? "texpaint_mask" : "texpaint") + fromId, "tex0");
			path.bindTarget("texpaint_nor" + fromId, "tex1");
			path.bindTarget("texpaint_pack" + fromId, "tex2");
			path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		}
		var undoLayer = undoLayers[undoI];
		undoLayer.targetObject = Project.paintObjects.indexOf(Context.paintObject);
		undoLayer.targetLayer = Project.layers.indexOf(Context.layer);
		undoLayer.targetIsMask = isMask;
		undoI = (undoI + 1) % Config.raw.undo_steps;
	}
}

typedef TStep = {
	public var name:String;
	public var layer:Int;
	public var object:Int;
	@:optional public var has_mask:Bool; // Deleted layer had mask
	@:optional public var prev_order:Int; // Previous layer position
}
