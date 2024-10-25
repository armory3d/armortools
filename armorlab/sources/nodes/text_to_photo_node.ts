
type text_to_photo_node_t = {
	base?: logic_node_t;
};

let text_to_photo_node_prompt: string = "";
let text_to_photo_node_image: image_t = null;
let text_to_photo_node_tiling: bool = false;
let text_to_photo_node_text_encoder_blob: buffer_t;
let text_to_photo_node_unet_blob: buffer_t;
let text_to_photo_node_vae_decoder_blob: buffer_t;

function text_to_photo_node_create(raw: ui_node_t, args: f32_array_t): text_to_photo_node_t {
	let n: text_to_photo_node_t = {};
	n.base = logic_node_create(n);
	n.base.get_as_image = text_to_photo_node_get_as_image;
	n.base.get_cached_image = text_to_photo_node_get_cached_image;
	return n;
}

function text_to_photo_node_get_as_image(self: text_to_photo_node_t, from: i32): image_t {
	text_to_photo_node_image = text_to_photo_node_stable_diffusion(text_to_photo_node_prompt);
	return text_to_photo_node_image;
}

function text_to_photo_node_get_cached_image(self: text_to_photo_node_t): image_t {
	return text_to_photo_node_image;
}

function text_to_photo_node_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	text_to_photo_node_tiling = node.buttons[0].default_value[0] == 0 ? false : true;
	text_to_photo_node_prompt = ui_text_area(ui_handle(__ID__), ui_align_t.LEFT, true, tr("prompt"), true);
	node.buttons[1].height = string_split(text_to_photo_node_prompt, "\n").length;
}

function text_to_photo_node_stable_diffusion(prompt: string, inpaint_latents: f32_array_t = null, offset: i32 = 0, upscale: bool = true, mask: f32_array_t = null, latents_orig: f32_array_t = null): image_t {
	let _text_encoder_blob: buffer_t = data_get_blob("models/sd_text_encoder.quant.onnx");
	let _unet_blob: buffer_t = data_get_blob("models/sd_unet.quant.onnx");
	let _vae_decoder_blob: buffer_t = data_get_blob("models/sd_vae_decoder.quant.onnx");
	text_to_photo_node_text_encoder_blob = _text_encoder_blob;
	text_to_photo_node_unet_blob = _unet_blob;
	text_to_photo_node_vae_decoder_blob = _vae_decoder_blob;
	let enc: text_encoder_result_t = text_to_photo_node_text_encoder(prompt, inpaint_latents);
	let latents: f32_array_t = text_to_photo_node_unet(enc.latents, enc.text_embeddings, mask, latents_orig, offset);
	return text_to_photo_node_vae_decoder(latents, upscale);
}

type text_encoder_result_t = {
	latents?: f32_array_t;
	text_embeddings?: f32_array_t;
};

function text_to_photo_node_text_encoder(prompt: string, inpaint_latents: f32_array_t): text_encoder_result_t {
	console_progress(tr("Processing") + " - " + tr("Text to Photo"));

	let words: string[] = string_split(
		trim_end(string_replace_all(string_replace_all(string_replace_all(prompt, "\n", " "), ",", " , "), "  ", " ")), " "
	);

	if (text_to_photo_node_vocab == null) {
		let vocab_buffer: buffer_t = data_get_blob("models/vocab.json");
		let vocab_json: string = sys_buffer_to_string(vocab_buffer);
		text_to_photo_node_vocab = json_parse_to_map(vocab_json);
	}

	for (let i: i32 = 0; i < words.length; ++i) {
		let word: string = to_lower_case(words[i]) + "</w>";
		let value_string: string = map_get(text_to_photo_node_vocab, word);
		let value: i32 = parse_int(value_string);
		text_to_photo_node_text_input_ids[i + 1] = value;
	}
	for (let i: i32 = words.length; i < (text_to_photo_node_text_input_ids.length - 1); ++i) {
		text_to_photo_node_text_input_ids[i + 1] = 49407; // <|endoftext|>
	}

	let i32a: i32_array_t = i32_array_create_from_array(text_to_photo_node_text_input_ids);
	let tensors: buffer_t[] = [buffer_create_from_raw(i32a.buffer, i32a.length * 4)];
	let input_shape: i32_array_t[] = [];
	let input_shape0: i32[] = [1, 77];
	array_push(input_shape, input_shape0);
	let output_shape: i32[] = [1, 77, 768];
	let text_embeddings_buf: buffer_t = iron_ml_inference(text_to_photo_node_text_encoder_blob, tensors, input_shape, output_shape, config_raw.gpu_inference);
	let text_embeddings: f32_array_t = f32_array_create_from_buffer(text_embeddings_buf);

	i32a = i32_array_create_from_array(text_to_photo_node_uncond_input_ids);
	tensors = [buffer_create_from_raw(i32a.buffer, i32a.length * 4)];
	let uncond_embeddings_buf: buffer_t = iron_ml_inference(text_to_photo_node_text_encoder_blob, tensors, input_shape, output_shape, config_raw.gpu_inference);
	let uncond_embeddings: f32_array_t = f32_array_create_from_buffer(uncond_embeddings_buf);

	let f32a: f32_array_t = f32_array_create(uncond_embeddings.length + text_embeddings.length);
	for (let i: i32 = 0; i < uncond_embeddings.length; ++i) f32a[i] = uncond_embeddings[i];
	for (let i: i32 = 0; i < text_embeddings.length; ++i) f32a[i + uncond_embeddings.length] = text_embeddings[i];
	text_embeddings = f32a;

	let width: i32 = 512;
	let height: i32 = 512;
	let latents: f32_array_t = f32_array_create(1 * 4 * math_floor(height / 8) * math_floor(width / 8));
	if (inpaint_latents == null) {
		for (let i: i32 = 0; i < latents.length; ++i) latents[i] = math_cos(2.0 * 3.14 * random_node_get_float()) * math_sqrt(-2.0 * math_log(random_node_get_float()));
	}
	else {
		for (let i: i32 = 0; i < latents.length; ++i) latents[i] = inpaint_latents[i];
	}

	let res: text_encoder_result_t = {
		latents: latents,
		text_embeddings: text_embeddings
	};
	return res;
}

function text_to_photo_node_unet(latents: f32_array_t, text_embeddings: f32_array_t, mask: f32_array_t, latents_orig: f32_array_t, offset: i32): f32_array_t {
	let latent_model_input: f32_array_t = f32_array_create(latents.length * 2);
	let noise_pred_uncond: f32_array_t = f32_array_create(latents.length);
	let noise_pred_text: f32_array_t = f32_array_create(latents.length);

	let cur_latents: f32_array_t = null;
	let num_train_timesteps: i32 = 1000;
	let num_inference_steps: i32 = 50;
	let ets: f32_array_t[] = [];
	let counter: i32 = 0;

	while (true) {
		let a: i32 = counter;
		let b: i32 = 50 - offset;
		console_progress(tr("Processing") + " - " + tr("Text to Photo") + " (" + a + "/" + b + ")");

		let timestep: i32 = text_to_photo_node_timesteps[counter + offset];
		for (let i: i32 = 0; i < latents.length; ++i) latent_model_input[i] = latents[i];
		for (let i: i32 = 0; i < latents.length; ++i) latent_model_input[i + latents.length] = latents[i];

		let t32: i32_array_t = i32_array_create(2);
		t32[0] = timestep;
		let tensors: buffer_t[] = [
			buffer_create_from_raw(latent_model_input.buffer, latent_model_input.length * 4),
			buffer_create_from_raw(t32.buffer, t32.length * 4),
			buffer_create_from_raw(text_embeddings.buffer, text_embeddings.length * 4),
		];
		let input_shape: i32_array_t[] = [];
		let input_shape0: i32[] = [2, 4, 64, 64];
		let input_shape1: i32[] = [1];
		let input_shape2: i32[] = [2, 77, 768];
		array_push(input_shape, input_shape0);
		array_push(input_shape, input_shape1);
		array_push(input_shape, input_shape2);
		let output_shape: i32[] = [2, 4, 64, 64];
		let noise_pred_buf: buffer_t = iron_ml_inference(text_to_photo_node_unet_blob, tensors, input_shape, output_shape, config_raw.gpu_inference);
		let noise_pred: f32_array_t = f32_array_create_from_buffer(noise_pred_buf);

		for (let i: i32 = 0; i < noise_pred_uncond.length; ++i) noise_pred_uncond[i] = noise_pred[i];
		for (let i: i32 = 0; i < noise_pred_text.length; ++i) noise_pred_text[i] = noise_pred[noise_pred_uncond.length + i];

		let guidance_scale: f32 = 7.5;
		noise_pred = f32_array_create(noise_pred_uncond.length);
		for (let i: i32 = 0; i < noise_pred_uncond.length; ++i) {
			noise_pred[i] = noise_pred_uncond[i] + guidance_scale * (noise_pred_text[i] - noise_pred_uncond[i]);
		}

		let prev_timestep: i32 = math_floor(math_max(timestep - math_floor(num_train_timesteps / num_inference_steps), 0));

		if (counter != 1) {
			array_push(ets, noise_pred);
		}
		else {
			prev_timestep = timestep;
			timestep = timestep + math_floor(num_train_timesteps / num_inference_steps);
		}

		if (ets.length == 1 && counter == 0) {
			cur_latents = latents;
		}
		else if (ets.length == 1 && counter == 1) {
			let _noise_pred: f32_array_t = f32_array_create(noise_pred.length);
			for (let i: i32 = 0; i < noise_pred.length; ++i) {
				_noise_pred[i] = (noise_pred[i] + ets[ets.length - 1][i]) / 2;
			}
			noise_pred = _noise_pred;
			latents = cur_latents;
			cur_latents = null;
		}
		else if (ets.length == 2) {
			let _noise_pred: f32_array_t = f32_array_create(noise_pred.length);
			for (let i: i32 = 0; i < noise_pred.length; ++i) {
				_noise_pred[i] = (3 * ets[ets.length - 1][i] - ets[ets.length - 2][i]) / 2;
			}
			noise_pred = _noise_pred;
		}
		else if (ets.length == 3) {
			let _noise_pred: f32_array_t = f32_array_create(noise_pred.length);
			for (let i: i32 = 0; i < noise_pred.length; ++i) {
				_noise_pred[i] = (23 * ets[ets.length - 1][i] - 16 * ets[ets.length - 2][i] + 5 * ets[ets.length - 3][i]) / 12;
			}
			noise_pred = _noise_pred;
		}
		else {
			let _noise_pred: f32_array_t = f32_array_create(noise_pred.length);
			for (let i: i32 = 0; i < noise_pred.length; ++i) {
				_noise_pred[i] = (1 / 24) * (55 * ets[ets.length - 1][i] - 59 * ets[ets.length - 2][i] + 37 * ets[ets.length - 3][i] - 9 * ets[ets.length - 4][i]);
			}
			noise_pred = _noise_pred;
		}

		let alpha_prod_t: f32 = text_to_photo_node_alphas_cumprod[timestep + 1];
		let alpha_prod_t_prev: f32 = text_to_photo_node_alphas_cumprod[prev_timestep + 1];
		let beta_prod_t: f32 = 1 - alpha_prod_t;
		let beta_prod_t_prev: f32 = 1 - alpha_prod_t_prev;
		let latents_coeff: f32 = math_pow(alpha_prod_t_prev / alpha_prod_t, (0.5));
		let noise_pred_denom_coeff: f32 = alpha_prod_t * math_pow(beta_prod_t_prev, (0.5)) + math_pow(alpha_prod_t * beta_prod_t * alpha_prod_t_prev, (0.5));
		for (let i: i32 = 0; i < latents.length; ++i) {
			latents[i] = (latents_coeff * latents[i] - (alpha_prod_t_prev - alpha_prod_t) * noise_pred[i] / noise_pred_denom_coeff);
		}
		counter += 1;

		if (mask != null) {
			let noise: f32_array_t = f32_array_create(latents.length);
			for (let i: i32 = 0; i < noise.length; ++i) {
				noise[i] = math_cos(2.0 * 3.14 * random_node_get_float()) * math_sqrt(-2.0 * math_log(random_node_get_float()));
			}
			let sqrt_alpha_prod: f32 = math_pow(text_to_photo_node_alphas_cumprod[timestep], 0.5);
			let sqrt_one_minus_alpha_prod: f32 = math_pow(1.0 - text_to_photo_node_alphas_cumprod[timestep], 0.5);

			let init_latents_proper: f32_array_t = f32_array_create(latents.length);
			for (let i: i32 = 0; i < init_latents_proper.length; ++i) {
				init_latents_proper[i] = sqrt_alpha_prod * latents_orig[i] + sqrt_one_minus_alpha_prod * noise[i];
			}

			for (let i: i32 = 0; i < latents.length; ++i) {
				latents[i] = (init_latents_proper[i] * mask[i]) + (latents[i] * (1.0 - mask[i]));
			}
		}

		if (counter == (51 - offset)) {
			break;
		}
	}

	return latents;
}

function text_to_photo_node_vae_decoder(latents: f32_array_t, upscale: bool): image_t {
	console_progress(tr("Processing") + " - " + tr("Text to Photo"));

	for (let i: i32 = 0; i < latents.length; ++i) {
		latents[i] = 1.0 / 0.18215 * latents[i];
	}

	let tensors: buffer_t[] = [buffer_create_from_raw(latents.buffer, latents.length * 4)];
	let input_shape: i32_array_t[] = [];
	let input_shape0: i32[] = [1, 4, 64, 64];
	array_push(input_shape, input_shape0);
	let output_shape: i32[] = [1, 3, 512, 512];
	let pyimage_buf: buffer_t = iron_ml_inference(text_to_photo_node_vae_decoder_blob, tensors, input_shape, output_shape, config_raw.gpu_inference);
	let pyimage: f32_array_t = f32_array_create_from_buffer(pyimage_buf);

	for (let i: i32 = 0; i < pyimage.length; ++i) {
		pyimage[i] = pyimage[i] / 2.0 + 0.5;
		if (pyimage[i] < 0) pyimage[i] = 0;
		else if (pyimage[i] > 1) pyimage[i] = 1;
	}

	let u8a: u8_array_t = u8_array_create(4 * 512 * 512);
	for (let i: i32 = 0; i < (512 * 512); ++i) {
		u8a[i * 4    ] = math_floor(pyimage[i                ] * 255);
		u8a[i * 4 + 1] = math_floor(pyimage[i + 512 * 512    ] * 255);
		u8a[i * 4 + 2] = math_floor(pyimage[i + 512 * 512 * 2] * 255);
		u8a[i * 4 + 3] = 255;
	}
	let image: image_t = image_from_bytes(u8a, 512, 512);

	if (text_to_photo_node_tiling) {
		tiling_node_prompt = text_to_photo_node_prompt;
		let seed: i32 = random_node_get_seed();
		return tiling_node_sd_tiling(image, seed);
	}
	else {
		if (upscale) {
			upscale_node_load_blob();
			while (image.width < config_get_texture_res_x()) {
				let last_image: image_t = image;
				image = upscale_node_esrgan(image);
				image_unload(last_image);
			}
			return image;
		}
		else {
			return image;
		}
	}
}

let text_to_photo_node_def: ui_node_t = {
	id: 0,
	name: _tr("Text to Photo"),
	type: "text_to_photo_node",
	x: 0,
	y: 0,
	color: 0xff4982a0,
	inputs: [],
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
			name: _tr("tiling"),
			type: "BOOL",
			output: 0,
			default_value: f32_array_create_x(0),
			data: null,
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		},
		{
			name: "text_to_photo_node_button",
			type: "CUSTOM",
			output: -1,
			default_value: f32_array_create_x(0),
			data: null,
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 1
		}
	],
	width: 0
};

let text_to_photo_node_text_input_ids: i32[] = [49406, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407];

let text_to_photo_node_uncond_input_ids: i32[] = [49406, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407, 49407,
	49407, 49407, 49407, 49407, 49407];

let text_to_photo_node_alphas_cumprod: f32[] = [0.99915, 0.998296, 0.9974381, 0.99657613, 0.99571025, 0.9948404,
	0.9939665,  0.99308866, 0.9922069,  0.9913211,  0.9904313,  0.98953754,
	0.9886398,  0.9877381,  0.9868324,  0.9859227,  0.985009,   0.98409134,
	0.9831697,  0.982244,   0.98131436, 0.9803807,  0.97944313, 0.97850156,
	0.977556,   0.9766064,  0.9756529,  0.9746954,  0.9737339,  0.9727684,
	0.97179896, 0.97082555, 0.96984816, 0.96886677, 0.9678814,  0.96689206,
	0.9658988,  0.96490157, 0.9639003,  0.96289515, 0.961886,   0.9608729,
	0.9598558,  0.9588347,  0.9578097,  0.95678073, 0.95574784, 0.95471096,
	0.95367014, 0.9526254,  0.95157677, 0.9505242,  0.9494677,  0.9484073,
	0.94734293, 0.94627464, 0.9452024,  0.9441263,  0.9430463,  0.94196236,
	0.9408745,  0.9397828,  0.9386872,  0.93758774, 0.93648434, 0.93537706,
	0.9342659,  0.9331509,  0.93203205, 0.93090934, 0.9297828,  0.92865235,
	0.92751807, 0.92638,    0.9252381,  0.9240923,  0.9229427,  0.92178935,
	0.9206321,  0.9194711,  0.9183063,  0.9171377,  0.9159653,  0.91478914,
	0.9136092,  0.9124255,  0.9112381,  0.9100469,  0.908852,   0.90765333,
	0.90645087, 0.9052447,  0.90403485, 0.9028213,  0.90160406, 0.9003831,
	0.8991585,  0.89793015, 0.8966982,  0.8954625,  0.8942232,  0.8929803,
	0.8917337,  0.8904835,  0.88922966, 0.88797224, 0.8867112,  0.88544655,
	0.88417834, 0.88290656, 0.8816312,  0.8803523,  0.87906986, 0.8777839,
	0.87649435, 0.87520134, 0.8739048,  0.8726048,  0.8713013,  0.8699943,
	0.8686838,  0.86736983, 0.86605245, 0.8647316,  0.8634073,  0.8620797,
	0.86074865, 0.85941416, 0.85807633, 0.8567351,  0.85539055, 0.8540426,
	0.8526913,  0.8513367,  0.8499788,  0.8486176,  0.84725314, 0.84588534,
	0.8445143,  0.84314,    0.8417625,  0.84038174, 0.8389977,  0.83761054,
	0.83622015, 0.8348266,  0.8334298,  0.83202994, 0.8306269,  0.8292208,
	0.8278115,  0.8263991,  0.82498366, 0.8235651,  0.82214355, 0.8207189,
	0.8192912,  0.8178604,  0.8164267,  0.81499,    0.8135503,  0.8121076,
	0.810662,   0.8092134,  0.8077619,  0.80630755, 0.8048503,  0.80339015,
	0.80192715, 0.8004613,  0.7989926,  0.79752105, 0.79604673, 0.7945696,
	0.79308975, 0.79160714, 0.79012173, 0.78863364, 0.7871428,  0.7856493,
	0.78415316, 0.78265435, 0.78115284, 0.7796487,  0.778142,   0.77663267,
	0.77512074, 0.77360624, 0.7720892,  0.7705696,  0.7690475,  0.7675229,
	0.7659958,  0.7644662,  0.76293427, 0.76139987, 0.759863,   0.7583238,
	0.7567822,  0.7552382,  0.75369185, 0.7521432,  0.7505923,  0.74903905,
	0.74748355, 0.7459258,  0.74436575, 0.7428035,  0.7412391,  0.7396724,
	0.7381036,  0.7365327,  0.73495966, 0.7333845,  0.73180723, 0.7302279,
	0.7286465,  0.72706306, 0.7254776,  0.7238901,  0.72230065, 0.72070926,
	0.7191159,  0.71752065, 0.7159235,  0.7143244,  0.7127235,  0.7111207,
	0.7095161,  0.7079097,  0.7063015,  0.7046916,  0.7030799,  0.70146644,
	0.6998513,  0.69823444, 0.69661593, 0.69499576, 0.693374,   0.6917506,
	0.6901256,  0.68849903, 0.6868709,  0.6852412,  0.6836101,  0.6819774,
	0.6803433,  0.6787077,  0.67707074, 0.6754323,  0.67379254, 0.6721514,
	0.67050886, 0.668865,   0.6672199,  0.6655735,  0.6639258,  0.6622769,
	0.6606268,  0.6589755,  0.657323,   0.65566933, 0.6540145,  0.6523586,
	0.6507016,  0.6490435,  0.64738435, 0.6457242,  0.644063,   0.6424008,
	0.64073765, 0.63907355, 0.6374085,  0.63574255, 0.63407576, 0.6324081,
	0.63073957, 0.6290703,  0.6274002,  0.6257293,  0.6240577,  0.6223853,
	0.6207122,  0.61903846, 0.61736405, 0.615689,   0.6140133,  0.61233705,
	0.6106602,  0.6089828,  0.6073049,  0.6056264,  0.60394746, 0.60226804,
	0.6005882,  0.59890795, 0.5972273,  0.59554625, 0.59386486, 0.5921831,
	0.59050107, 0.5888187,  0.5871361,  0.5854532,  0.5837701,  0.5820868,
	0.5804033,  0.57871974, 0.57703596, 0.5753521,  0.57366806, 0.571984,
	0.5702999,  0.5686158,  0.56693166, 0.56524754, 0.56356347, 0.56187946,
	0.56019557, 0.55851173, 0.5568281,  0.55514455, 0.5534612,  0.551778,
	0.5500951,  0.5484124,  0.5467299,  0.54504776, 0.5433659,  0.5416844,
	0.5400032,  0.5383224,  0.536642,   0.534962,   0.53328246, 0.53160334,
	0.52992475, 0.52824664, 0.52656907, 0.52489203, 0.5232156,  0.5215397,
	0.51986444, 0.5181898,  0.51651585, 0.51484257, 0.51317,    0.51149815,
	0.509827,   0.50815666, 0.5064871,  0.5048183,  0.5031504,  0.5014833,
	0.49981716, 0.49815187, 0.4964875,  0.49482408, 0.49316162, 0.49150014,
	0.48983967, 0.48818022, 0.48652178, 0.48486444, 0.48320818, 0.48155302,
	0.479899,   0.47824612, 0.47659442, 0.4749439,  0.47329462, 0.47164655,
	0.46999976, 0.46835423, 0.46671,    0.4650671,  0.46342552, 0.46178532,
	0.46014652, 0.45850912, 0.45687312, 0.45523855, 0.45360544, 0.45197386,
	0.45034376, 0.44871515, 0.44708812, 0.44546264, 0.44383875, 0.44221646,
	0.4405958,  0.4389768,  0.43735942, 0.43574375, 0.43412977, 0.43251753,
	0.430907,   0.42929825, 0.42769128, 0.42608613, 0.4244828,  0.4228813,
	0.42128167, 0.4196839,  0.41808805, 0.4164941,  0.4149021,  0.41331202,
	0.41172394, 0.41013786, 0.40855378, 0.40697172, 0.40539172, 0.40381378,
	0.40223792, 0.40066415, 0.39909253, 0.39752305, 0.3959557,  0.39439055,
	0.3928276,  0.39126685, 0.38970834, 0.38815206, 0.38659805, 0.38504633,
	0.3834969,  0.3819498,  0.38040504, 0.37886262, 0.37732255, 0.37578487,
	0.3742496,  0.37271675, 0.37118635, 0.36965838, 0.3681329,  0.3666099,
	0.3650894,  0.3635714,  0.36205596, 0.36054307, 0.35903275, 0.35752502,
	0.35601988, 0.35451737, 0.35301748, 0.35152024, 0.35002568, 0.34853378,
	0.3470446,  0.34555808, 0.3440743,  0.34259328, 0.341115,   0.3396395,
	0.3381668,  0.3366969,  0.33522978, 0.3337655,  0.3323041,  0.3308455,
	0.3293898,  0.327937,   0.3264871,  0.3250401,  0.32359603, 0.3221549,
	0.32071674, 0.31928152, 0.3178493,  0.3164201,  0.3149939,  0.3135707,
	0.31215054, 0.31073344, 0.3093194,  0.30790845, 0.30650055, 0.30509576,
	0.3036941,  0.30229557, 0.30090016, 0.2995079,  0.29811877, 0.29673284,
	0.2953501,  0.29397056, 0.29259422, 0.2912211,  0.28985122, 0.28848457,
	0.28712118, 0.28576106, 0.28440422, 0.28305066, 0.2817004,  0.28035346,
	0.27900982, 0.27766952, 0.27633256, 0.27499893, 0.27366868, 0.2723418,
	0.27101827, 0.26969814, 0.26838142, 0.26706812, 0.26575825, 0.26445177,
	0.26314875, 0.26184916, 0.26055303, 0.25926036, 0.25797117, 0.25668547,
	0.25540325, 0.25412452, 0.2528493,  0.25157762, 0.25030944, 0.24904479,
	0.24778369, 0.24652614, 0.24527213, 0.2440217,  0.24277483, 0.24153154,
	0.24029182, 0.2390557,  0.23782317, 0.23659426, 0.23536895, 0.23414725,
	0.23292919, 0.23171476, 0.23050396, 0.2292968,  0.2280933,  0.22689344,
	0.22569725, 0.22450472, 0.22331588, 0.22213072, 0.22094923, 0.21977143,
	0.21859734, 0.21742693, 0.21626024, 0.21509725, 0.21393798, 0.21278243,
	0.2116306,  0.2104825,  0.20933813, 0.20819749, 0.2070606,  0.20592746,
	0.20479806, 0.20367241, 0.20255052, 0.20143238, 0.200318,   0.19920738,
	0.19810054, 0.19699748, 0.19589819, 0.19480269, 0.19371095, 0.192623,
	0.19153884, 0.19045846, 0.18938187, 0.18830907, 0.18724008, 0.18617487,
	0.18511346, 0.18405585, 0.18300205, 0.18195206, 0.18090586, 0.17986348,
	0.1788249,  0.17779014, 0.17675918, 0.17573205, 0.17470871, 0.17368919,
	0.17267348, 0.17166159, 0.1706535,  0.16964924, 0.1686488,  0.16765216,
	0.16665934, 0.16567034, 0.16468513, 0.16370374, 0.16272618, 0.16175242,
	0.16078247, 0.15981634, 0.15885401, 0.1578955,  0.1569408,  0.15598992,
	0.15504283, 0.15409954, 0.15316005, 0.15222436, 0.15129249, 0.15036441,
	0.14944012, 0.14851964, 0.14760293, 0.14669003, 0.1457809,  0.14487557,
	0.14397402, 0.14307626, 0.14218228, 0.14129207, 0.14040563, 0.13952295,
	0.13864405, 0.13776892, 0.13689755, 0.13602993, 0.13516606, 0.13430595,
	0.13344958, 0.13259697, 0.1317481,  0.13090296, 0.13006155, 0.12922388,
	0.12838994, 0.1275597,  0.1267332,  0.1259104,  0.12509131, 0.12427593,
	0.12346424, 0.12265625, 0.12185195, 0.12105133, 0.1202544,  0.11946114,
	0.11867155, 0.11788563, 0.11710336, 0.11632475, 0.11554979, 0.11477847,
	0.1140108,  0.11324675, 0.11248633, 0.11172953, 0.11097635, 0.11022678,
	0.10948081, 0.10873844, 0.10799967, 0.10726449, 0.10653288, 0.10580485,
	0.10508038, 0.10435947, 0.10364211, 0.1029283,  0.10221803, 0.1015113,
	0.10080809, 0.10010841, 0.09941223, 0.09871957, 0.0980304,  0.09734473,
	0.09666254, 0.09598383, 0.09530859, 0.09463682, 0.0939685,  0.09330362,
	0.09264219, 0.09198419, 0.09132962, 0.09067846, 0.09003071, 0.08938637,
	0.08874542, 0.08810785, 0.08747366, 0.08684284, 0.08621538, 0.08559129,
	0.08497053, 0.0843531,  0.08373901, 0.08312824, 0.08252078, 0.08191663,
	0.08131576, 0.08071819, 0.08012389, 0.07953286, 0.07894509, 0.07836057,
	0.07777929, 0.07720125, 0.07662643, 0.07605482, 0.07548642, 0.07492122,
	0.07435921, 0.07380038, 0.07324471, 0.07269221, 0.07214285, 0.07159664,
	0.07105356, 0.07051361, 0.06997676, 0.06944302, 0.06891238, 0.06838482,
	0.06786034, 0.06733891, 0.06682055, 0.06630524, 0.06579296, 0.06528371,
	0.06477747, 0.06427424, 0.06377401, 0.06327677, 0.0627825,  0.06229121,
	0.06180287, 0.06131747, 0.06083502, 0.06035549, 0.05987888, 0.05940517,
	0.05893436, 0.05846644, 0.05800139, 0.05753921, 0.05707989, 0.05662341,
	0.05616977, 0.05571895, 0.05527094, 0.05482575, 0.05438334, 0.05394372,
	0.05350687, 0.05307278, 0.05264145, 0.05221286, 0.05178699, 0.05136385,
	0.05094342, 0.05052568, 0.05011064, 0.04969827, 0.04928857, 0.04888153,
	0.04847714, 0.04807537, 0.04767624, 0.04727972, 0.0468858,  0.04649448,
	0.04610574, 0.04571956, 0.04533596, 0.0449549,  0.04457638, 0.04420039,
	0.04382691, 0.04345594, 0.04308747, 0.04272148, 0.04235797, 0.04199693,
	0.04163833, 0.04128218, 0.04092846, 0.04057716, 0.04022827, 0.03988178,
	0.03953768, 0.03919596, 0.0388566,  0.0385196,  0.03818495, 0.03785263,
	0.03752263, 0.03719494, 0.03686956, 0.03654647, 0.03622566, 0.03590712,
	0.03559083, 0.0352768,  0.034965,   0.03465543, 0.03434808, 0.03404293,
	0.03373997, 0.0334392,  0.0331406,  0.03284416, 0.03254988, 0.03225773,
	0.03196772, 0.03167982, 0.03139404, 0.03111035, 0.03082875, 0.03054923,
	0.03027177, 0.02999637, 0.02972301, 0.02945168, 0.02918238, 0.0289151,
	0.02864981, 0.02838652, 0.02812521, 0.02786587, 0.02760849, 0.02735306,
	0.02709957, 0.02684801, 0.02659837, 0.02635064, 0.0261048,  0.02586086,
	0.02561878, 0.02537858, 0.02514023, 0.02490373, 0.02466906, 0.02443622,
	0.0242052,  0.02397598, 0.02374856, 0.02352292, 0.02329905, 0.02307695,
	0.02285661, 0.02263801, 0.02242114, 0.022206,   0.02199257, 0.02178084,
	0.02157081, 0.02136246, 0.02115579, 0.02095079, 0.02074743, 0.02054573,
	0.02034565, 0.0201472,  0.01995037, 0.01975514, 0.01956151, 0.01936947,
	0.019179,   0.0189901,  0.01880275, 0.01861695, 0.01843269, 0.01824996,
	0.01806875, 0.01788905, 0.01771084, 0.01753413, 0.0173589,  0.01718514,
	0.01701284, 0.016842,   0.0166726,  0.01650463, 0.0163381,  0.01617297,
	0.01600925, 0.01584694, 0.01568601, 0.01552646, 0.01536828, 0.01521146,
	0.015056,   0.01490187, 0.01474909, 0.01459763, 0.01444749, 0.01429865,
	0.01415112, 0.01400487, 0.01385991, 0.01371622, 0.0135738,  0.01343263,
	0.01329271, 0.01315403, 0.01301658, 0.01288035, 0.01274534, 0.01261153,
	0.01247892, 0.01234749, 0.01221725, 0.01208818, 0.01196027, 0.01183351,
	0.01170791, 0.01158344, 0.0114601,  0.01133789, 0.01121679, 0.0110968,
	0.01097791, 0.01086011, 0.01074339, 0.01062774, 0.01051317, 0.01039965,
	0.01028718, 0.01017576, 0.01006538, 0.00995602, 0.00984768, 0.00974036,
	0.00963405, 0.00952873, 0.0094244,  0.00932106, 0.00921869, 0.00911729,
	0.00901685, 0.00891737, 0.00881884, 0.00872124, 0.00862457, 0.00852883,
	0.00843401, 0.0083401,  0.0082471,  0.00815499, 0.00806377, 0.00797343,
	0.00788397, 0.00779538, 0.00770765, 0.00762078, 0.00753476, 0.00744958,
	0.00736523, 0.00728171, 0.00719902, 0.00711714, 0.00703607, 0.0069558,
	0.00687633, 0.00679765, 0.00671975, 0.00664263, 0.00656627, 0.00649069,
	0.00641586, 0.00634178, 0.00626845, 0.00619586, 0.006124,   0.00605286,
	0.00598245, 0.00591276, 0.00584377, 0.00577549, 0.0057079,  0.00564101,
	0.0055748,  0.00550927, 0.00544442, 0.00538023, 0.00531671, 0.00525384,
	0.00519163, 0.00513006, 0.00506913, 0.00500883, 0.00494917, 0.00489013,
	0.0048317,  0.00477389, 0.00471669, 0.00466009];

let text_to_photo_node_timesteps: i32[] = [981, 961, 961, 941, 921, 901, 881, 861, 841, 821, 801, 781, 761, 741, 721, 701, 681, 661,
	641, 621, 601, 581, 561, 541, 521, 501, 481, 461, 441, 421, 401, 381, 361, 341, 321, 301,
	281, 261, 241, 221, 201, 181, 161, 141, 121, 101, 81, 61, 41, 21, 1];

let text_to_photo_node_vocab: map_t<string, string> = null;
