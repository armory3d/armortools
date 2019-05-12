package arm;

import arm.ui.UITrait;
import arm.ui.UIView2D;

class History {

	public static var stack:Array<String> = ["New"];

	public static function doUndo() {
		if (UITrait.inst.undos > 0) {
			UITrait.inst.undoI = UITrait.inst.undoI - 1 < 0 ? App.C.undo_steps - 1 : UITrait.inst.undoI - 1;
			var lay = UITrait.inst.undoLayers[UITrait.inst.undoI];
			var opos = UITrait.inst.paintObjects.indexOf(lay.targetObject);
			var lpos = UITrait.inst.layers.indexOf(lay.targetLayer);
			if (opos >= 0 && lpos >= 0) {
				UITrait.inst.selectPaintObject(UITrait.inst.paintObjects[opos]);
				UITrait.inst.setLayer(UITrait.inst.layers[lpos], lay.targetIsMask);
				lay.targetIsMask ? UITrait.inst.selectedLayer.swapMask(lay) : UITrait.inst.selectedLayer.swap(lay);
				UITrait.inst.layerPreviewDirty = true;
			}
			UITrait.inst.undos--;
			UITrait.inst.redos++;
			if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
		}
	}

	public static function doRedo() {
		if (UITrait.inst.redos > 0) {
			var lay = UITrait.inst.undoLayers[UITrait.inst.undoI];
			var opos = UITrait.inst.paintObjects.indexOf(lay.targetObject);
			var lpos = UITrait.inst.layers.indexOf(lay.targetLayer);
			if (opos >= 0 && lpos >= 0) {
				UITrait.inst.selectPaintObject(UITrait.inst.paintObjects[opos]);
				UITrait.inst.setLayer(UITrait.inst.layers[lpos], lay.targetIsMask);
				lay.targetIsMask ? UITrait.inst.selectedLayer.swapMask(lay) : UITrait.inst.selectedLayer.swap(lay);
				UITrait.inst.layerPreviewDirty = true;
			}
			UITrait.inst.undoI = (UITrait.inst.undoI + 1) % App.C.undo_steps;
			UITrait.inst.undos++;
			UITrait.inst.redos--;
			if (UIView2D.inst.show) UIView2D.inst.hwnd.redraws = 2;
		}
	}

	public static function reset() {
		stack = ["New"];
	}
}
