package arm;

class MaterialSlot {
	public var nodes = new zui.Nodes();
	public var image:kha.Image = null;
	public var data:iron.data.MaterialData;

	static var counter = 0;
	public var id = 0;
	
	public function new(m:iron.data.MaterialData = null) {
		id = ++counter;
		image = kha.Image.createRenderTarget(100, 100);
		data = m;
	}
}
