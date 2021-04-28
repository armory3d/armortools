package arm;

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

class History {

	public static var steps: Array<TStep>;
	public static var undoI = 0; // Undo layer
	public static var undos = 0; // Undos available
	public static var redos = 0; // Redos available
	public static var pushUndo = false; // Store undo on next paint
	public static var undoLayers: Array<LayerSlot> = null;

	public static function undo() {
		if (undos > 0) {
			var active = steps.length - 1 - redos;
			var step = steps[active];

			if (step.name == tr("New Layer")) {
				Context.layer = Project.layers[step.layer];
				Context.layer.delete();
			}
			else if (step.name == tr("Delete Layer")) {
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
				Context.layer = Project.layers[step.layer + 1];
				Context.layer.delete();
			}
			else if (step.name == tr("Order Layers")) {
				var target = Project.layers[step.prev_order];
				Project.layers[step.prev_order] = Project.layers[step.layer];
				Project.layers[step.layer] = target;
			}
			else if (step.name == tr("Merge Layers")) {
				Context.layer = Project.layers[step.layer];
				Context.layer.delete();

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
				Context.layer.maskOpacity = step.layer_opacity;
				Context.layer.blending = step.layer_blending;
				Context.layer.objectMask = step.layer_object;
				Context.layersPreviewDirty = true;
				MakeMaterial.parseMeshMaterial();
			}
			else if (step.name == tr("New Mask")) {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.setLayer(Project.layers[step.layer], true);
				Context.layer.swapMask(lay);
				Context.layer.deleteMask();
				Context.setLayer(Project.layers[step.layer], false);
			}
			else if (step.name == tr("Delete Mask")) {
				Context.setLayer(Project.layers[step.layer], false);
				Context.layer.createMask(0, false);
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swapMask(lay);
				Context.layersPreviewDirty = true;
				Context.setLayer(Context.layer, true);
			}
			else if (step.name == tr("Apply Mask")) {
				Context.layer = Project.layers[step.layer];
				Context.layer.delete();

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
			else if (step.name == "Apply Filter") {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.setLayer(Project.layers[step.layer], step.is_mask);
				Context.layer.swap(lay);
				Context.layer.createMask(0, false);
				Context.layer.swapMask(lay);
				lay.deleteMask();
				Context.layerPreviewDirty = true;
			}
			else if (step.name == tr("To Fill Layer")) {
				Context.layer.toPaintLayer();
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
			}
			else if (step.name == tr("To Paint Layer")) {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				Context.layer.fill_layer = Project.materials[step.material];
			}
			else if (step.name == tr("To Fill Mask")) {
				Context.layer.toPaintMask();
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swapMask(lay);
			}
			else if (step.name == tr("To Paint Mask")) {
				undoI = undoI - 1 < 0 ? Config.raw.undo_steps - 1 : undoI - 1;
				var lay = undoLayers[undoI];
				Context.layer.swapMask(lay);
				Context.layer.fill_mask = Project.materials[step.material];
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
				Context.setLayer(Project.layers[step.layer], step.is_mask);
				step.is_mask ? Context.layer.swapMask(lay) : Context.layer.swap(lay);
				Context.layerPreviewDirty = true;
			}
			undos--;
			redos++;
			UISidebar.inst.hwnd0.redraws = 2;
			UISidebar.inst.hwnd1.redraws = 2;
			Context.ddirty = 2;
			if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
		}
	}

	public static function redo() {
		if (redos > 0) {
			var active = steps.length - redos;
			var step = steps[active];

			if (step.name == tr("New Layer")) {
				Layers.newLayer();
				Project.layers.insert(step.layer, Project.layers.pop());
			}
			else if (step.name == tr("Delete Layer")) {
				Context.layer = Project.layers[step.layer];
				swapActive();
				Context.layer.delete();
			}
			else if (step.name == tr("Clear Layer")) {
				Context.layer = Project.layers[step.layer];
				swapActive();
				Context.layer.clearLayer();
				Context.layerPreviewDirty = true;
			}
			else if (step.name == tr("Duplicate Layer")) {
				Context.layer = Project.layers[step.layer];
				Context.layer = Context.layer.duplicate();
			}
			else if (step.name == tr("Order Layers")) {
				var target = Project.layers[step.prev_order];
				Project.layers[step.prev_order] = Project.layers[step.layer];
				Project.layers[step.layer] = target;
			}
			else if (step.name == tr("Merge Layers")) {
				Context.layer = Project.layers[step.layer + 1];
				iron.App.notifyOnInit(redoMergeLayers);
				iron.App.notifyOnInit(Layers.mergeDown);
			}
			else if (step.name == tr("New Mask")) {
				Context.layer = Project.layers[step.layer];
				Context.layer.createMask(0, false);
				Context.setLayer(Project.layers[step.layer], true);
				var lay = undoLayers[undoI];
				Context.layer.swapMask(lay);
				Context.layerPreviewDirty = true;
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("Delete Mask")) {
				Context.layer = Project.layers[step.layer];
				swapMaskActive();
				Context.layer.deleteMask();
				Context.setLayer(Context.layer, false);
			}
			else if (step.name == tr("Apply Mask")) {
				function _next() {
					Context.layer = Project.layers[step.layer];
					copyToUndoWithMask();
					Context.layer.applyMask();
					Context.setLayer(Context.layer, false);
				}
				App.notifyOnNextFrame(_next);
			}
			else if (step.name == tr("Apply Filter")) {
				var lay = undoLayers[undoI];
				Context.setLayer(Project.layers[step.layer], false);
				Context.layer.swap(lay);
				lay.createMask(0, false);
				Context.layer.swapMask(lay);
				Context.layer.deleteMask();
				Context.layerPreviewDirty = true;
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Fill Layer")) {
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				Context.layer.fill_layer = Project.materials[step.material];
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Paint Layer")) {
				Context.layer.toPaintLayer();
				var lay = undoLayers[undoI];
				Context.layer.swap(lay);
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Fill Mask")) {
				var lay = undoLayers[undoI];
				Context.layer.swapMask(lay);
				Context.layer.fill_mask = Project.materials[step.material];
				undoI = (undoI + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Paint Mask")) {
				Context.layer.toPaintMask();
				var lay = undoLayers[undoI];
				Context.layer.swapMask(lay);
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
				Context.setLayer(Project.layers[step.layer], step.is_mask);
				step.is_mask ? Context.layer.swapMask(lay) : Context.layer.swap(lay);
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
		steps = [{name: tr("New"), layer: 0, object: 0, material: 0, brush: 0, is_mask: false, has_mask: false}];
		undos = 0;
		redos = 0;
		undoI = 0;
	}

	public static function paint() {
		var isMask = Context.layerIsMask;
		copyToUndo(Context.layer.id, undoI, isMask);

		pushUndo = false;
		push(tr(UIToolbar.inst.toolNames[Context.tool]));
	}

	public static function newLayer() {
		push(tr("New Layer"));
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
		steps.shift(); // Merge consumes 2 steps
		undos--;
		// TODO: use undo layer in Layers.mergeDown to save memory
	}

	public static function newMask() {
		push(tr("New Mask"));
	}

	public static function deleteMask() {
		swapMaskActive();
		push(tr("Delete Mask"));
	}

	public static function applyMask() {
		copyToUndoWithMask();
		push(tr("Apply Mask"));
	}

	@:keep
	public static function applyFilter() {
		copyToUndoWithMask();
		push(tr("Apply Filter"));
	}

	public static function toFillLayer() {
		copyToUndo(Context.layer.id, undoI, false);
		push(tr("To Fill Layer"));
	}

	public static function toPaintLayer() {
		copyToUndo(Context.layer.id, undoI, false);
		push(tr("To Paint Layer"));
	}

	public static function toFillMask() {
		copyToUndo(Context.layer.id, undoI, true);
		push(tr("To Fill Mask"));
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

	static function push(name: String): TStep {
		var filename = Project.filepath == "" ? UIFiles.filename : Project.filepath.substring(Project.filepath.lastIndexOf(Path.sep) + 1, Project.filepath.length - 4);
		kha.Window.get(0).title = filename + "* - ArmorPaint";

		if (undos < Config.raw.undo_steps) undos++;
		if (redos > 0) {
			for (i in 0...redos) steps.pop();
			redos = 0;
		}

		var opos = Project.paintObjects.indexOf(Context.paintObject);
		var lpos = Project.layers.indexOf(Context.layer);
		var mpos = Project.materials.indexOf(Context.material);
		var bpos = Project.brushes.indexOf(Context.brush);

		steps.push({
			name: name,
			layer: lpos,
			object: opos,
			material: mpos,
			brush: bpos,
			is_mask: Context.layerIsMask,
			has_mask: Context.layer.texpaint_mask != null,
			layer_opacity: Context.layer.maskOpacity,
			layer_object: Context.layer.objectMask,
			layer_blending: Context.layer.blending
		});

		while (steps.length > Config.raw.undo_steps + 1) steps.shift();
		return steps[steps.length - 1];
	}

	static function redoMergeLayers() {
		copyMergingLayers();
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

	static function copyToUndo(fromId: Int, toId: Int, isMask: Bool) {
		var path = iron.RenderPath.active;
		if (isMask) {
			path.setTarget("texpaint_mask_undo" + toId);
			path.bindTarget("texpaint_mask" + fromId, "tex");
			path.drawShader("shader_datas/copy_pass/copyR8_pass");
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
}

typedef TStep = {
	public var name: String;
	public var layer: Int;
	public var object: Int;
	public var material: Int;
	public var brush: Int;
	public var is_mask: Bool; // Mask operation
	public var has_mask: Bool; // Layer contains mask
	@:optional public var layer_opacity: Float;
	@:optional public var layer_object: Int;
	@:optional public var layer_blending: Int;
	@:optional public var prev_order: Int; // Previous layer position
	@:optional public var canvas: TNodeCanvas; // Node history
	@:optional public var canvas_type: Int;
	@:optional public var canvas_group: Int;
}
