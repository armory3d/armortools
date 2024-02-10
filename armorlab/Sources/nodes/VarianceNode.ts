
class VarianceNode extends LogicNode {

	static temp: image_t = null;
	static image: image_t = null;
	static inst: VarianceNode = null;
	static prompt = "";

	constructor() {
		super();
		VarianceNode.inst = this;
		VarianceNode.init();
	}

	static init = () => {
		if (VarianceNode.temp == null) {
			VarianceNode.temp = image_create_render_target(512, 512);
		}
	}

	static buttons = (ui: zui_t, nodes: zui_nodes_t, node: zui_node_t) => {
		VarianceNode.prompt = zui_text_area(zui_handle("variancenode_0"), Align.Left, true, tr("prompt"), true);
		node.buttons[0].height = VarianceNode.prompt.split("\n").length;
	}

	override getAsImage = (from: i32, done: (img: image_t)=>void) => {
		let strength = (VarianceNode.inst.inputs[1].node as any).value;

		VarianceNode.inst.inputs[0].getAsImage((source: image_t) => {
			g2_begin(VarianceNode.temp, false);
			g2_draw_scaled_image(source, 0, 0, 512, 512);
			g2_end();

			let bytes_img = image_get_pixels(VarianceNode.temp);
			let u8a = new Uint8Array(bytes_img);
			let f32a = new Float32Array(3 * 512 * 512);
			for (let i = 0; i < (512 * 512); ++i) {
				f32a[i                ] = (u8a[i * 4    ] / 255) * 2.0 - 1.0;
				f32a[i + 512 * 512    ] = (u8a[i * 4 + 1] / 255) * 2.0 - 1.0;
				f32a[i + 512 * 512 * 2] = (u8a[i * 4 + 2] / 255) * 2.0 - 1.0;
			}

			Console.progress(tr("Processing") + " - " + tr("Variance"));
			Base.notifyOnNextFrame(() => {
				data_get_blob("models/sd_vae_encoder.quant.onnx", (vae_encoder_blob: ArrayBuffer) => {
					let latents_buf = krom_ml_inference(vae_encoder_blob, [f32a.buffer], [[1, 3, 512, 512]], [1, 4, 64, 64], Config.raw.gpu_inference);
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

					TextToPhotoNode.stableDiffusion(VarianceNode.prompt, (_image: image_t) => {
						VarianceNode.image = _image;
						done(VarianceNode.image);
					}, latents, t_start);
				});
			});
		});
	}

	override getCachedImage = (): image_t => {
		return VarianceNode.image;
	}

	static def: zui_node_t = {
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
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
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
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
			}
		],
		buttons: [
			{
				name: "VarianceNode.buttons",
				type: "CUSTOM",
				height: 1
			}
		]
	};
}
