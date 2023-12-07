package arm.logic;

import zui.Zui.Nodes;
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

		var f32 = new js.lib.Float32Array(4);
		var raw = LogicParser.getRawNode(this);
		var default_value = raw.outputs[0].default_value;
		f32[0] = default_value[0];
		f32[1] = default_value[1];
		f32[2] = default_value[2];
		f32[3] = default_value[3];
		image = kha.Image.fromBytes(f32.buffer, 1, 1, kha.Image.TextureFormat.RGBA128);
		done(image);
	}

	override public function getCachedImage(): kha.Image {
		getAsImage(0, function(img: kha.Image) {});
		return image;
	}

	public static var def: zui.Zui.TNode = {
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
