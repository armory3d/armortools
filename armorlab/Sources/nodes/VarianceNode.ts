
// @:keep
class VarianceNode extends LogicNode {

	static temp: Image = null;
	static image: Image = null;
	static inst: VarianceNode = null;
	static prompt = "";

	constructor() {
		super();
		inst = this;
		init();
	}

	static init = () => {
		if (temp == null) {
			temp = Image.createRenderTarget(512, 512);
		}
	}

	static buttons = (ui: zui.Zui, nodes: zui.Zui.Nodes, node: zui.Zui.TNode) => {
		prompt = ui.textArea(zui.Zui.handle("variancenode_0"), true, tr("prompt"), true);
		node.buttons[0].height = prompt.split("\n").length;
	}

	override getAsImage = (from: i32, done: (img: Image)=>void) => {
		let strength = inst.inputs[1].node.value;

		inst.inputs[0].getAsImage((source: Image) => {
			temp.g2.begin(false);
			temp.g2.drawScaledImage(source, 0, 0, 512, 512);
			temp.g2.end();

			let bytes_img = temp.getPixels().b.buffer;
			let u8a = new Uint8Array(bytes_img);
			let f32a = new Float32Array(3 * 512 * 512);
			for (let i = 0; i < (512 * 512); ++i) {
				f32a[i                ] = (u8a[i * 4    ] / 255) * 2.0 - 1.0;
				f32a[i + 512 * 512    ] = (u8a[i * 4 + 1] / 255) * 2.0 - 1.0;
				f32a[i + 512 * 512 * 2] = (u8a[i * 4 + 2] / 255) * 2.0 - 1.0;
			}

			Console.progress(tr("Processing") + " - " + tr("Variance"));
			Base.notifyOnNextFrame(() => {
				Data.getBlob("models/sd_vae_encoder.quant.onnx", (vae_encoder_blob: ArrayBuffer) => {
					let latents_buf = Krom.mlInference(vae_encoder_blob, [f32a.buffer], [[1, 3, 512, 512]], [1, 4, 64, 64], Config.raw.gpu_inference);
					let latents = new Float32Array(latents_buf);
					for (let i = 0; i < latents.length; ++i) {
						latents[i] = 0.18215 * latents[i];
					}

					let noise = new Float32Array(latents.length);
					for (let i = 0; i < noise.length; ++i) noise[i] = Math.cos(2.0 * 3.14 * RandomNode.getFloat()) * Math.sqrt(-2.0 * Math.log(RandomNode.getFloat()));
					let num_inference_steps = 50;
					let init_timestep = Math.floor(num_inference_steps * strength);
					let timesteps = TextToPhotoNode.timesteps[num_inference_steps - init_timestep];
					let alphas_cumprod = TextToPhotoNode.alphas_cumprod;
					let sqrt_alpha_prod = Math.pow(alphas_cumprod[timesteps], 0.5);
					let sqrt_one_minus_alpha_prod = Math.pow(1.0 - alphas_cumprod[timesteps], 0.5);
					for (let i = 0; i < latents.length; ++i) {
						latents[i] = sqrt_alpha_prod * latents[i] + sqrt_one_minus_alpha_prod * noise[i];
					}
					let t_start = num_inference_steps - init_timestep;

					TextToPhotoNode.stableDiffusion(prompt, (_image: Image) => {
						image = _image;
						done(image);
					}, latents, t_start);
				});
			});
		});
	}

	override getCachedImage = (): Image => {
		return image;
	}

	static def: zui.Zui.TNode = {
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
				default_value: array_f32([0.0, 0.0, 0.0, 1.0])
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
				default_value: array_f32([0.0, 0.0, 0.0, 1.0])
			}
		],
		buttons: [
			{
				name: "arm.nodes.VarianceNode.buttons",
				type: "CUSTOM",
				height: 1
			}
		]
	};
}
