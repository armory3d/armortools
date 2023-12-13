package arm.nodes;

import zui.Zui.Nodes;
import iron.System;
import iron.Data;
import arm.LogicNode;
import arm.ParserLogic.f32;
import arm.Translator._tr;

@:keep
class PhotoToPBRNode extends LogicNode {

	static var temp: Image = null;
	public static var images: Array<Image> = null;
	static var modelNames = ["base", "occlusion", "roughness", "metallic", "normal", "height"];

	public static var cachedSource: Image = null;
	static inline var borderW = 64;
	static inline var tileW = 2048;
	static inline var tileWithBorderW = tileW + borderW * 2;

	public function new() {
		super();

		if (temp == null) {
			temp = Image.createRenderTarget(tileWithBorderW, tileWithBorderW);
		}

		init();
	}

	public static function init() {
		if (images == null) {
			images = [];
			for (i in 0...modelNames.length) {
				images.push(Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY()));
			}
		}
	}

	override function getAsImage(from: Int, done: Image->Void) {
		function getSource(done: Image->Void) {
			if (cachedSource != null) done(cachedSource);
			else inputs[0].getAsImage(done);
		}

		getSource(function(source: Image) {
			cachedSource = source;

			Console.progress(tr("Processing") + " - " + tr("Photo to PBR"));
			Base.notifyOnNextFrame(function() {
				var tileFloats: Array<js.lib.Float32Array> = [];
				var tilesX = Std.int(Config.getTextureResX() / tileW);
				var tilesY = Std.int(Config.getTextureResY() / tileW);
				var numTiles = tilesX * tilesY;
				for (i in 0...numTiles) {
					var x = i % tilesX;
					var y = Std.int(i / tilesX);

					temp.g2.begin(false);
					temp.g2.drawScaledImage(source, borderW - x * tileW, borderW - y * tileW, -Config.getTextureResX(), Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW, borderW - y * tileW, Config.getTextureResX(), -Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW, borderW - y * tileW, -Config.getTextureResX(), -Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW + tileW, borderW - y * tileW + tileW, Config.getTextureResX(), Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW + tileW, borderW - y * tileW + tileW, -Config.getTextureResX(), Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW + tileW, borderW - y * tileW + tileW, Config.getTextureResX(), -Config.getTextureResY());
					temp.g2.drawScaledImage(source, borderW - x * tileW, borderW - y * tileW, Config.getTextureResX(), Config.getTextureResY());
					temp.g2.end();

					var bytes_img = temp.getPixels();
					var u8 = new js.lib.Uint8Array(bytes_img);
					var f32 = new js.lib.Float32Array(3 * tileWithBorderW * tileWithBorderW);
					for (i in 0...(tileWithBorderW * tileWithBorderW)) {
						f32[i                                        ] = (u8[i * 4    ] / 255 - 0.5) / 0.5;
						f32[i + tileWithBorderW * tileWithBorderW    ] = (u8[i * 4 + 1] / 255 - 0.5) / 0.5;
						f32[i + tileWithBorderW * tileWithBorderW * 2] = (u8[i * 4 + 2] / 255 - 0.5) / 0.5;
					}

					Data.getBlob("models/photo_to_" + modelNames[from] + ".quant.onnx", function(model_blob: js.lib.ArrayBuffer) {
						var buf = Krom.mlInference(untyped model_blob, [f32.buffer], null, null, Config.raw.gpu_inference);
						var ar = new js.lib.Float32Array(buf);
						var u8 = new js.lib.Uint8Array(4 * tileW * tileW);
						var offsetG = (from == ChannelBaseColor || from == ChannelNormalMap) ? tileWithBorderW * tileWithBorderW : 0;
						var offsetB = (from == ChannelBaseColor || from == ChannelNormalMap) ? tileWithBorderW * tileWithBorderW * 2 : 0;
						for (i in 0...(tileW * tileW)) {
							var x = borderW + i % tileW;
							var y = borderW + Std.int(i / tileW);
							u8[i * 4    ] = Std.int((ar[y * tileWithBorderW + x          ] * 0.5 + 0.5) * 255);
							u8[i * 4 + 1] = Std.int((ar[y * tileWithBorderW + x + offsetG] * 0.5 + 0.5) * 255);
							u8[i * 4 + 2] = Std.int((ar[y * tileWithBorderW + x + offsetB] * 0.5 + 0.5) * 255);
							u8[i * 4 + 3] = 255;
						}
						tileFloats.push(ar);

						// Use border pixels to blend seams
						if (i > 0) {
							if (x > 0) {
								var ar = tileFloats[i - 1];
								for (yy in 0...tileW) {
									for (xx in 0...borderW) {
										var i = yy * tileW + xx;
										var a = u8[i * 4];
										var b = u8[i * 4 + 1];
										var c = u8[i * 4 + 2];

										var aa = Std.int((ar[(borderW + yy) * tileWithBorderW + borderW + tileW + xx          ] * 0.5 + 0.5) * 255);
										var bb = Std.int((ar[(borderW + yy) * tileWithBorderW + borderW + tileW + xx + offsetG] * 0.5 + 0.5) * 255);
										var cc = Std.int((ar[(borderW + yy) * tileWithBorderW + borderW + tileW + xx + offsetB] * 0.5 + 0.5) * 255);

										var f = xx / borderW;
										var invf = 1.0 - f;
										a = Std.int(a * f + aa * invf);
										b = Std.int(b * f + bb * invf);
										c = Std.int(c * f + cc * invf);

										u8[i * 4    ] = a;
										u8[i * 4 + 1] = b;
										u8[i * 4 + 2] = c;
									}
								}
							}
							if (y > 0) {
								var ar = tileFloats[i - tilesX];
								for (xx in 0...tileW) {
									for (yy in 0...borderW) {
										var i = yy * tileW + xx;
										var a = u8[i * 4];
										var b = u8[i * 4 + 1];
										var c = u8[i * 4 + 2];

										var aa = Std.int((ar[(borderW + tileW + yy) * tileWithBorderW + borderW + xx          ] * 0.5 + 0.5) * 255);
										var bb = Std.int((ar[(borderW + tileW + yy) * tileWithBorderW + borderW + xx + offsetG] * 0.5 + 0.5) * 255);
										var cc = Std.int((ar[(borderW + tileW + yy) * tileWithBorderW + borderW + xx + offsetB] * 0.5 + 0.5) * 255);

										var f = yy / borderW;
										var invf = 1.0 - f;
										a = Std.int(a * f + aa * invf);
										b = Std.int(b * f + bb * invf);
										c = Std.int(c * f + cc * invf);

										u8[i * 4    ] = a;
										u8[i * 4 + 1] = b;
										u8[i * 4 + 2] = c;
									}
								}
							}
						}

						#if (krom_metal || krom_vulkan)
						if (from == ChannelBaseColor) bgraSwap(u8.buffer);
						#end

						var temp2 = Image.fromBytes(u8.buffer, tileW, tileW);
						images[from].g2.begin(false);
						images[from].g2.drawImage(temp2, x * tileW, y * tileW);
						images[from].g2.end();
						Base.notifyOnNextFrame(function() {
							temp2.unload();
						});
					});
				}

				done(images[from]);
			});
		});
	}

	#if (krom_metal || krom_vulkan)
	static function bgraSwap(buffer: js.lib.ArrayBuffer) {
		var u8 = new js.lib.Uint8Array(buffer);
		for (i in 0...Std.int(buffer.byteLength / 4)) {
			var r = u8[i * 4];
			u8[i * 4] = u8[i * 4 + 2];
			u8[i * 4 + 2] = r;
		}
		return buffer;
	}
	#end

	public static var def: zui.Zui.TNode = {
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
				default_value: f32([0.0, 0.0, 0.0, 1.0])
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Base Color"),
				type: "RGBA",
				color: 0xffc7c729,
				default_value: f32([0.0, 0.0, 0.0, 1.0])
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
