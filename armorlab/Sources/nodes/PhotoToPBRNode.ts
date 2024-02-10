
class PhotoToPBRNode extends LogicNode {

	static temp: image_t = null;
	static images: image_t[] = null;
	static modelNames = ["base", "occlusion", "roughness", "metallic", "normal", "height"];

	static cachedSource: image_t = null;
	static borderW = 64;
	static tileW = 2048;
	static tileWithBorderW = PhotoToPBRNode.tileW + PhotoToPBRNode.borderW * 2;

	constructor() {
		super();

		if (PhotoToPBRNode.temp == null) {
			PhotoToPBRNode.temp = image_create_render_target(PhotoToPBRNode.tileWithBorderW, PhotoToPBRNode.tileWithBorderW);
		}

		PhotoToPBRNode.init();
	}

	static init = () => {
		if (PhotoToPBRNode.images == null) {
			PhotoToPBRNode.images = [];
			for (let i = 0; i < PhotoToPBRNode.modelNames.length; ++i) {
				PhotoToPBRNode.images.push(image_create_render_target(Config.getTextureResX(), Config.getTextureResY()));
			}
		}
	}

	override getAsImage = (from: i32, done: (img: image_t)=>void) => {
		let getSource = (done: (img: image_t)=>void) => {
			if (PhotoToPBRNode.cachedSource != null) done(PhotoToPBRNode.cachedSource);
			else this.inputs[0].getAsImage(done);
		}

		getSource((source: image_t) => {
			PhotoToPBRNode.cachedSource = source;

			Console.progress(tr("Processing") + " - " + tr("Photo to PBR"));
			Base.notifyOnNextFrame(() => {
				let tileFloats: Float32Array[] = [];
				let tilesX = Math.floor(Config.getTextureResX() / PhotoToPBRNode.tileW);
				let tilesY = Math.floor(Config.getTextureResY() / PhotoToPBRNode.tileW);
				let numTiles = tilesX * tilesY;
				for (let i = 0; i < numTiles; ++i) {
					let x = i % tilesX;
					let y = Math.floor(i / tilesX);

					g2_begin(PhotoToPBRNode.temp, false);
					g2_draw_scaled_image(source, PhotoToPBRNode.borderW - x * PhotoToPBRNode.tileW, PhotoToPBRNode.borderW - y * PhotoToPBRNode.tileW, -Config.getTextureResX(), Config.getTextureResY());
					g2_draw_scaled_image(source, PhotoToPBRNode.borderW - x * PhotoToPBRNode.tileW, PhotoToPBRNode.borderW - y * PhotoToPBRNode.tileW, Config.getTextureResX(), -Config.getTextureResY());
					g2_draw_scaled_image(source, PhotoToPBRNode.borderW - x * PhotoToPBRNode.tileW, PhotoToPBRNode.borderW - y * PhotoToPBRNode.tileW, -Config.getTextureResX(), -Config.getTextureResY());
					g2_draw_scaled_image(source, PhotoToPBRNode.borderW - x * PhotoToPBRNode.tileW + PhotoToPBRNode.tileW, PhotoToPBRNode.borderW - y * PhotoToPBRNode.tileW + PhotoToPBRNode.tileW, Config.getTextureResX(), Config.getTextureResY());
					g2_draw_scaled_image(source, PhotoToPBRNode.borderW - x * PhotoToPBRNode.tileW + PhotoToPBRNode.tileW, PhotoToPBRNode.borderW - y * PhotoToPBRNode.tileW + PhotoToPBRNode.tileW, -Config.getTextureResX(), Config.getTextureResY());
					g2_draw_scaled_image(source, PhotoToPBRNode.borderW - x * PhotoToPBRNode.tileW + PhotoToPBRNode.tileW, PhotoToPBRNode.borderW - y * PhotoToPBRNode.tileW + PhotoToPBRNode.tileW, Config.getTextureResX(), -Config.getTextureResY());
					g2_draw_scaled_image(source, PhotoToPBRNode.borderW - x * PhotoToPBRNode.tileW, PhotoToPBRNode.borderW - y * PhotoToPBRNode.tileW, Config.getTextureResX(), Config.getTextureResY());
					g2_end();

					let bytes_img = image_get_pixels(PhotoToPBRNode.temp);
					let u8a = new Uint8Array(bytes_img);
					let f32a = new Float32Array(3 * PhotoToPBRNode.tileWithBorderW * PhotoToPBRNode.tileWithBorderW);
					for (let i = 0; i < (PhotoToPBRNode.tileWithBorderW * PhotoToPBRNode.tileWithBorderW); ++i) {
						f32a[i                                        ] = (u8a[i * 4    ] / 255 - 0.5) / 0.5;
						f32a[i + PhotoToPBRNode.tileWithBorderW * PhotoToPBRNode.tileWithBorderW    ] = (u8a[i * 4 + 1] / 255 - 0.5) / 0.5;
						f32a[i + PhotoToPBRNode.tileWithBorderW * PhotoToPBRNode.tileWithBorderW * 2] = (u8a[i * 4 + 2] / 255 - 0.5) / 0.5;
					}

					data_get_blob("models/photo_to_" + PhotoToPBRNode.modelNames[from] + ".quant.onnx", (model_blob: ArrayBuffer) => {
						let buf = krom_ml_inference(model_blob, [f32a.buffer], null, null, Config.raw.gpu_inference);
						let ar = new Float32Array(buf);
						let u8a = new Uint8Array(4 * PhotoToPBRNode.tileW * PhotoToPBRNode.tileW);
						let offsetG = (from == ChannelType.ChannelBaseColor || from == ChannelType.ChannelNormalMap) ? PhotoToPBRNode.tileWithBorderW * PhotoToPBRNode.tileWithBorderW : 0;
						let offsetB = (from == ChannelType.ChannelBaseColor || from == ChannelType.ChannelNormalMap) ? PhotoToPBRNode.tileWithBorderW * PhotoToPBRNode.tileWithBorderW * 2 : 0;
						for (let i = 0; i < (PhotoToPBRNode.tileW * PhotoToPBRNode.tileW); ++i) {
							let x = PhotoToPBRNode.borderW + i % PhotoToPBRNode.tileW;
							let y = PhotoToPBRNode.borderW + Math.floor(i / PhotoToPBRNode.tileW);
							u8a[i * 4    ] = Math.floor((ar[y * PhotoToPBRNode.tileWithBorderW + x          ] * 0.5 + 0.5) * 255);
							u8a[i * 4 + 1] = Math.floor((ar[y * PhotoToPBRNode.tileWithBorderW + x + offsetG] * 0.5 + 0.5) * 255);
							u8a[i * 4 + 2] = Math.floor((ar[y * PhotoToPBRNode.tileWithBorderW + x + offsetB] * 0.5 + 0.5) * 255);
							u8a[i * 4 + 3] = 255;
						}
						tileFloats.push(ar);

						// Use border pixels to blend seams
						if (i > 0) {
							if (x > 0) {
								let ar = tileFloats[i - 1];
								for (let yy = 0; yy < PhotoToPBRNode.tileW; ++yy) {
									for (let xx = 0; xx < PhotoToPBRNode.borderW; ++xx) {
										let i = yy * PhotoToPBRNode.tileW + xx;
										let a = u8a[i * 4];
										let b = u8a[i * 4 + 1];
										let c = u8a[i * 4 + 2];

										let aa = Math.floor((ar[(PhotoToPBRNode.borderW + yy) * PhotoToPBRNode.tileWithBorderW + PhotoToPBRNode.borderW + PhotoToPBRNode.tileW + xx          ] * 0.5 + 0.5) * 255);
										let bb = Math.floor((ar[(PhotoToPBRNode.borderW + yy) * PhotoToPBRNode.tileWithBorderW + PhotoToPBRNode.borderW + PhotoToPBRNode.tileW + xx + offsetG] * 0.5 + 0.5) * 255);
										let cc = Math.floor((ar[(PhotoToPBRNode.borderW + yy) * PhotoToPBRNode.tileWithBorderW + PhotoToPBRNode.borderW + PhotoToPBRNode.tileW + xx + offsetB] * 0.5 + 0.5) * 255);

										let f = xx / PhotoToPBRNode.borderW;
										let invf = 1.0 - f;
										a = Math.floor(a * f + aa * invf);
										b = Math.floor(b * f + bb * invf);
										c = Math.floor(c * f + cc * invf);

										u8a[i * 4    ] = a;
										u8a[i * 4 + 1] = b;
										u8a[i * 4 + 2] = c;
									}
								}
							}
							if (y > 0) {
								let ar = tileFloats[i - tilesX];
								for (let xx = 0; xx < PhotoToPBRNode.tileW; ++xx) {
									for (let yy = 0; yy < PhotoToPBRNode.borderW; ++yy) {
										let i = yy * PhotoToPBRNode.tileW + xx;
										let a = u8a[i * 4];
										let b = u8a[i * 4 + 1];
										let c = u8a[i * 4 + 2];

										let aa = Math.floor((ar[(PhotoToPBRNode.borderW + PhotoToPBRNode.tileW + yy) * PhotoToPBRNode.tileWithBorderW + PhotoToPBRNode.borderW + xx          ] * 0.5 + 0.5) * 255);
										let bb = Math.floor((ar[(PhotoToPBRNode.borderW + PhotoToPBRNode.tileW + yy) * PhotoToPBRNode.tileWithBorderW + PhotoToPBRNode.borderW + xx + offsetG] * 0.5 + 0.5) * 255);
										let cc = Math.floor((ar[(PhotoToPBRNode.borderW + PhotoToPBRNode.tileW + yy) * PhotoToPBRNode.tileWithBorderW + PhotoToPBRNode.borderW + xx + offsetB] * 0.5 + 0.5) * 255);

										let f = yy / PhotoToPBRNode.borderW;
										let invf = 1.0 - f;
										a = Math.floor(a * f + aa * invf);
										b = Math.floor(b * f + bb * invf);
										c = Math.floor(c * f + cc * invf);

										u8a[i * 4    ] = a;
										u8a[i * 4 + 1] = b;
										u8a[i * 4 + 2] = c;
									}
								}
							}
						}

						///if (krom_metal || krom_vulkan)
						if (from == ChannelType.ChannelBaseColor) PhotoToPBRNode.bgraSwap(u8a.buffer);
						///end

						let temp2 = image_from_bytes(u8a.buffer, PhotoToPBRNode.tileW, PhotoToPBRNode.tileW);
						g2_begin(PhotoToPBRNode.images[from], false);
						g2_draw_image(temp2, x * PhotoToPBRNode.tileW, y * PhotoToPBRNode.tileW);
						g2_end();
						Base.notifyOnNextFrame(() => {
							image_unload(temp2);
						});
					});
				}

				done(PhotoToPBRNode.images[from]);
			});
		});
	}

	///if (krom_metal || krom_vulkan)
	static bgraSwap = (buffer: ArrayBuffer) => {
		let u8a = new Uint8Array(buffer);
		for (let i = 0; i < Math.floor(buffer.byteLength / 4); ++i) {
			let r = u8a[i * 4];
			u8a[i * 4] = u8a[i * 4 + 2];
			u8a[i * 4 + 2] = r;
		}
		return buffer;
	}
	///end

	static def: zui_node_t = {
		id: 0,
		name: _tr("Photo to PBR"),
		type: "PhotoToPBRNode",
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
				name: _tr("Base Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Occlusion"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Roughness"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Metallic"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Normal Map"),
				type: "VECTOR",
				color: 0xffc7c729,
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Height"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 1.0
			}
		],
		buttons: []
	};
}
