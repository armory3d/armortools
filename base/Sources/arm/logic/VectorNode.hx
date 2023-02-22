package arm.logic;

import iron.math.Vec4;
import zui.Nodes;
import arm.logic.LogicNode;
import arm.logic.LogicParser.f32;
import arm.Translator._tr;

@:keep
class VectorNode extends LogicNode {

	var value = new Vec4();
	var image: kha.Image = null;

	public function new(tree: LogicTree, x: Null<Float> = null, y: Null<Float> = null, z: Null<Float> = null) {
		super(tree);

		if (x != null) {
			addInput(new FloatNode(tree, x), 0);
			addInput(new FloatNode(tree, y), 0);
			addInput(new FloatNode(tree, z), 0);
		}
	}

	override function get(from: Int, done: Dynamic->Void) {
		inputs[0].get(function(x: Float) {
			inputs[1].get(function(y: Float) {
				inputs[2].get(function(z: Float) {
					value.x = x;
					value.y = y;
					value.z = z;
					done(value);
				});
			});
		});
	}

	override function getAsImage(from: Int, done: kha.Image->Void) {
		inputs[0].get(function(x: Float) {
			inputs[1].get(function(y: Float) {
				inputs[2].get(function(z: Float) {
					if (image != null) image.unload();
					var b = haxe.io.Bytes.alloc(16);
					b.setFloat(0, untyped inputs[0].node.value);
					b.setFloat(4, untyped inputs[1].node.value);
					b.setFloat(8, untyped inputs[2].node.value);
					b.setFloat(12, 1.0);
					image = kha.Image.fromBytes(b, 1, 1, kha.graphics4.TextureFormat.RGBA128);
					done(image);
				});
			});
		});
	}

	override function set(value: Dynamic) {
		inputs[0].set(value.x);
		inputs[1].set(value.y);
		inputs[2].set(value.z);
	}

	public static var def: TNode = {
		id: 0,
		name: _tr("Vector"),
		type: "VectorNode",
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("X"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Y"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Z"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Vector"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: f32([0.0, 0.0, 0.0])
			}
		],
		buttons: []
	};
}
