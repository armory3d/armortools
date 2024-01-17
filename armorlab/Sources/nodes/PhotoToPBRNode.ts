
// @:keep
class PhotoToPBRNode extends LogicNode {

	static temp: Image = null;
	static images: Image[] = null;
	static modelNames = ["base", "occlusion", "roughness", "metallic", "normal", "height"];

	static cachedSource: Image = null;
	static borderW = 64;
	static tileW = 2048;
	static tileWithBorderW = tileW + borderW * 2;

	constructor() {
		super();

		if (temp == null) {
			temp = Image.createRenderTarget(tileWithBorderW, tileWithBorderW);
		}

		init();
	}

	static init = () => {
		if (images == null) {
			images = [];
			for (let i = 0; i < modelNames.length; ++i) {
				images.push(Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY()));
			}
		}
	}

	override getAsImage = (from: i32, done: (img: Image)=>void) => {
		let getSource = (done: (img: Image)=>void) => {
			if (cachedSource != null) done(cachedSource);
			else inputs[0].getAsImage(done);
		}

		getSource((source: Image) => {
			cachedSource = source;

			Console.progress(tr("Processing") + " - " + tr("Photo to PBR"));
			Base.notifyOnNextFrame(() => {
				let tileFloats: Float32Array[] = [];
				let tilesX = Math.floor(Config.getTextureResX() / tileW);
				let tilesY = Math.floor(Config.getTextureResY() / tileW);
				let numTiles = tilesX * tilesY;
				for (let i = 0; i < numTiles; ++i) {
					let x = i % tilesX;
					let y = Math.floor(i / tilesX);

					temp.g2.begin(false);
					temp.g2.drawScaledImage(source, borderW - x * tileW, borderW - y * tileW, -Config.getTextureResX(), Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW, borderW - y * tileW, Config.getTextureResX(), -Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW, borderW - y * tileW, -Config.getTextureResX(), -Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW + tileW, borderW - y * tileW + tileW, Config.getTextureResX(), Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW + tileW, borderW - y * tileW + tileW, -Config.getTextureResX(), Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW + tileW, borderW - y * tileW + tileW, Config.getTextureResX(), -Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW, borderW - y * tileW, Config.getTextureResX(), Config.getTextureResY());
					temp.g2.end();

					let bytes_img = temp.getPixels();
					let u8a = new Uint8Array(bytes_img);
					let f32a = new Float32Array(3 * tileWithBorderW * tileWithBorderW);
					for (let i = 0; i < (tileWithBorderW * tileWithBorderW)) {
						f32a[i                                        ] = (u8a[i * 4    ] / 255 - 0.5) / 0.5;
						f32a[i + tileWithBorderW * tileWithBorderW    ] = (u8a[i * 4 + 1] / 255 - 0.5) / 0.5;
						f32a[i + tileWithBorderW * tileWithBorderW * 2] = (u8a[i * 4 + 2] / 255 - 0.5) / 0.5;
					}

					Data.getBlob("models/photo_to_" + modelNames[from] + ".quant.onnx", (model_blob: ArrayBuffer) => {
						let buf = Krom.mlInference(model_blob, [f32a.buffer], null, null, Config.raw.gpu_inference);
						let ar = new Float32Array(buf);
						let u8a = new Uint8Array(4 * tileW * tileW);
						let offsetG = (from == ChannelBaseColor || from == ChannelNormalMap) ? tileWithBorderW * tileWithBorderW : 0;
						let offsetB = (from == ChannelBaseColor || from == ChannelNormalMap) ? tileWithBorderW * tileWithBorderW * 2 : 0;
						for (let i = 0; i < (tileW * tileW); ++i) {
							let x = borderW + i % tileW;
							let y = borderW + Math.floor(i / tileW);
							u8a[i * 4    ] = Math.floor((ar[y * tileWithBorderW + x          ] * 0.5 + 0.5) * 255);
							u8a[i * 4 + 1] = Math.floor((ar[y * tileWithBorderW + x + offsetG] * 0.5 + 0.5) * 255);
							u8a[i * 4 + 2] = Math.floor((ar[y * tileWithBorderW + x + offsetB] * 0.5 + 0.5) * 255);
							u8a[i * 4 + 3] = 255;
						}
						tileFloats.push(ar);

						// Use border pixels to blend seams
						if (i > 0) {
							if (x > 0) {
								let ar = tileFloats[i - 1];
								for (let yy = 0; yy < tileW; ++yy) {
									for (let xx = 0; xx < borderW; ++xx) {
										let i = yy * tileW + xx;
										let a = u8a[i * 4];
										let b = u8a[i * 4 + 1];
										let c = u8a[i * 4 + 2];

										let aa = Math.floor((ar[(borderW + yy) * tileWithBorderW + borderW + tileW + xx          ] * 0.5 + 0.5) * 255);
										let bb = Math.floor((ar[(borderW + yy) * tileWithBorderW + borderW + tileW + xx + offsetG] * 0.5 + 0.5) * 255);
										let cc = Math.floor((ar[(borderW + yy) * tileWithBorderW + borderW + tileW + xx + offsetB] * 0.5 + 0.5) * 255);

										let f = xx / borderW;
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
								for (let xx = 0; xx < tileW; ++xx) {
									for (let yy = 0; yy < borderW; ++yy) {
										let i = yy * tileW + xx;
										let a = u8a[i * 4];
										let b = u8a[i * 4 + 1];
										let c = u8a[i * 4 + 2];

										let aa = Math.floor((ar[(borderW + tileW + yy) * tileWithBorderW + borderW + xx          ] * 0.5 + 0.5) * 255);
										let bb = Math.floor((ar[(borderW + tileW + yy) * tileWithBorderW + borderW + xx + offsetG] * 0.5 + 0.5) * 255);
										let cc = Math.floor((ar[(borderW + tileW + yy) * tileWithBorderW + borderW + xx + offsetB] * 0.5 + 0.5) * 255);

										let f = yy / borderW;
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
						if (from == ChannelBaseColor) bgraSwap(u8a.buffer);
						///end

						let temp2 = Image.fromBytes(u8a.buffer, tileW, tileW);
						images[from].g2.begin(false);
						images[from].g2.drawImage(temp2, x * tileW, y * tileW);
						images[from].g2.end();
						Base.notifyOnNextFrame(() => {
							temp2.unload();
						});
					});
				}

				done(images[from]);
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

	static def: zui.Zui.TNode = {
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
				default_value: array_f32([0.0, 0.0, 0.0, 1.0])
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Base Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: array_f32([0.0, 0.0, 0.0, 1.0])
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
				default_value: f32([0.0, 0.0, 0.0, 1.0])
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
