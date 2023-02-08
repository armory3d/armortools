package arm.node.brush;

@:keep
class FloatNode extends LogicNode {

	public var value: Float;
	var image: kha.Image = null;

	public function new(tree: LogicTree, value = 0.0) {
		super(tree);
		this.value = value;
	}

	override function get(from: Int, done: Dynamic->Void) {
		if (inputs.length > 0) { inputs[0].get(done); return; }
		if (image != null) image.unload();
		var b = haxe.io.Bytes.alloc(16);
		b.setFloat(0, value);
		b.setFloat(4, value);
		b.setFloat(8, value);
		b.setFloat(12, 1.0);
		image = kha.Image.fromBytes(b, 1, 1, kha.graphics4.TextureFormat.RGBA128);
		done(image);
	}

	override function set(value: Dynamic) {
		if (inputs.length > 0) inputs[0].set(value);
		else this.value = value;
	}
}
