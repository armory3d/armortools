package arm.nodes;

import zui.Zui.Nodes;
import iron.System;
import iron.Data;
import arm.LogicNode;
import arm.ParserLogic.f32;
import arm.Translator._tr;

@:keep
class UpscaleNode extends LogicNode {

	static var temp: Image = null;
	static var image: Image = null;
	static var esrgan_blob: js.lib.ArrayBuffer;

	public function new() {
		super();
	}

	override function getAsImage(from: Int, done: Image->Void) {
		inputs[0].getAsImage(function(_image: Image) {
			image = _image;

			Console.progress(tr("Processing") + " - " + tr("Upscale"));
			Base.notifyOnNextFrame(function() {
				loadBlob(function() {
					if (image.width < Config.getTextureResX()) {
						image = esrgan(image);
						while (image.width < Config.getTextureResX()) {
							var lastImage = image;
							image = esrgan(image);
							lastImage.unload();
						}
					}
					done(image);
				});
			});
		});
	}

	public static function loadBlob(done: Void->Void) {
		Data.getBlob("models/esrgan.quant.onnx", function(_esrgan_blob: js.lib.ArrayBuffer) {
			esrgan_blob = _esrgan_blob;
			done();
		});
	}

	override public function getCachedImage(): Image {
		return image;
	}

	static function doTile(source: Image) {
		var result: Image = null;
		var size1w = source.width;
		var size1h = source.height;
		var size2w = Std.int(size1w * 2);
		var size2h = Std.int(size1h * 2);
		if (temp != null) {
			temp.unload();
		}
		temp = Image.createRenderTarget(size1w, size1h);
		temp.g2.begin(false);
		temp.g2.drawScaledImage(source, 0, 0, size1w, size1h);
		temp.g2.end();

		var bytes_img = temp.getPixels();
		var u8 = new js.lib.Uint8Array(bytes_img);
		var f32 = new js.lib.Float32Array(3 * size1w * size1h);
		for (i in 0...(size1w * size1h)) {
			f32[i                      ] = (u8[i * 4    ] / 255);
			f32[i + size1w * size1w    ] = (u8[i * 4 + 1] / 255);
			f32[i + size1w * size1w * 2] = (u8[i * 4 + 2] / 255);
		}

		var esrgan2x_buf = Krom.mlInference(esrgan_blob, [f32.buffer], [[1, 3, size1w, size1h]], [1, 3, size2w, size2h], Config.raw.gpu_inference);
		var esrgan2x = new js.lib.Float32Array(esrgan2x_buf);
		for (i in 0...esrgan2x.length) {
			if (esrgan2x[i] < 0) esrgan2x[i] = 0;
			else if (esrgan2x[i] > 1) esrgan2x[i] = 1;
		}

		var u8 = new js.lib.Uint8Array(4 * size2w * size2h);
		for (i in 0...(size2w * size2h)) {
			u8[i * 4    ] = Std.int(esrgan2x[i                      ] * 255);
			u8[i * 4 + 1] = Std.int(esrgan2x[i + size2w * size2w    ] * 255);
			u8[i * 4 + 2] = Std.int(esrgan2x[i + size2w * size2w * 2] * 255);
			u8[i * 4 + 3] = 255;
		}

		result = Image.fromBytes(u8.buffer, size2w, size2h);
		return result;
	}

	public static function esrgan(source: Image): Image {
		var result: Image = null;
		var size1w = source.width;
		var size1h = source.height;
		var tileSize = 512;
		var tileSize2x = Std.int(tileSize * 2);

		if (size1w >= tileSize2x || size1h >= tileSize2x) { // Split into tiles
			var size2w = Std.int(size1w * 2);
			var size2h = Std.int(size1h * 2);
			result = Image.createRenderTarget(size2w, size2h);
			var tileSource = Image.createRenderTarget(tileSize + 32 * 2, tileSize + 32 * 2);
			for (x in 0...Std.int(size1w / tileSize)) {
				for (y in 0...Std.int(size1h / tileSize)) {
					tileSource.g2.begin(false);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize, 32 - y * tileSize, -source.width, source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize, 32 - y * tileSize, source.width, -source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize, 32 - y * tileSize, -source.width, -source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize + tileSize, 32 - y * tileSize + tileSize, source.width, source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize + tileSize, 32 - y * tileSize + tileSize, -source.width, source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize + tileSize, 32 - y * tileSize + tileSize, source.width, -source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize, 32 - y * tileSize, source.width, source.height);
					tileSource.g2.end();
					var tileResult = doTile(tileSource);
					result.g2.begin(false);
					result.g2.drawSubImage(tileResult, x * tileSize2x, y * tileSize2x, 64, 64, tileSize2x, tileSize2x);
					result.g2.end();
					tileResult.unload();
				}
			}
			tileSource.unload();
		}
		else result = doTile(source); // Single tile
		return result;
	}

	public static var def: zui.Zui.TNode = {
		id: 0,
		name: _tr("Upscale"),
		type: "UpscaleNode",
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
		buttons: []
	};
}
