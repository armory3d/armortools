package arm;

class MaterialSlot {
	public var nodes = new zui.Nodes();
	public var image:kha.Image = null;
	public var data:iron.data.MaterialData;

	static var counter = 0;
	public var id = 0;

	public var paintBase = true;
	public var paintOpac = true;
	public var paintOcc = true;
	public var paintRough = true;
	public var paintMet = true;
	public var paintNor = true;
	public var paintHeight = false;
	public var paintEmis = false;
	public var paintSubs = false;
	
	public function new(m:iron.data.MaterialData = null) {
		id = ++counter;
		image = kha.Image.createRenderTarget(200, 200);
		data = m;
	}
}
