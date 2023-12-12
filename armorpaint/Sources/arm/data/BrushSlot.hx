package arm.data;

import zui.Zui.Nodes;
import zui.Zui.TNodeCanvas;
import iron.System;
import iron.Data;
import iron.ArmPack;

class BrushSlot {
	public var nodes = new Nodes();
	public var canvas: TNodeCanvas;
	public var image: Image = null; // 200px
	public var imageIcon: Image = null; // 50px
	public var previewReady = false;
	public var id = 0;
	static var defaultCanvas: js.lib.ArrayBuffer = null;

	public function new(c: TNodeCanvas = null) {
		for (brush in Project.brushes) if (brush.id >= id) id = brush.id + 1;

		if (c == null) {
			if (defaultCanvas == null) { // Synchronous
				Data.getBlob("default_brush.arm", function(b: js.lib.ArrayBuffer) {
					defaultCanvas = b;
				});
			}
			canvas = ArmPack.decode(defaultCanvas);
			canvas.name = "Brush " + (id + 1);
		}
		else {
			canvas = c;
		}
	}
}
