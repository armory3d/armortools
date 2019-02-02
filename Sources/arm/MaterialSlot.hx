package arm;

class MaterialSlot {
	public var nodes = new zui.Nodes();
	public var image:kha.Image = null;
	#if arm_editor
	public var data:iron.data.MaterialData;
	#end
	
	public function new(m:iron.data.MaterialData = null) {
		image = kha.Image.createRenderTarget(100, 100);
		#if arm_editor
		data = m;
		#end
	}
}
