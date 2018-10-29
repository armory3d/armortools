package arm;

import zui.*;

class MaterialSlot {
	public var nodes = new Nodes();
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
