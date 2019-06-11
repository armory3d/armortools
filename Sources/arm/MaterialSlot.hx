package arm;

import kha.Image;
import zui.Nodes;
import iron.data.MaterialData;
import arm.util.RenderUtil;

class MaterialSlot {
	public var nodes = new Nodes();
	public var image:Image = null; // 200
	public var imageIcon:Image = null; // 50
	public var previewReady = false;
	public var data:MaterialData;

	static var counter = 0;
	public var id = 0;

	public var paintBase = true;
	public var paintOpac = true;
	public var paintOcc = true;
	public var paintRough = true;
	public var paintMet = true;
	public var paintNor = true;
	public var paintHeight = true;
	public var paintEmis = true;
	public var paintSubs = true;
	
	public function new(m:MaterialData = null) {
		id = ++counter;
		data = m;

		var w = RenderUtil.matPreviewSize;
		var wIcon = Std.int(w / 4);
		image = Image.createRenderTarget(w, w);
		imageIcon = Image.createRenderTarget(wIcon, wIcon);
	}
}
