package arm.logic;

import zui.Nodes;
import iron.math.Vec4;
import arm.logic.LogicNode;
import arm.logic.LogicParser.f32;
import arm.Translator._tr;

@:keep
class RGBNode extends LogicNode {

	var image: kha.Image = null;

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function getAsImage(from: Int, done: kha.Image->Void) {
		if (image != null) {
			App.notifyOnNextFrame(function() {
				image.unload();
			});
		}

		var b = haxe.io.Bytes.alloc(16);
		var raw = LogicParser.getRawNode(this);
		var default_value = raw.outputs[0].default_value;
		b.setFloat(0, default_value[0]);
		b.setFloat(4, default_value[1]);
		b.setFloat(8, default_value[2]);
		b.setFloat(12, default_value[3]);
		image = kha.Image.fromBytes(b, 1, 1, kha.graphics4.TextureFormat.RGBA128);
		done(image);
	}

	override public function getCachedImage(): kha.Image {
		getAsImage(0, function(img: kha.Image) {});
		return image;
	}

	public static var def: TNode = {
		id: 0,
		name: _tr("RGB"),
		type: "RGBNode",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32([0.8, 0.8, 0.8, 1.0])
			}
		],
		buttons: [
			{
				name: _tr("default_value"),
				type: "RGBA",
				output: 0,
				default_value: f32([0.8, 0.8, 0.8, 1.0])
			}
		]
	};
}
