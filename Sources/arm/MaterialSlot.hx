package arm;

class MaterialSlot {
	public var nodes = new zui.Nodes();
	public var image:kha.Image = null;
	
	public var data:iron.data.MaterialData;
	
	public function new(m:iron.data.MaterialData = null) {
		image = kha.Image.createRenderTarget(100, 100);
		data = m;
	}
}
