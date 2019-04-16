package arm;

class MaterialSlot {
	public var nodes = new zui.Nodes();
	public var image:kha.Image = null; // 200
	public var imageIcon:kha.Image = null; // 50
	public var data:iron.data.MaterialData;

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
	
	public function new(m:iron.data.MaterialData = null) {
		id = ++counter;

		var w = arm.util.RenderUtil.matPreviewSize;
		var wIcon = Std.int(w / 4);
		image = kha.Image.createRenderTarget(w, w);
		imageIcon = kha.Image.createRenderTarget(wIcon, wIcon);
		
		data = m;
	}
}
