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
	static var defaultCanvas: String = null;

	public function new(c: TNodeCanvas = null) {
		for (brush in Project.brushes) if (brush.id >= id) id = brush.id + 1;

		if (c == null) {
			if (defaultCanvas == null) { // Synchronous
				Data.getBlob("defaults/default_brush.json", function(b: Blob) {
					defaultCanvas = b.toString();
				});
			}
			canvas = Json.parse(defaultCanvas);
		}
		else {
			canvas = c;
		}

		canvas.name = "Brush " + (id + 1);
	}
}
