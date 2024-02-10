
declare let Krom_texsynth: any;

class InpaintNode extends LogicNode {

	static image: image_t = null;
	static mask: image_t = null;
	static result: image_t = null;

	static temp: image_t = null;
	static prompt = "";
	static strength = 0.5;
	static auto = true;

	constructor() {
		super();
		InpaintNode.init();
	}

	static init = () => {
		if (InpaintNode.image == null) {
			InpaintNode.image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY());
		}

		if (InpaintNode.mask == null) {
			InpaintNode.mask = image_create_render_target(Config.getTextureResX(), Config.getTextureResY(), tex_format_t.R8);
			Base.notifyOnNextFrame(() => {
				g4_begin(InpaintNode.mask);
				g4_clear(color_from_floats(1.0, 1.0, 1.0, 1.0));
				g4_end();
			});
		}

		if (InpaintNode.temp == null) {
			InpaintNode.temp = image_create_render_target(512, 512);
		}

		if (InpaintNode.result == null) {
			InpaintNode.result = image_create_render_target(Config.getTextureResX(), Config.getTextureResY());
		}
	}

	static buttons = (ui: zui_t, nodes: zui_nodes_t, node: zui_node_t) => {
		InpaintNode.auto = node.buttons[0].default_value == 0 ? false : true;
		if (!InpaintNode.auto) {
			InpaintNode.strength = zui_slider(zui_handle("inpaintnode_0", { value: InpaintNode.strength }), tr("strength"), 0, 1, true);
			InpaintNode.prompt = zui_text_area(zui_handle("inpaintnode_1"), Align.Left, true, tr("prompt"), true);
			node.buttons[1].height = 1 + InpaintNode.prompt.split("\n").length;
		}
		else node.buttons[1].height = 0;
	}

	override getAsImage = (from: i32, done: (img: image_t)=>void) => {
		this.inputs[0].getAsImage((source: image_t) => {

			Console.progress(tr("Processing") + " - " + tr("Inpaint"));
			Base.notifyOnNextFrame(() => {
				g2_begin(InpaintNode.image, false);
				g2_draw_scaled_image(source, 0, 0, Config.getTextureResX(), Config.getTextureResY());
				g2_end();

				InpaintNode.auto ? InpaintNode.texsynthInpaint(InpaintNode.image, false, InpaintNode.mask, done) : InpaintNode.sdInpaint(InpaintNode.image, InpaintNode.mask, done);
			});
		});
	}

	override getCachedImage = (): image_t => {
		Base.notifyOnNextFrame(() => {
			this.inputs[0].getAsImage((source: image_t) => {
				if (Base.pipeCopy == null) Base.makePipe();
				if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
				g4_begin(InpaintNode.image);
				g4_set_pipeline(Base.pipeInpaintPreview);
				g4_set_tex(Base.tex0InpaintPreview, source);
				g4_set_tex(Base.texaInpaintPreview, InpaintNode.mask);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			});
		});
		return InpaintNode.image;
	}

	getTarget = (): image_t => {
		return InpaintNode.mask;
	}

	static texsynthInpaint = (image: image_t, tiling: bool, mask: image_t/* = null*/, done: (img: image_t)=>void) => {
		let w = Config.getTextureResX();
		let h = Config.getTextureResY();

		let bytes_img = image_get_pixels(image);
		let bytes_mask = mask != null ? image_get_pixels(mask) : new ArrayBuffer(w * h);
		let bytes_out = new ArrayBuffer(w * h * 4);
		Krom_texsynth.inpaint(w, h, bytes_out, bytes_img, bytes_mask, tiling);

		InpaintNode.result = image_from_bytes(bytes_out, w, h);
		done(InpaintNode.result);
	}

	static sdInpaint = (image: image_t, mask: image_t, done: (img: image_t)=>void) => {
		InpaintNode.init();

		let bytes_img = image_get_pixels(mask);
		let u8 = new Uint8Array(bytes_img);
		let f32mask = new Float32Array(4 * 64 * 64);

		data_get_blob("models/sd_vae_encoder.quant.onnx", (vae_encoder_blob: ArrayBuffer) => {
			// for (let x = 0; x < Math.floor(image.width / 512); ++x) {
				// for (let y = 0; y < Math.floor(image.height / 512); ++y) {
					let x = 0;
					let y = 0;

					for (let xx = 0; xx < 64; ++xx) {
						for (let yy = 0; yy < 64; ++yy) {
							// let step = Math.floor(512 / 64);
							// let j = (yy * step * mask.width + xx * step) + (y * 512 * mask.width + x * 512);
							let step = Math.floor(mask.width / 64);
							let j = (yy * step * mask.width + xx * step);
							let f = u8[j] / 255.0;
							let i = yy * 64 + xx;
							f32mask[i              ] = f;
							f32mask[i + 64 * 64    ] = f;
							f32mask[i + 64 * 64 * 2] = f;
							f32mask[i + 64 * 64 * 3] = f;
						}
					}

					g2_begin(InpaintNode.temp, false);
					// g2_drawImage(image, -x * 512, -y * 512);
					g2_draw_scaled_image(image, 0, 0, 512, 512);
					g2_end();

					let bytes_img = image_get_pixels(InpaintNode.temp);
					let u8a = new Uint8Array(bytes_img);
					let f32a = new Float32Array(3 * 512 * 512);
					for (let i = 0; i < (512 * 512); ++i) {
						f32a[i                ] = (u8a[i * 4    ] / 255.0) * 2.0 - 1.0;
						f32a[i + 512 * 512    ] = (u8a[i * 4 + 1] / 255.0) * 2.0 - 1.0;
						f32a[i + 512 * 512 * 2] = (u8a[i * 4 + 2] / 255.0) * 2.0 - 1.0;
					}

					let latents_buf = krom_ml_inference(vae_encoder_blob, [f32a.buffer], [[1, 3, 512, 512]], [1, 4, 64, 64], Config.raw.gpu_inference);
					let latents = new Float32Array(latents_buf);
					for (let i = 0; i < latents.length; ++i) {
						latents[i] = 0.18215 * latents[i];
					}
					let latents_orig = latents.slice(0);

					let noise = new Float32Array(latents.length);
					for (let i = 0; i < noise.length; ++i) noise[i] = Math.cos(2.0 * 3.14 * RandomNode.getFloat()) * Math.sqrt(-2.0 * Math.log(RandomNode.getFloat()));

					let num_inference_steps = 50;
					let init_timestep = Math.floor(num_inference_steps * InpaintNode.strength);
					let timestep = TextToPhotoNode.timesteps[num_inference_steps - init_timestep];
					let alphas_cumprod = TextToPhotoNode.alphas_cumprod;
					let sqrt_alpha_prod = Math.pow(alphas_cumprod[timestep], 0.5);
					let sqrt_one_minus_alpha_prod = Math.pow(1.0 - alphas_cumprod[timestep], 0.5);
					for (let i = 0; i < latents.length; ++i) {
						latents[i] = sqrt_alpha_prod * latents[i] + sqrt_one_minus_alpha_prod * noise[i];
					}

					let start = num_inference_steps - init_timestep;

					TextToPhotoNode.stableDiffusion(InpaintNode.prompt, (img: image_t) => {
						// result.g2_begin(false);
						// result.g2_draw_image(img, x * 512, y * 512);
						// result.g2_end();
						InpaintNode.result = img;
						done(img);
					}, latents, start, true, f32mask, latents_orig);
				// }
			// }
		});
	}

	static def: zui_node_t = {
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
				default_value: new Float32Array([1.0, 1.0, 1.0, 1.0])
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
				name: _tr("auto"),
				type: "BOOL",
				default_value: true,
				output: 0
			},
			{
				name: "InpaintNode.buttons",
				type: "CUSTOM",
				height: 0
			}
		]
	};
}
