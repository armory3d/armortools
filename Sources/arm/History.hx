package arm;

import arm.ui.UITrait;
import arm.ui.UIView2D;
import arm.data.LayerSlot;

class History {

	public static var stack:Array<String> = ["New"];
	public static var undoI = 0; // Undo layer
	public static var undos = 0; // Undos available
	public static var redos = 0; // Redos available
	public static var pushUndo = false; // Store undo on next paint
	public static var undoLayers:Array<LayerSlot> = null;

	public static function doUndo() {
		if (undos > 0) {
			undoI = undoI - 1 < 0 ? App.C.undo_steps - 1 : undoI - 1;
			var lay = undoLayers[undoI];
			var opos = Context.paintObjects.indexOf(lay.targetObject);
			var lpos = Project.layers.indexOf(lay.targetLayer);
			if (opos >= 0 && lpos >= 0) {
				Context.selectPaintObject(Context.paintObjects[opos]);
				Context.setLayer(Project.layers[lpos], lay.targetIsMask);
				lay.targetIsMask ? Context.layer.swapMask(lay) : Context.layer.swap(lay);
				Context.layerPreviewDirty = true;
			}
			undos--;
			redos++;
			if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
		}
	}

	public static function doRedo() {
		if (redos > 0) {
			var lay = undoLayers[undoI];
			var opos = Context.paintObjects.indexOf(lay.targetObject);
			var lpos = Project.layers.indexOf(lay.targetLayer);
			if (opos >= 0 && lpos >= 0) {
				Context.selectPaintObject(Context.paintObjects[opos]);
				Context.setLayer(Project.layers[lpos], lay.targetIsMask);
				lay.targetIsMask ? Context.layer.swapMask(lay) : Context.layer.swap(lay);
				Context.layerPreviewDirty = true;
			}
			undoI = (undoI + 1) % App.C.undo_steps;
			undos++;
			redos--;
			if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
		}
	}

	public static function reset() {
		stack = ["New"];
		undos = 0;
		redos = 0;
		undoI = 0;
	}
}
