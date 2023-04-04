package arm.logic;

import zui.Nodes;
import arm.logic.LogicNode;
import arm.logic.LogicParser.f32;
import arm.Translator._tr;

@:keep
class VarianceNode extends LogicNode {

	static var temp: kha.Image = null;
	static var image: kha.Image = null;
	static var inst: VarianceNode = null;
	static var prompt = "";

	public function new(tree: LogicTree) {
		super(tree);

		inst = this;

		init();
	}

	public static function init() {
		if (temp == null) {
			temp = kha.Image.createRenderTarget(512, 512);
		}
	}

	public static function buttons(ui: zui.Zui, nodes: zui.Nodes, node: zui.Nodes.TNode) {
		prompt = zui.Ext.textArea(ui, zui.Id.handle(), true, tr("prompt"), true);
		node.buttons[0].height = prompt.split("\n").length;
	}

	override function getAsImage(from: Int, done: kha.Image->Void) {
		var strength = untyped inst.inputs[1].node.value;

		inst.inputs[0].getAsImage(function(source: kha.Image) {
			temp.g2.begin(false);
			temp.g2.drawScaledImage(source, 0, 0, 512, 512);
			temp.g2.end();

			var bytes_img = untyped temp.getPixels().b.buffer;
			var u8 = new js.lib.Uint8Array(untyped bytes_img);
			var f32 = new js.lib.Float32Array(3 * 512 * 512);
			for (i in 0...(512 * 512)) {
				f32[i                ] = (u8[i * 4    ] / 255) * 2.0 - 1.0;
				f32[i + 512 * 512    ] = (u8[i * 4 + 1] / 255) * 2.0 - 1.0;
				f32[i + 512 * 512 * 2] = (u8[i * 4 + 2] / 255) * 2.0 - 1.0;
			}

			Console.progress(tr("Processing") + " - " + tr("Variance"));
			App.notifyOnNextFrame(function() {
				kha.Assets.loadBlobFromPath("data/models/sd_vae_encoder.quant.onnx", function(vae_encoder_blob: kha.Blob) {
					var latents_buf = Krom.mlInference(untyped vae_encoder_blob.toBytes().b.buffer, [f32.buffer], [[1, 3, 512, 512]], [1, 4, 64, 64], Config.raw.gpu_inference);
					var latents = new js.lib.Float32Array(latents_buf);
					for (i in 0...latents.length) {
						latents[i] = 0.18215 * latents[i];
					}

					var noise = new js.lib.Float32Array(latents.length);
					for (i in 0...noise.length) noise[i] = Math.cos(2.0 * 3.14 * RandomNode.getFloat()) * Math.sqrt(-2.0 * Math.log(RandomNode.getFloat()));
					var num_inference_steps = 50;
					var init_timestep = Std.int(num_inference_steps * strength);
					var timesteps = @:privateAccess TextToPhotoNode.timesteps[num_inference_steps - init_timestep];
					var alphas_cumprod = @:privateAccess TextToPhotoNode.alphas_cumprod;
					var sqrt_alpha_prod = Math.pow(alphas_cumprod[timesteps], 0.5);
					var sqrt_one_minus_alpha_prod = Math.pow(1.0 - alphas_cumprod[timesteps], 0.5);
					for (i in 0...latents.length) {
						latents[i] = sqrt_alpha_prod * latents[i] + sqrt_one_minus_alpha_prod * noise[i];
					}
					var t_start = num_inference_steps - init_timestep;

					TextToPhotoNode.stableDiffusion(prompt, function(_image: kha.Image) {
						image = _image;
						done(image);
					}, latents, t_start);
				});
			});
		});
	}

	override public function getCachedImage(): kha.Image {
		return image;
	}

	public static var def: TNode = {
		id: 0,
		name: _tr("Variance"),
		type: "VarianceNode",
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
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Strength"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.5
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
				name: "arm.logic.VarianceNode.buttons",
				type: "CUSTOM",
				height: 1
			}
		]
	};
}
