package arm.node.brush;

@:keep
class UpscaleNode extends LogicNode {

	static var temp: kha.Image = null;
	static var image: kha.Image = null;

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function get(from: Int, done: Dynamic->Void) {
		inputs[0].get(function(img: kha.Image) {
			image = img;

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
	}

	override public function getImage(): kha.Image {
		return image;
	}

	public static function esrgan(source: kha.Image): kha.Image {
		function doTile(source: kha.Image) {
			var result: kha.Image = null;
			var size1w = source.width;
			var size1h = source.height;
			var size2w = Std.int(size1w * 2);
			var size2h = Std.int(size1h * 2);
			if (temp != null) {
				temp.unload();
			}
			temp = kha.Image.createRenderTarget(size1w, size1h);
			temp.g2.begin(false);
			temp.g2.drawScaledImage(source, 0, 0, size1w, size1h);
			temp.g2.end();

			var bytes_img = untyped temp.getPixels().b.buffer;
			var u8 = new js.lib.Uint8Array(untyped bytes_img);
			var f32 = new js.lib.Float32Array(3 * size1w * size1h);
			for (i in 0...(size1w * size1h)) {
				f32[i                      ] = (u8[i * 4    ] / 255);
				f32[i + size1w * size1w    ] = (u8[i * 4 + 1] / 255);
				f32[i + size1w * size1w * 2] = (u8[i * 4 + 2] / 255);
			}

			kha.Assets.loadBlobFromPath("data/models/esrgan.quant.onnx", function(esrgan_blob: kha.Blob) {
				var esrgan2x_buf = Krom.mlInference(untyped esrgan_blob.toBytes().b.buffer, [f32.buffer], [[1, 3, size1w, size1h]], [1, 3, size2w, size2h], Config.raw.gpu_inference, true);
				var esrgan2x = new js.lib.Float32Array(esrgan2x_buf);
				for (i in 0...esrgan2x.length) {
					if (esrgan2x[i] < 0) esrgan2x[i] = 0;
					else if (esrgan2x[i] > 1) esrgan2x[i] = 1;
				}

				var bytes = haxe.io.Bytes.alloc(4 * size2w * size2h);
				for (i in 0...(size2w * size2h)) {
					bytes.set(i * 4    , Std.int(esrgan2x[i                      ] * 255));
					bytes.set(i * 4 + 1, Std.int(esrgan2x[i + size2w * size2w    ] * 255));
					bytes.set(i * 4 + 2, Std.int(esrgan2x[i + size2w * size2w * 2] * 255));
					bytes.set(i * 4 + 3, 255);
				}

				result = kha.Image.fromBytes(bytes, size2w, size2h);
			});

			return result;
		}

		var result: kha.Image = null;
		var size1w = source.width;
		var size1h = source.height;
		var size2w = Std.int(size1w * 2);
		var size2h = Std.int(size1h * 2);
		var tileSize = 512;
		// var tileSize = 1024;
		var tileSize2x = Std.int(tileSize * 2);

		if (size1w >= tileSize2x || size1h >= tileSize2x) { // Split into tiles
			result = kha.Image.createRenderTarget(size2w, size2h);
			var tileSource = kha.Image.createRenderTarget(tileSize, tileSize);
			for (x in 0...Std.int(size1w / tileSize)) {
				for (y in 0...Std.int(size1h / tileSize)) {
					tileSource.g2.begin(false);
					tileSource.g2.drawImage(source, -x * tileSize, -y * tileSize);
					tileSource.g2.end();
					var tileResult = doTile(tileSource);
					result.g2.begin(false);
					result.g2.drawImage(tileResult, x * tileSize2x, y * tileSize2x);
					result.g2.end();
					tileResult.unload();
				}
			}
			tileSource.unload();
		}
		else result = doTile(source); // Single tile
		return result;
	}
}
