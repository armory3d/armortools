package arm.data;

import haxe.Json;
import kha.Image;
import kha.Blob;
import zui.Nodes;
import iron.data.MaterialData;
import iron.data.Data;
import arm.util.RenderUtil;

class MaterialSlot {
	public var nodes = new Nodes();
	public var canvas: TNodeCanvas;
	public var image: Image = null; // 200px
	public var imageIcon: Image = null; // 50px
	public var previewReady = false;
	public var data: MaterialData;
	public var id = 0;
	static var defaultCanvas: String = null;

	public var paintBase = true;
	public var paintOpac = true;
	public var paintOcc = true;
	public var paintRough = true;
	public var paintMet = true;
	public var paintNor = true;
	public var paintHeight = true;
	public var paintEmis = true;
	public var paintSubs = true;

	public function new(m: MaterialData = null, c: TNodeCanvas = null) {
		for (mat in Project.materials) if (mat.id >= id) id = mat.id + 1;
		data = m;

		var w = RenderUtil.matPreviewSize;
		var wIcon = Std.int(w / 4);
		image = Image.createRenderTarget(w, w);
		imageIcon = Image.createRenderTarget(wIcon, wIcon);

		if (c == null) {
			if (defaultCanvas == null) { // Synchronous
				Data.getBlob("defaults/default_material.json", function(b: Blob) {
					defaultCanvas = b.toString();
				});
			}
			canvas = Json.parse(defaultCanvas);
		}
		else {
			canvas = c;
		}

		canvas.name = "Material " + (id + 1);
	}

	public function unload() {
		image.unload();
		imageIcon.unload();
	}
}
