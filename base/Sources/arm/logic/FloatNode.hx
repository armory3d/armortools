package arm.logic;

import zui.Nodes;
import arm.logic.LogicNode;
import arm.Translator._tr;

@:keep
class FloatNode extends LogicNode {

	public var value: Float;
	var image: kha.Image = null;

	public function new(tree: LogicTree, value = 0.0) {
		super(tree);
		this.value = value;
	}

	override function get(from: Int, done: Dynamic->Void) {
		if (inputs.length > 0) inputs[0].get(done);
		else done(value);
	}

	override function getAsImage(from: Int, done: kha.Image->Void) {
		if (inputs.length > 0) { inputs[0].getAsImage(done); return; }
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

	public static var def: TNode = {
		id: 0,
		name: _tr("Value"),
		type: "FloatNode",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.5,
				min: 0.0,
				max: 10.0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.5
			}
		],
		buttons: []
	};
}
