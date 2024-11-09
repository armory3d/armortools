
///include "../plugins/proc_texsynth/proc_texsynth.h"

type inpaint_node_t = {
	base?: logic_node_t;
};

let inpaint_node_image: image_t = null;
let inpaint_node_mask: image_t = null;
let inpaint_node_result: image_t = null;

let inpaint_node_temp: image_t = null;
let inpaint_node_prompt: string = "";
let inpaint_node_strength: f32 = 0.5;
let inpaint_node_auto: bool = true;

function inpaint_node_create(raw: ui_node_t, args: f32_array_t): inpaint_node_t {
	let n: inpaint_node_t = {};
	n.base = logic_node_create(n);
	n.base.get_as_image = inpaint_node_get_as_image;
	n.base.get_cached_image = inpaint_node_get_cached_image;

	inpaint_node_init();

	return n;
}

function inpaint_node_init() {
	if (inpaint_node_image == null) {
		inpaint_node_image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y());
	}

	if (inpaint_node_mask == null) {
		inpaint_node_mask = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);
		app_notify_on_next_frame(function () {
			g4_begin(inpaint_node_mask);
			g4_clear(color_from_floats(1.0, 1.0, 1.0, 1.0));
			g4_end();
		});
	}

	if (inpaint_node_temp == null) {
		inpaint_node_temp = image_create_render_target(512, 512);
	}

	if (inpaint_node_result == null) {
		inpaint_node_result = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y());
	}
}

function inpaint_node_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	inpaint_node_auto = node.buttons[0].default_value[0] == 0 ? false : true;
	if (!inpaint_node_auto) {

		let inpaint_node_strength_handle: ui_handle_t = ui_handle(__ID__);
		if (inpaint_node_strength_handle.init) {
			inpaint_node_strength_handle.value = inpaint_node_strength;
		}

		inpaint_node_strength = ui_slider(inpaint_node_strength_handle, tr("strength"), 0, 1, true);
		inpaint_node_prompt = ui_text_area(ui_handle(__ID__), ui_align_t.LEFT, true, tr("prompt"), true);
		node.buttons[1].height = 1 + string_split(inpaint_node_prompt, "\n").length;
	}
	else {
		node.buttons[1].height = 0;
	}
}

function inpaint_node_get_as_image(self: inpaint_node_t, from: i32): image_t {
	let source: image_t = logic_node_input_get_as_image(self.base.inputs[0]);
	console_progress(tr("Processing") + " - " + tr("Inpaint"));

	g2_begin(inpaint_node_image);
	g2_draw_scaled_image(source, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
	g2_end();

	if (inpaint_node_auto) {
		return inpaint_node_texsynth_inpaint(inpaint_node_image, false, inpaint_node_mask);
	}
	else {
		return inpaint_node_sd_inpaint(inpaint_node_image, inpaint_node_mask);
	}
}

function inpaint_node_get_cached_image(self: inpaint_node_t): image_t {
	app_notify_on_next_frame(function (self: inpaint_node_t) {
		let source: image_t = logic_node_input_get_as_image(self.base.inputs[0]);
		g4_begin(inpaint_node_image);
		g4_set_pipeline(pipes_inpaint_preview);
		g4_set_tex(pipes_tex0_inpaint_preview, source);
		g4_set_tex(pipes_texa_inpaint_preview, inpaint_node_mask);
		g4_set_vertex_buffer(const_data_screen_aligned_vb);
		g4_set_index_buffer(const_data_screen_aligned_ib);
		g4_draw();
		g4_end();
	}, self);
	return inpaint_node_image;
}

function inpaint_node_get_target(): image_t {
	return inpaint_node_mask;
}

function inpaint_node_texsynth_inpaint(image: image_t, tiling: bool, mask: image_t): image_t {
	let w: i32 = config_get_texture_res_x();
	let h: i32 = config_get_texture_res_y();

	let bytes_img: buffer_t = image_get_pixels(image);
	let bytes_mask: buffer_t = mask != null ? image_get_pixels(mask) : buffer_create(w * h);
	let bytes_out: buffer_t = buffer_create(w * h * 4);
	texsynth_inpaint(w, h, bytes_out.buffer, bytes_img.buffer, bytes_mask.buffer, tiling);

	inpaint_node_result = image_from_bytes(bytes_out, w, h);
	return inpaint_node_result;
}

function inpaint_node_sd_inpaint(image: image_t, mask: image_t): image_t {
	inpaint_node_init();

	let bytes_img: buffer_t = image_get_pixels(mask);
	let u8_img: buffer_t = bytes_img;
	let f32mask: f32_array_t = f32_array_create(4 * 64 * 64);

	let vae_encoder_blob: buffer_t = data_get_blob("models/sd_vae_encoder.quant.onnx");
	// for (let x: i32 = 0; x < math_floor(image.width / 512); ++x) {
		// for (let y: i32 = 0; y < math_floor(image.height / 512); ++y) {
			let x: i32 = 0;
			let y: i32 = 0;

			for (let xx: i32 = 0; xx < 64; ++xx) {
				for (let yy: i32 = 0; yy < 64; ++yy) {
					// let step = math_floor(512 / 64);
					// let j = (yy * step * mask.width + xx * step) + (y * 512 * mask.width + x * 512);
					let step: i32 = math_floor(mask.width / 64);
					let j: i32 = (yy * step * mask.width + xx * step);
					let f: f32 = u8_img[j] / 255.0;
					let i: i32 = yy * 64 + xx;
					f32mask[i              ] = f;
					f32mask[i + 64 * 64    ] = f;
					f32mask[i + 64 * 64 * 2] = f;
					f32mask[i + 64 * 64 * 3] = f;
				}
			}

			g2_begin(inpaint_node_temp);
			// g2_drawImage(image, -x * 512, -y * 512);
			g2_draw_scaled_image(image, 0, 0, 512, 512);
			g2_end();

			bytes_img = image_get_pixels(inpaint_node_temp);
			let u8a: buffer_t = bytes_img;
			let f32a: f32_array_t = f32_array_create(3 * 512 * 512);
			for (let i: i32 = 0; i < (512 * 512); ++i) {
				f32a[i                ] = (u8a[i * 4    ] / 255.0) * 2.0 - 1.0;
				f32a[i + 512 * 512    ] = (u8a[i * 4 + 1] / 255.0) * 2.0 - 1.0;
				f32a[i + 512 * 512 * 2] = (u8a[i * 4 + 2] / 255.0) * 2.0 - 1.0;
			}

			let tensors: buffer_t[] = [buffer_create_from_raw(f32a.buffer, f32a.length * 4)];
			let input_shape: i32_array_t[] = [];
			let input_shape0: i32[] = [1, 3, 512, 512];
			array_push(input_shape, input_shape0);
			let output_shape: i32[] = [1, 4, 64, 64];
			let latents_buf: buffer_t = iron_ml_inference(vae_encoder_blob, tensors, input_shape, output_shape, config_raw.gpu_inference);
			let latents: f32_array_t = f32_array_create_from_buffer(latents_buf);
			for (let i: i32 = 0; i < latents.length; ++i) {
				latents[i] = 0.18215 * latents[i];
			}
			let latents_orig: f32_array_t = array_slice(latents, 0, latents.length);

			let noise: f32_array_t = f32_array_create(latents.length);
			for (let i: i32 = 0; i < noise.length; ++i) {
				noise[i] = math_cos(2.0 * 3.14 * random_node_get_float()) * math_sqrt(-2.0 * math_log(random_node_get_float()));
			}

			let num_inference_steps: i32 = 50;
			let init_timestep: i32 = math_floor(num_inference_steps * inpaint_node_strength);
			let timestep: i32 = text_to_photo_node_timesteps[num_inference_steps - init_timestep];
			let alphas_cumprod: f32[] = text_to_photo_node_alphas_cumprod;
			let sqrt_alpha_prod: f32 = math_pow(alphas_cumprod[timestep], 0.5);
			let sqrt_one_minus_alpha_prod: f32 = math_pow(1.0 - alphas_cumprod[timestep], 0.5);
			for (let i: i32 = 0; i < latents.length; ++i) {
				latents[i] = sqrt_alpha_prod * latents[i] + sqrt_one_minus_alpha_prod * noise[i];
			}

			let start: i32 = num_inference_steps - init_timestep;

			inpaint_node_result = text_to_photo_node_stable_diffusion(inpaint_node_prompt, latents, start, true, f32mask, latents_orig);
			return inpaint_node_result;
		// }
	// }
}

let inpaint_node_def: ui_node_t = {
	id: 0,
	name: _tr("Inpaint"),
	type: "inpaint_node",
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
			default_value: f32_array_create_xyzw(1.0, 1.0, 1.0, 1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Color"),
			type: "RGBA",
			color: 0xffc7c729,
			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [
		{
			name: _tr("auto"),
			type: "BOOL",
			output: 0,
			default_value: f32_array_create_x(1),
			data: null,
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		},
		{
			name: "inpaint_node_button",
			type: "CUSTOM",
			output: -1,
			default_value: f32_array_create_x(0),
			data: null,
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		}
	],
	width: 0
};
