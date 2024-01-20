
// @:keep
class UpscaleNode extends LogicNode {

	static temp: Image = null;
	static image: Image = null;
	static esrgan_blob: ArrayBuffer;

	constructor() {
		super();
	}

	override getAsImage = (from: i32, done: (img: Image)=>void) => {
		inputs[0].getAsImage((_image: Image) => {
			image = _image;

			Console.progress(tr("Processing") + " - " + tr("Upscale"));
			Base.notifyOnNextFrame(() => {
				loadBlob(() => {
					if (image.width < Config.getTextureResX()) {
						image = esrgan(image);
						while (image.width < Config.getTextureResX()) {
							let lastImage = image;
							image = esrgan(image);
							lastImage.unload();
						}
					}
					done(image);
				});
			});
		});
	}

	static loadBlob = (done: ()=>void) => {
		Data.getBlob("models/esrgan.quant.onnx", (_esrgan_blob: ArrayBuffer) => {
			esrgan_blob = _esrgan_blob;
			done();
		});
	}

	override getCachedImage = (): Image => {
		return image;
	}

	static doTile = (source: Image) => {
		let result: Image = null;
		let size1w = source.width;
		let size1h = source.height;
		let size2w = Math.floor(size1w * 2);
		let size2h = Math.floor(size1h * 2);
		if (temp != null) {
			temp.unload();
		}
		temp = Image.createRenderTarget(size1w, size1h);
		temp.g2.begin(false);
		temp.g2.drawScaledImage(source, 0, 0, size1w, size1h);
		temp.g2.end();

		let bytes_img = temp.getPixels();
		let u8a = new Uint8Array(bytes_img);
		let f32a = new Float32Array(3 * size1w * size1h);
		for (let i = 0; i < (size1w * size1h); ++i) {
			f32a[i                      ] = (u8a[i * 4    ] / 255);
			f32a[i + size1w * size1w    ] = (u8a[i * 4 + 1] / 255);
			f32a[i + size1w * size1w * 2] = (u8a[i * 4 + 2] / 255);
		}

		let esrgan2x_buf = Krom.mlInference(esrgan_blob, [f32a.buffer], [[1, 3, size1w, size1h]], [1, 3, size2w, size2h], Config.raw.gpu_inference);
		let esrgan2x = new Float32Array(esrgan2x_buf);
		for (let i = 0; i < esrgan2x.length; ++i) {
			if (esrgan2x[i] < 0) esrgan2x[i] = 0;
			else if (esrgan2x[i] > 1) esrgan2x[i] = 1;
		}

		let u8a = new Uint8Array(4 * size2w * size2h);
		for (let i = 0; i < (size2w * size2h); ++i) {
			u8a[i * 4    ] = Math.floor(esrgan2x[i                      ] * 255);
			u8a[i * 4 + 1] = Math.floor(esrgan2x[i + size2w * size2w    ] * 255);
			u8a[i * 4 + 2] = Math.floor(esrgan2x[i + size2w * size2w * 2] * 255);
			u8a[i * 4 + 3] = 255;
		}

		result = Image.fromBytes(u8a.buffer, size2w, size2h);
		return result;
	}

	static esrgan = (source: Image): Image => {
		let result: Image = null;
		let size1w = source.width;
		let size1h = source.height;
		let tileSize = 512;
		let tileSize2x = Math.floor(tileSize * 2);

		if (size1w >= tileSize2x || size1h >= tileSize2x) { // Split into tiles
			let size2w = Math.floor(size1w * 2);
			let size2h = Math.floor(size1h * 2);
			result = Image.createRenderTarget(size2w, size2h);
			let tileSource = Image.createRenderTarget(tileSize + 32 * 2, tileSize + 32 * 2);
			for (let x = 0; x < Math.floor(size1w / tileSize); ++x) {
				for (let y = 0; y < Math.floor(size1h / tileSize); ++y) {
					tileSource.g2.begin(false);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize, 32 - y * tileSize, -source.width, source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize, 32 - y * tileSize, source.width, -source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize, 32 - y * tileSize, -source.width, -source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize + tileSize, 32 - y * tileSize + tileSize, source.width, source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize + tileSize, 32 - y * tileSize + tileSize, -source.width, source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize + tileSize, 32 - y * tileSize + tileSize, source.width, -source.height);
					tileSource.g2.drawScaledImage(source, 32 - x * tileSize, 32 - y * tileSize, source.width, source.height);
					tileSource.g2.end();
					let tileResult = doTile(tileSource);
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

	static def: zui.Zui.TNode = {
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
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
			}
		],
		buttons: []
	};
}
