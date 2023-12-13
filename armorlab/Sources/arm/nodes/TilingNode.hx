package arm.nodes;

import zui.Zui.Nodes;
import iron.System;
import arm.LogicNode;
import arm.ParserLogic.f32;
import arm.Translator._tr;

@:keep
class TilingNode extends LogicNode {

	var result: Image = null;
	public static var image: Image = null;
	public static var prompt = "";
	static var strength = 0.5;
	static var auto = true;

	public function new() {
		super();

		init();
	}

	public static function init() {
		if (image == null) {
			image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		}
	}

	public static function buttons(ui: zui.Zui, nodes: zui.Zui.Nodes, node: zui.Zui.TNode) {
		auto = node.buttons[0].default_value == 0 ? false : true;
		if (!auto) {
			strength = ui.slider(zui.Zui.handle("tilingnode_0", {value: strength}), tr("strength"), 0, 1, true);
			prompt = ui.textArea(zui.Zui.handle("tilingnode_1"), true, tr("prompt"), true);
			node.buttons[1].height = 1 + prompt.split("\n").length;
		}
		else node.buttons[1].height = 0;
	}

	override function getAsImage(from: Int, done: Image->Void) {
		inputs[0].getAsImage(function(source: Image) {
			image.g2.begin(false);
			image.g2.drawScaledImage(source, 0, 0, Config.getTextureResX(), Config.getTextureResY());
			image.g2.end();

			Console.progress(tr("Processing") + " - " + tr("Tiling"));
			Base.notifyOnNextFrame(function() {
				function _done(image: Image) {
					result = image;
					done(image);
				}
				auto ? InpaintNode.texsynthInpaint(image, true, null, _done) : sdTiling(image, -1, _done);
			});
		});
	}

	override public function getCachedImage(): Image {
		return result;
	}

	public static function sdTiling(image: Image, seed: Int/* = -1*/, done: Image->Void) {
		TextToPhotoNode.tiling = false;
		var tile = Image.createRenderTarget(512, 512);
		tile.g2.begin(false);
		tile.g2.drawScaledImage(image, -256, -256, 512, 512);
		tile.g2.drawScaledImage(image, 256, -256, 512, 512);
		tile.g2.drawScaledImage(image, -256, 256, 512, 512);
		tile.g2.drawScaledImage(image, 256, 256, 512, 512);
		tile.g2.end();

		var u8 = new js.lib.Uint8Array(512 * 512);
		for (i in 0...512 * 512) {
			var x = i % 512;
			var y = Std.int(i / 512);
			var l = y < 256 ? y : (511 - y);
			u8[i] = (x > 256 - l && x < 256 + l) ? 0 : 255;
		}
		// for (i in 0...512 * 512) u8[i] = 255;
		// for (x in (256 - 32)...(256 + 32)) {
		// 	for (y in 0...512) {
		// 		u8[y * 512 + x] = 0;
		// 	}
		// }
		// for (x in 0...512) {
		// 	for (y in (256 - 32)...(256 + 32)) {
		// 		u8[y * 512 + x] = 0;
		// 	}
		// }
		var mask = Image.fromBytes(u8.buffer, 512, 512, TextureFormat.R8);

		InpaintNode.prompt = prompt;
		InpaintNode.strength = strength;
		if (seed >= 0) RandomNode.setSeed(seed);
		InpaintNode.sdInpaint(tile, mask, done);
	}

	public static var def: zui.Zui.TNode = {
		id: 0,
		name: _tr("Tiling"),
		type: "TilingNode",
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32([0.0, 0.0, 0.0, 1.0])
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32([0.0, 0.0, 0.0, 1.0])
			}
		],
		buttons: [
			{
				name: _tr("auto"),
				type: "BOOL",
				default_value: true,
				output: 0
			},
			{
				name: "arm.nodes.TilingNode.buttons",
				type: "CUSTOM",
				height: 0
			}
		]
	};
}
