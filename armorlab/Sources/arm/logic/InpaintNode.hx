package arm.logic;

import zui.Zui.Nodes;
import iron.System;
import iron.Data;
import iron.ConstData;
import arm.logic.LogicNode;
import arm.logic.LogicParser.f32;
import arm.Translator._tr;

@:keep
class InpaintNode extends LogicNode {

	public static var image: Image = null;
	public static var mask: Image = null;
	static var result: Image = null;

	static var temp: Image = null;
	public static var prompt = "";
	public static var strength = 0.5;
	static var auto = true;

	public function new() {
		super();

		init();
	}

	public static function init() {
		if (image == null) {
			image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		}

		if (mask == null) {
			mask = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.R8);
			Base.notifyOnNextFrame(function() {
				mask.g4.begin();
				mask.g4.clear(Color.fromFloats(1.0, 1.0, 1.0, 1.0));
				mask.g4.end();
			});
		}

		if (temp == null) {
			temp = Image.createRenderTarget(512, 512);
		}

		if (result == null) {
			result = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		}
	}

	public static function buttons(ui: zui.Zui, nodes: zui.Zui.Nodes, node: zui.Zui.TNode) {
		auto = node.buttons[0].default_value == 0 ? false : true;
		if (!auto) {
			strength = ui.slider(zui.Zui.handle("inpaintnode_0", {value: strength}), tr("strength"), 0, 1, true);
			prompt = ui.textArea(zui.Zui.handle("inpaintnode_1"), true, tr("prompt"), true);
			node.buttons[1].height = 1 + prompt.split("\n").length;
		}
		else node.buttons[1].height = 0;
	}

	override function getAsImage(from: Int, done: Image->Void) {
		inputs[0].getAsImage(function(source: Image) {

			Console.progress(tr("Processing") + " - " + tr("Inpaint"));
			Base.notifyOnNextFrame(function() {
				image.g2.begin(false);
				image.g2.drawScaledImage(source, 0, 0, Config.getTextureResX(), Config.getTextureResY());
				image.g2.end();

				auto ? texsynthInpaint(image, false, mask, done) : sdInpaint(image, mask, done);
			});
		});
	}

	override public function getCachedImage(): Image {
		Base.notifyOnNextFrame(function() {
			inputs[0].getAsImage(function(source: Image) {
				if (Base.pipeCopy == null) Base.makePipe();
				if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
				image.g4.begin();
				image.g4.setPipeline(Base.pipeInpaintPreview);
				image.g4.setTexture(Base.tex0InpaintPreview, source);
				image.g4.setTexture(Base.texaInpaintPreview, mask);
				image.g4.setVertexBuffer(ConstData.screenAlignedVB);
				image.g4.setIndexBuffer(ConstData.screenAlignedIB);
				image.g4.drawIndexedVertices();
				image.g4.end();
			});
		});
		return image;
	}

	public function getTarget(): Image {
		return mask;
	}

	public static function texsynthInpaint(image: Image, tiling: Bool, mask: Image/* = null*/, done: Image->Void) {
		var w = arm.Config.getTextureResX();
		var h = arm.Config.getTextureResY();

		var bytes_img = image.getPixels();
		var bytes_mask = mask != null ? mask.getPixels() : new js.lib.ArrayBuffer(w * h);
		var bytes_out = new js.lib.ArrayBuffer(w * h * 4);
		untyped Krom_texsynth.inpaint(w, h, bytes_out, bytes_img, bytes_mask, tiling);

		result = Image.fromBytes(bytes_out, w, h);
		done(result);
	}

	public static function sdInpaint(image: Image, mask: Image, done: Image->Void) {
		init();

		var bytes_img = untyped mask.getPixels().b.buffer;
		var u8 = new js.lib.Uint8Array(untyped bytes_img);
		var f32mask = new js.lib.Float32Array(4 * 64 * 64);

		Data.getBlob("models/sd_vae_encoder.quant.onnx", function(vae_encoder_blob: js.lib.ArrayBuffer) {
			// for (x in 0...Std.int(image.width / 512)) {
				// for (y in 0...Std.int(image.height / 512)) {
					var x = 0;
					var y = 0;

					for (xx in 0...64) {
						for (yy in 0...64) {
							// var step = Std.int(512 / 64);
							// var j = (yy * step * mask.width + xx * step) + (y * 512 * mask.width + x * 512);
							var step = Std.int(mask.width / 64);
							var j = (yy * step * mask.width + xx * step);
							var f = u8[j] / 255.0;
							var i = yy * 64 + xx;
							f32mask[i              ] = f;
							f32mask[i + 64 * 64    ] = f;
							f32mask[i + 64 * 64 * 2] = f;
							f32mask[i + 64 * 64 * 3] = f;
						}
					}

					temp.g2.begin(false);
					// temp.g2.drawImage(image, -x * 512, -y * 512);
					temp.g2.drawScaledImage(image, 0, 0, 512, 512);
					temp.g2.end();

					var bytes_img = untyped temp.getPixels().b.buffer;
					var u8 = new js.lib.Uint8Array(untyped bytes_img);
					var f32 = new js.lib.Float32Array(3 * 512 * 512);
					for (i in 0...(512 * 512)) {
						f32[i                ] = (u8[i * 4    ] / 255.0) * 2.0 - 1.0;
						f32[i + 512 * 512    ] = (u8[i * 4 + 1] / 255.0) * 2.0 - 1.0;
						f32[i + 512 * 512 * 2] = (u8[i * 4 + 2] / 255.0) * 2.0 - 1.0;
					}

					var latents_buf = Krom.mlInference(untyped vae_encoder_blob, [f32.buffer], [[1, 3, 512, 512]], [1, 4, 64, 64], Config.raw.gpu_inference);
					var latents = new js.lib.Float32Array(latents_buf);
					for (i in 0...latents.length) {
						latents[i] = 0.18215 * latents[i];
					}
					var latents_orig = latents.slice(0);

					var noise = new js.lib.Float32Array(latents.length);
					for (i in 0...noise.length) noise[i] = Math.cos(2.0 * 3.14 * RandomNode.getFloat()) * Math.sqrt(-2.0 * Math.log(RandomNode.getFloat()));

					var num_inference_steps = 50;
					var init_timestep = Std.int(num_inference_steps * strength);
					var timestep = TextToPhotoNode.timesteps[num_inference_steps - init_timestep];
					var alphas_cumprod = TextToPhotoNode.alphas_cumprod;
					var sqrt_alpha_prod = Math.pow(alphas_cumprod[timestep], 0.5);
					var sqrt_one_minus_alpha_prod = Math.pow(1.0 - alphas_cumprod[timestep], 0.5);
					for (i in 0...latents.length) {
						latents[i] = sqrt_alpha_prod * latents[i] + sqrt_one_minus_alpha_prod * noise[i];
					}

					var start = num_inference_steps - init_timestep;

					TextToPhotoNode.stableDiffusion(prompt, function(img: Image) {
						// result.g2.begin(false);
						// result.g2.drawImage(img, x * 512, y * 512);
						// result.g2.end();
						result = img;
						done(img);
					}, latents, start, true, f32mask, latents_orig);
				// }
			// }
		});
	}

	public static var def: zui.Zui.TNode = {
		id: 0,
		name: _tr("Inpaint"),
		type: "InpaintNode",
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
				default_value: f32([1.0, 1.0, 1.0, 1.0])
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
				name: "arm.logic.InpaintNode.buttons",
				type: "CUSTOM",
				height: 0
			}
		]
	};
}
