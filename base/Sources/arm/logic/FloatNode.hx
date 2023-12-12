package arm.logic;

import iron.System;
import zui.Zui.Nodes;
import zui.Zui.TNode;
import arm.logic.LogicNode;
import arm.Translator._tr;

@:keep
class FloatNode extends LogicNode {

	public var value: Float;
	var image: Image = null;

	public function new(value = 0.0) {
		super();
		this.value = value;
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
		v.setFloat32(0, value, true);
		v.setFloat32(4, value, true);
		v.setFloat32(8, value, true);
		v.setFloat32(12, 1.0, true);
		image = Image.fromBytes(b, 1, 1, TextureFormat.RGBA128);
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
