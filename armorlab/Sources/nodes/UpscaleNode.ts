
class UpscaleNode extends LogicNode {

	static temp: image_t = null;
	static image: image_t = null;
	static esrgan_blob: ArrayBuffer;

	constructor() {
		super();
	}

	override getAsImage = (from: i32, done: (img: image_t)=>void) => {
		this.inputs[0].getAsImage((_image: image_t) => {
			UpscaleNode.image = _image;

			Console.progress(tr("Processing") + " - " + tr("Upscale"));
			Base.notifyOnNextFrame(() => {
				UpscaleNode.loadBlob(() => {
					if (UpscaleNode.image.width < Config.getTextureResX()) {
						UpscaleNode.image = UpscaleNode.esrgan(UpscaleNode.image);
						while (UpscaleNode.image.width < Config.getTextureResX()) {
							let lastImage = UpscaleNode.image;
							UpscaleNode.image = UpscaleNode.esrgan(UpscaleNode.image);
							image_unload(lastImage);
						}
					}
					done(UpscaleNode.image);
				});
			});
		});
	}

	static loadBlob = (done: ()=>void) => {
		data_get_blob("models/esrgan.quant.onnx", (_esrgan_blob: ArrayBuffer) => {
			UpscaleNode.esrgan_blob = _esrgan_blob;
			done();
		});
	}

	override getCachedImage = (): image_t => {
		return UpscaleNode.image;
	}

	static doTile = (source: image_t) => {
		let result: image_t = null;
		let size1w = source.width;
		let size1h = source.height;
		let size2w = Math.floor(size1w * 2);
		let size2h = Math.floor(size1h * 2);
		if (UpscaleNode.temp != null) {
			image_unload(UpscaleNode.temp);
		}
		UpscaleNode.temp = image_create_render_target(size1w, size1h);
		g2_begin(UpscaleNode.temp, false);
		g2_draw_scaled_image(source, 0, 0, size1w, size1h);
		g2_end();

		let bytes_img = image_get_pixels(UpscaleNode.temp);
		let u8a = new Uint8Array(bytes_img);
		let f32a = new Float32Array(3 * size1w * size1h);
		for (let i = 0; i < (size1w * size1h); ++i) {
			f32a[i                      ] = (u8a[i * 4    ] / 255);
			f32a[i + size1w * size1w    ] = (u8a[i * 4 + 1] / 255);
			f32a[i + size1w * size1w * 2] = (u8a[i * 4 + 2] / 255);
		}

		let esrgan2x_buf = krom_ml_inference(UpscaleNode.esrgan_blob, [f32a.buffer], [[1, 3, size1w, size1h]], [1, 3, size2w, size2h], Config.raw.gpu_inference);
		let esrgan2x = new Float32Array(esrgan2x_buf);
		for (let i = 0; i < esrgan2x.length; ++i) {
			if (esrgan2x[i] < 0) esrgan2x[i] = 0;
			else if (esrgan2x[i] > 1) esrgan2x[i] = 1;
		}

		u8a = new Uint8Array(4 * size2w * size2h);
		for (let i = 0; i < (size2w * size2h); ++i) {
			u8a[i * 4    ] = Math.floor(esrgan2x[i                      ] * 255);
			u8a[i * 4 + 1] = Math.floor(esrgan2x[i + size2w * size2w    ] * 255);
			u8a[i * 4 + 2] = Math.floor(esrgan2x[i + size2w * size2w * 2] * 255);
			u8a[i * 4 + 3] = 255;
		}

		result = image_from_bytes(u8a.buffer, size2w, size2h);
		return result;
	}

	static esrgan = (source: image_t): image_t => {
		let result: image_t = null;
		let size1w = source.width;
		let size1h = source.height;
		let tileSize = 512;
		let tileSize2x = Math.floor(tileSize * 2);

		if (size1w >= tileSize2x || size1h >= tileSize2x) { // Split into tiles
			let size2w = Math.floor(size1w * 2);
			let size2h = Math.floor(size1h * 2);
			result = image_create_render_target(size2w, size2h);
			let tileSource = image_create_render_target(tileSize + 32 * 2, tileSize + 32 * 2);
			for (let x = 0; x < Math.floor(size1w / tileSize); ++x) {
				for (let y = 0; y < Math.floor(size1h / tileSize); ++y) {
					g2_begin(tileSource, false);
					g2_draw_scaled_image(source, 32 - x * tileSize, 32 - y * tileSize, -source.width, source.height);
					g2_draw_scaled_image(source, 32 - x * tileSize, 32 - y * tileSize, source.width, -source.height);
					g2_draw_scaled_image(source, 32 - x * tileSize, 32 - y * tileSize, -source.width, -source.height);
					g2_draw_scaled_image(source, 32 - x * tileSize + tileSize, 32 - y * tileSize + tileSize, source.width, source.height);
					g2_draw_scaled_image(source, 32 - x * tileSize + tileSize, 32 - y * tileSize + tileSize, -source.width, source.height);
					g2_draw_scaled_image(source, 32 - x * tileSize + tileSize, 32 - y * tileSize + tileSize, source.width, -source.height);
					g2_draw_scaled_image(source, 32 - x * tileSize, 32 - y * tileSize, source.width, source.height);
					g2_end();
					let tileResult = UpscaleNode.doTile(tileSource);
					g2_begin(result, false);
					g2_draw_sub_image(tileResult, x * tileSize2x, y * tileSize2x, 64, 64, tileSize2x, tileSize2x);
					g2_end();
					image_unload(tileResult);
				}
			}
			image_unload(tileSource);
		}
		else result = UpscaleNode.doTile(source); // Single tile
		return result;
	}

	static def: zui_node_t = {
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
