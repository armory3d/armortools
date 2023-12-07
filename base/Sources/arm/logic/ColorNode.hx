package arm.logic;

import iron.math.Vec4;
import arm.logic.LogicNode;

@:keep
class ColorNode extends LogicNode {

	var value = new Vec4();
	var image: kha.Image = null;

	public function new(tree: LogicTree, r = 0.8, g = 0.8, b = 0.8, a = 1.0) {
		super(tree);
		value.set(r, g, b, a);
	}

	override function get(from: Int, done: Dynamic->Void) {
		if (inputs.length > 0) inputs[0].get(done);
		else done(value);
	}

	override function getAsImage(from: Int, done: kha.Image->Void) {
		if (inputs.length > 0) { inputs[0].getAsImage(done); return; }
		if (image != null) image.unload();
		var b = new js.lib.ArrayBuffer(16);
		var v = new js.lib.DataView(b);
		v.setFloat32(0, value.x, true);
		v.setFloat32(4, value.y, true);
		v.setFloat32(8, value.z, true);
		v.setFloat32(12, value.w, true);
		image = kha.Image.fromBytes(b, 1, 1, kha.Image.TextureFormat.RGBA128);
		done(image);
	}

	override function set(value: Dynamic) {
		if (inputs.length > 0) inputs[0].set(value);
		else this.value = value;
	}
}
