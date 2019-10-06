package arm;

import iron.RenderPath;
import arm.ui.UITrait;
import arm.ui.UIView2D;
import arm.ui.UIFiles;
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
				Project.layers.insert(step.layer, Project.layers.pop());
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				l.swap(lay);
				if (step.has_mask) {
					l.createMask(0, false);
					l.swapMask(lay);
					Context.setLayer(l, true);
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
				Project.layers.insert(step.layer, Project.layers.pop());
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);

				Context.layer = Layers.newLayer(false);
				Project.layers.insert(step.layer + 1, Project.layers.pop());
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				if (step.has_mask) {
					Context.layer.createMask(0, false);
					Context.layer.swapMask(lay);
					Context.setLayer(Context.layer, true);
				}
				Context.layersPreviewDirty = true;
			}
			else if (step.name == "New Mask") {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.setLayer(Project.layers[step.layer], true);
				Context.layer.swapMask(lay);
				Context.layer.deleteMask();
				Context.setLayer(Project.layers[step.layer], false);
			}
			else if (step.name == "Delete Mask") {
				Context.setLayer(Project.layers[step.layer], false);
				Context.layer.createMask(0, false);
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swapMask(lay);
				Context.layersPreviewDirty = true;
				Context.setLayer(Context.layer, true);
			}
			else if (step.name == "Apply Mask") {
				Context.layer = Project.layers[step.layer];
				Layers.deleteSelectedLayer();

				Context.layer = Layers.newLayer(false);
				Project.layers.insert(step.layer, Project.layers.pop());
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				Context.layer.createMask(0, false);
				Context.layer.swapMask(lay);

				Context.layersPreviewDirty = true;
				Context.setLayer(Context.layer, true);
			}
			else if (step.name == "To Fill Layer") {
				Layers.toPaintLayer(Context.layer);
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
			}
			else if (step.name == "To Paint Layer") {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				Context.layer.material_mask = Project.materials[step.material];
			}
			else { // Paint operation
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.selectPaintObject(Project.paintObjects[step.object]);
				Context.setLayer(Project.layers[step.layer], step.is_mask);
				step.is_mask ? Context.layer.swapMask(lay) : Context.layer.swap(lay);
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
				Project.layers.insert(step.layer, Project.layers.pop());
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
				iron.App.notifyOnRender(redoMergeLayers);
				iron.App.notifyOnRender(Layers.mergeSelectedLayer);
			}
			else if (step.name == "New Mask") {
				Context.layer = Project.layers[step.layer];
				Context.layer.createMask(0, false);
				Context.setLayer(Project.layers[step.layer], true);
				var lay = undoLayers[undoI];
				Context.layer.swapMask(lay);
				Context.layerPreviewDirty = true;
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == "Delete Mask") {
				Context.layer = Project.layers[step.layer];
				swapMaskActive();
				Context.layer.deleteMask();
				Context.setLayer(Context.layer, false);
			}
			else if (step.name == "Apply Mask") {
				function makeApply(g:kha.graphics4.Graphics) {
					g.end();
					Context.layer = Project.layers[step.layer];
					copyToUndoWithMask();
					Context.layer.applyMask();
					Context.setLayer(Context.layer, false);
					g.begin();
					iron.App.removeRender(makeApply);
				}
				iron.App.notifyOnRender(makeApply);
			}
			else if (step.name == "To Fill Layer") {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				Context.layer.material_mask = Project.materials[step.material];
			}
			else if (step.name == "To Paint Layer") {
				Layers.toPaintLayer(Context.layer);
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
			}
			else { // Paint operation
				var lay = undoLayers[undoI];
				Context.selectPaintObject(Project.paintObjects[step.object]);
				Context.setLayer(Project.layers[step.layer], step.is_mask);
				step.is_mask ? Context.layer.swapMask(lay) : Context.layer.swap(lay);
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
		steps = [{name: "New", layer: 0, object: 0, material: 0, is_mask: false, has_mask: false}];
		undos = 0;
		redos = 0;
		undoI = 0;
	}

	public static function paint() {
		var isMask = Context.layerIsMask;
		copyToUndo(Context.layer.id, undoI, isMask);

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
		push("Delete Layer");
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
		undos--;
		iron.App.removeRender(mergeLayers);
		// TODO: use undo layer in Layers.mergeSelectedLayer to save memory
	}

	public static function newMask() {
		push("New Mask");
	}

	public static function deleteMask() {
		swapMaskActive();
		push("Delete Mask");
	}

	public static function applyMask() {
		copyToUndoWithMask();
		push("Apply Mask");
	}

	public static function toFillLayer() {
		copyToUndo(Context.layer.id, undoI, false);
		push("To Fill Layer");
	}

	public static function toPaintLayer() {
		copyToUndo(Context.layer.id, undoI, false);
		push("To Paint Layer");
	}

	// public static function newMaterial() {}
	// public static function deleteMaterial() {}

	static function push(name:String):TStep {
		kha.Window.get(0).title = UIFiles.filename + "* - ArmorPaint";

		if (undos < Config.raw.undo_steps) undos++;
		if (redos > 0) {
			for (i in 0...redos) steps.pop();
			redos = 0;
		}

		var opos = Project.paintObjects.indexOf(cast Context.object);
		var lpos = Project.layers.indexOf(Context.layer);
		var mpos = Project.materials.indexOf(Context.material);

		steps.push({
			name: name,
			layer: lpos,
			object: opos,
			material: mpos,
			is_mask: Context.layerIsMask,
			has_mask: Context.layer.texpaint_mask != null
		});

		while (steps.length > Config.raw.undo_steps + 1) steps.shift();
		return steps[steps.length - 1];
	}

	static function redoMergeLayers(g:kha.graphics4.Graphics) {
		copyMergingLayers();
		iron.App.removeRender(redoMergeLayers);
	}

	static function copyMergingLayers() {
		var lay = Context.layer;
		copyToUndo(lay.id, undoI, false);
		if (lay.texpaint_mask != null) {
			undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
			copyToUndo(lay.id, undoI, true);
		}

		var below = Project.layers.indexOf(lay) - 1;
		lay = Project.layers[below];
		copyToUndo(lay.id, undoI, false);
	}

	static function copyToUndoWithMask() {
		copyToUndo(Context.layer.id, undoI, false);
		undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
		copyToUndo(Context.layer.id, undoI, true);
	}

	static function swapActive() {
		var undoLayer = undoLayers[undoI];
		undoLayer.swap(Context.layer);
		if (Context.layer.texpaint_mask != null) {
			undoLayer.swapMask(Context.layer);
		}
		undoI = (undoI + 1) % Config.raw.undo_steps;
	}

	static function swapMaskActive() {
		var undoLayer = undoLayers[undoI];
		undoLayer.swapMask(Context.layer);
		undoI = (undoI + 1) % Config.raw.undo_steps;
	}

	static function copyToUndo(fromId:Int, toId:Int, isMask:Bool) {
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
		undoI = (undoI + 1) % Config.raw.undo_steps;
	}
}

typedef TStep = {
	public var name:String;
	public var layer:Int;
	public var object:Int;
	public var material:Int;
	public var is_mask:Bool; // Mask operation
	public var has_mask:Bool; // Layer contains mask
	@:optional public var prev_order:Int; // Previous layer position
}
