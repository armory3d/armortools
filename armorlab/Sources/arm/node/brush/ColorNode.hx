package arm.node.brush;

import iron.math.Vec4;

@:keep
class ColorNode extends LogicNode {

	var value = new Vec4();
	var image: kha.Image = null;

	public function new(tree: LogicTree, r = 0.8, g = 0.8, b = 0.8, a = 1.0) {
		super(tree);

		value.set(r, g, b, a);
	}

	override function get(from: Int, done: Dynamic->Void) {
		if (inputs.length > 0) { inputs[0].get(done); return; }
		if (image != null) image.unload();
		var b = haxe.io.Bytes.alloc(16);
		b.setFloat(0, value.x);
		b.setFloat(4, value.y);
		b.setFloat(8, value.z);
		b.setFloat(12, value.w);
		image = kha.Image.fromBytes(b, 1, 1, kha.graphics4.TextureFormat.RGBA128);
		done(image);
	}

	override function set(value: Dynamic) {
		if (inputs.length > 0) inputs[0].set(value);
		else this.value = value;
	}
}
