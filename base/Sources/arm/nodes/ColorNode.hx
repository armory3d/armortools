package arm.nodes;

import iron.Vec4;
import iron.System;
import arm.LogicNode;

@:keep
class ColorNode extends LogicNode {

	var value = new Vec4();
	var image: Image = null;

	public function new(r = 0.8, g = 0.8, b = 0.8, a = 1.0) {
		super();
		value.set(r, g, b, a);
	}

	override function get(from: Int, done: Dynamic->Void) {
		if (inputs.length > 0) inputs[0].get(done);
		else done(value);
	}

	override function getAsImage(from: Int, done: Image->Void) {
		if (inputs.length > 0) { inputs[0].getAsImage(done); return; }
		if (image != null) image.unload();
		var b = new js.lib.ArrayBuffer(16);
		var v = new js.lib.DataView(b);
		v.setFloat32(0, value.x, true);
		v.setFloat32(4, value.y, true);
		v.setFloat32(8, value.z, true);
		v.setFloat32(12, value.w, true);
		image = Image.fromBytes(b, 1, 1, TextureFormat.RGBA128);
		done(image);
	}

	override function set(value: Dynamic) {
		if (inputs.length > 0) inputs[0].set(value);
		else this.value = value;
	}
}
