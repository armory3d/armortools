package arm.data;

import haxe.Json;
import kha.Image;
import kha.Blob;
import zui.Nodes;
import iron.data.Data;

class BrushSlot {
	public var nodes = new Nodes();
	public var canvas: TNodeCanvas;
	public var image: Image = null; // 200px
	public var imageIcon: Image = null; // 50px
	public var previewReady = false;
	public var id = 0;
	static var defaultCanvas: Blob = null;

	public function new(c: TNodeCanvas = null) {
		for (brush in Project.brushes) if (brush.id >= id) id = brush.id + 1;

		if (c == null) {
			if (defaultCanvas == null) { // Synchronous
				Data.getBlob("default_brush.arm", function(b: Blob) {
					defaultCanvas = b;
				});
			}
			canvas = iron.system.ArmPack.decode(defaultCanvas.toBytes());
			canvas.name = "Brush " + (id + 1);
		}
		else {
			canvas = c;
		}
	}
}
