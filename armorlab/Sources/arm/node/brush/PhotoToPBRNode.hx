package arm.node.brush;

import arm.Enums;

@:keep
class PhotoToPBRNode extends LogicNode {

	static var temp: kha.Image = null;
	static var images: Array<kha.Image> = null;
	static var modelNames = ["base", "occlusion", "roughness", "metallic", "normal", "height"];

	public static var cachedSource: Dynamic = null;

	public function new(tree: LogicTree) {
		super(tree);

		if (temp == null) {
			temp = kha.Image.createRenderTarget(2176, 2176);
		}

		init();
	}

	public static function init() {
		if (images == null) {
			images = [];
			for (i in 0...modelNames.length) {
				images.push(kha.Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY()));
			}
		}
	}

	override function get(from: Int, done: Dynamic->Void) {
		function getSource(done: Dynamic->Void) {
			if (cachedSource != null) done(cachedSource);
			else inputs[0].get(done);
		}

		getSource(function(source: kha.Image) {
			cachedSource = source;

			if (!Std.isOfType(source, kha.Image)) { done(null); return; }

			var tilesX = Std.int(Config.getTextureResX() / 2048);
			var tilesY = Std.int(Config.getTextureResY() / 2048);
			for (i in 0...(tilesX * tilesY)) {
				var x = i % tilesX;
				var y = Std.int(i / tilesX);

				temp.g2.begin(false);
				temp.g2.drawScaledImage(source, 64 - x * 2048, 64 - y * 2048, -Config.getTextureResX(), Config.getTextureResY());
				temp.g2.drawScaledImage(source, 64 - x * 2048, 64 - y * 2048, Config.getTextureResX(), -Config.getTextureResY());
				temp.g2.drawScaledImage(source, 64 - x * 2048, 64 - y * 2048, -Config.getTextureResX(), -Config.getTextureResY());
				temp.g2.drawScaledImage(source, 64 - x * 2048 + 2048, 64 - y * 2048 + 2048, Config.getTextureResX(), Config.getTextureResY());
				temp.g2.drawScaledImage(source, 64 - x * 2048 + 2048, 64 - y * 2048 + 2048, -Config.getTextureResX(), Config.getTextureResY());
				temp.g2.drawScaledImage(source, 64 - x * 2048 + 2048, 64 - y * 2048 + 2048, Config.getTextureResX(), -Config.getTextureResY());
				temp.g2.drawScaledImage(source, 64 - x * 2048, 64 - y * 2048, Config.getTextureResX(), Config.getTextureResY());
				temp.g2.end();

				var bytes_img = untyped temp.getPixels().b.buffer;
				var u8 = new js.lib.Uint8Array(untyped bytes_img);
				var f32 = new js.lib.Float32Array(3 * 2176 * 2176);
				for (i in 0...(2176 * 2176)) {
					f32[i                  ] = (u8[i * 4    ] / 255 - 0.5) / 0.5;
					f32[i + 2176 * 2176    ] = (u8[i * 4 + 1] / 255 - 0.5) / 0.5;
					f32[i + 2176 * 2176 * 2] = (u8[i * 4 + 2] / 255 - 0.5) / 0.5;
				}

				kha.Assets.loadBlobFromPath("data/models/photo_to_" + modelNames[from] + ".quant.onnx", function(model_blob: kha.Blob) {
					var buf = Krom.mlInference(untyped model_blob.toBytes().b.buffer, [f32.buffer], null, null, Config.raw.gpu_inference);
					var ar = new js.lib.Float32Array(buf);
					var bytes = haxe.io.Bytes.alloc(4 * 2048 * 2048);
					var offsetG = (from == ChannelBaseColor || from == ChannelNormalMap) ? 2176 * 2176 : 0;
					var offsetB = (from == ChannelBaseColor || from == ChannelNormalMap) ? 2176 * 2176 * 2 : 0;
					for (i in 0...(2048 * 2048)) {
						var x = 64 + i % 2048;
						var y = 64 + Std.int(i / 2048);
						bytes.set(i * 4    , Std.int((ar[y * 2176 + x          ] * 0.5 + 0.5) * 255));
						bytes.set(i * 4 + 1, Std.int((ar[y * 2176 + x + offsetG] * 0.5 + 0.5) * 255));
						bytes.set(i * 4 + 2, Std.int((ar[y * 2176 + x + offsetB] * 0.5 + 0.5) * 255));
						bytes.set(i * 4 + 3, 255);
					}

					#if (kha_metal || kha_vulkan)
					if (from == ChannelBaseColor) bgraSwap(bytes);
					#end

					var temp2 = kha.Image.fromBytes(bytes, 2048, 2048);
					images[from].g2.begin(false);
					images[from].g2.drawImage(temp2, x * 2048, y * 2048);
					images[from].g2.end();
					App.notifyOnNextFrame(function() {
						temp2.unload();
					});
				});
			}

			done(images[from]);
		});
	}

	#if (kha_metal || kha_vulkan)
	static function bgraSwap(bytes: haxe.io.Bytes) {
		for (i in 0...Std.int(bytes.length / 4)) {
			var r = bytes.get(i * 4);
			bytes.set(i * 4, bytes.get(i * 4 + 2));
			bytes.set(i * 4 + 2, r);
		}
		return bytes;
	}
	#end
}
