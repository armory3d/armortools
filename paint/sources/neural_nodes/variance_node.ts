
// type variance_node_t = {
// 	base?: logic_node_t;
// };

// let variance_node_temp: gpu_texture_t = null;
// let variance_node_image: gpu_texture_t = null;
// let variance_node_inst: variance_node_t = null;
// let variance_node_prompt: string = "";

// function variance_node_create(raw: ui_node_t, args: f32_array_t): variance_node_t {
// 	let n: variance_node_t = {};
// 	n.base = logic_node_create(n);
// 	n.base.get_as_image = variance_node_get_as_image;
// 	n.base.get_cached_image = variance_node_get_cached_image;

// 	variance_node_inst = n;
// 	variance_node_init();

// 	return n;
// }

// function variance_node_init() {
// 	if (variance_node_temp == null) {
// 		variance_node_temp = gpu_create_render_target(512, 512);
// 	}
// }

// function variance_node_button(node_id: i32) {
// 	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

// 	variance_node_prompt = ui_text_area(ui_handle(__ID__), ui_align_t.LEFT, true, tr("prompt"), true);
// 	node.buttons[0].height = string_split(variance_node_prompt, "\n").length;
// }

// function variance_node_get_as_image(self: variance_node_t, from: i32): gpu_texture_t {
// 	let node: float_node_t = variance_node_inst.base.inputs[1].node;
// 	let strength: f32 = node.value;

// 	let source: gpu_texture_t = logic_node_input_get_as_image(variance_node_inst.base.inputs[0]);
// 	draw_begin(variance_node_temp);
// 	draw_scaled_image(source, 0, 0, 512, 512);
// 	draw_end();

// 	let bytes_img: buffer_t = gpu_get_texture_pixels(variance_node_temp);
// 	let u8a: u8_array_t = bytes_img;
// 	let f32a: f32_array_t = f32_array_create(3 * 512 * 512);
// 	for (let i: i32 = 0; i < (512 * 512); ++i) {
// 		f32a[i                ] = (u8a[i * 4    ] / 255) * 2.0 - 1.0;
// 		f32a[i + 512 * 512    ] = (u8a[i * 4 + 1] / 255) * 2.0 - 1.0;
// 		f32a[i + 512 * 512 * 2] = (u8a[i * 4 + 2] / 255) * 2.0 - 1.0;
// 	}

// 	console_progress(tr("Processing") + " - " + tr("Variance"));

// 	let vae_encoder_blob: buffer_t = data_get_blob("models/sd_vae_encoder.quant.onnx");
// 	let tensors: buffer_t[] = [buffer_create_from_raw(f32a.buffer, f32a.length * 4)];
// 	let input_shape: i32_array_t[] = [];
// 	let input_shape0: i32[] = [1, 3, 512, 512];
// 	array_push(input_shape, input_shape0);
// 	let output_shape: i32[] = [1, 4, 64, 64];
// 	let latents_buf: buffer_t = iron_ml_inference(vae_encoder_blob, tensors, input_shape, output_shape, config_raw.gpu_inference);
// 	let latents: f32_array_t = f32_array_create_from_buffer(latents_buf);
// 	for (let i: i32 = 0; i < latents.length; ++i) {
// 		latents[i] = 0.18215 * latents[i];
// 	}

// 	let noise: f32_array_t = f32_array_create(latents.length);
// 	for (let i: i32 = 0; i < noise.length; ++i) {
// 		noise[i] = math_cos(2.0 * 3.14 * random_node_get_float()) * math_sqrt(-2.0 * math_log(random_node_get_float()));
// 	}
// 	let num_inference_steps: i32 = 50;
// 	let init_timestep: i32 = math_floor(num_inference_steps * strength);
// 	let timesteps: i32 = text_to_photo_node_timesteps[num_inference_steps - init_timestep];
// 	let alphas_cumprod: f32[] = text_to_photo_node_alphas_cumprod;
// 	let sqrt_alpha_prod: f32 = math_pow(alphas_cumprod[timesteps], 0.5);
// 	let sqrt_one_minus_alpha_prod: f32 = math_pow(1.0 - alphas_cumprod[timesteps], 0.5);
// 	for (let i: i32 = 0; i < latents.length; ++i) {
// 		latents[i] = sqrt_alpha_prod * latents[i] + sqrt_one_minus_alpha_prod * noise[i];
// 	}
// 	let t_start: f32 = num_inference_steps - init_timestep;
// 	variance_node_image = text_to_photo_node_stable_diffusion(variance_node_prompt, latents, t_start);
// 	return variance_node_image;
// }

// function variance_node_get_cached_image(self: variance_node_t): gpu_texture_t {
// 	return variance_node_image;
// }

// let variance_node_def: ui_node_t = {
// 	id: 0,
// 	name: _tr("Variance"),
// 	type: "variance_node",
// 	x: 0,
// 	y: 0,
// 	color: 0xff4982a0,
// 	inputs: [
// 		{
// 			id: 0,
// 			node_id: 0,
// 			name: _tr("Color"),
// 			type: "RGBA",
// 			color: 0xffc7c729,
// 			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
// 			min: 0.0,
// 			max: 1.0,
// 			precision: 100,
// 			display: 0
// 		},
// 		{
// 			id: 0,
// 			node_id: 0,
// 			name: _tr("Strength"),
// 			type: "VALUE",
// 			color: 0xffa1a1a1,
// 			default_value: f32_array_create_x(0.5),
// 			min: 0.0,
// 			max: 1.0,
// 			precision: 100,
// 			display: 0
// 		}
// 	],
// 	outputs: [
// 		{
// 			id: 0,
// 			node_id: 0,
// 			name: _tr("Color"),
// 			type: "RGBA",
// 			color: 0xffc7c729,
// 			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
// 			min: 0.0,
// 			max: 1.0,
// 			precision: 100,
// 			display: 0
// 		}
// 	],
// 	buttons: [
// 		{
// 			name: "variance_node_button",
// 			type: "CUSTOM",
// 			output: -1,
// 			default_value: f32_array_create_x(0),
// 			data: null,
// 			min: 0.0,
// 			max: 1.0,
// 			precision: 100,
// 			height: 1
// 		}
// 	],
// 	width: 0,
// 	flags: 0
// };
