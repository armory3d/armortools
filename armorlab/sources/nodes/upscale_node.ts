
type upscale_node_t = {
	base?: logic_node_t;
};

let upscale_node_temp: image_t = null;
let upscale_node_image: image_t = null;
let upscale_node_esrgan_blob: buffer_t;

function upscale_node_create(raw: ui_node_t, args: f32_array_t): upscale_node_t {
	let n: float_node_t = {};
	n.base = logic_node_create(n);
	n.base.get_as_image = upscale_node_get_as_image;
	n.base.get_cached_image = upscale_node_get_cached_image;
	return n;
}

function upscale_node_get_as_image(self: upscale_node_t, from: i32): image_t {
	upscale_node_image = logic_node_input_get_as_image(self.base.inputs[0]);

	console_progress(tr("Processing") + " - " + tr("Upscale"));

	upscale_node_load_blob();
	if (upscale_node_image.width < config_get_texture_res_x()) {
		upscale_node_image = upscale_node_esrgan(upscale_node_image);
		while (upscale_node_image.width < config_get_texture_res_x()) {
			let last_image: image_t = upscale_node_image;
			upscale_node_image = upscale_node_esrgan(upscale_node_image);
			image_unload(last_image);
		}
	}
	return upscale_node_image;
}

function upscale_node_load_blob() {
	upscale_node_esrgan_blob = data_get_blob("models/esrgan.quant.onnx");
}

function upscale_node_get_cached_image(self: upscale_node_t): image_t {
	return upscale_node_image;
}

function upscale_node_do_tile(source: image_t): image_t {
	let result: image_t = null;
	let size1w: i32 = source.width;
	let size1h: i32 = source.height;
	let size2w: i32 = math_floor(size1w * 2);
	let size2h: i32 = math_floor(size1h * 2);
	if (upscale_node_temp != null) {
		image_unload(upscale_node_temp);
	}
	upscale_node_temp = image_create_render_target(size1w, size1h);
	g2_begin(upscale_node_temp);
	g2_draw_scaled_image(source, 0, 0, size1w, size1h);
	g2_end();

	let bytes_img: buffer_t = image_get_pixels(upscale_node_temp);
	let u8a: u8_array_t = bytes_img;
	let f32a: f32_array_t = f32_array_create(3 * size1w * size1h);
	for (let i: i32 = 0; i < (size1w * size1h); ++i) {
		f32a[i                      ] = (u8a[i * 4    ] / 255);
		f32a[i + size1w * size1w    ] = (u8a[i * 4 + 1] / 255);
		f32a[i + size1w * size1w * 2] = (u8a[i * 4 + 2] / 255);
	}

	let tensors: buffer_t[] = [buffer_create_from_raw(f32a.buffer, f32a.length * 4)];
	let input_shape: i32_array_t[] = [];
	let input_shape0: i32[] = [1, 3, size1w, size1h];
	array_push(input_shape, input_shape0);
	let output_shape: i32[] = [1, 3, size2w, size2h];
	let esrgan2x_buf: buffer_t = iron_ml_inference(upscale_node_esrgan_blob, tensors, input_shape, output_shape, config_raw.gpu_inference);
	let esrgan2x: f32_array_t = f32_array_create_from_buffer(esrgan2x_buf);
	for (let i: i32 = 0; i < esrgan2x.length; ++i) {
		if (esrgan2x[i] < 0) {
			esrgan2x[i] = 0;
		}
		else if (esrgan2x[i] > 1) {
			esrgan2x[i] = 1;
		}
	}

	u8a = u8_array_create(4 * size2w * size2h);
	for (let i: i32 = 0; i < (size2w * size2h); ++i) {
		u8a[i * 4    ] = math_floor(esrgan2x[i                      ] * 255);
		u8a[i * 4 + 1] = math_floor(esrgan2x[i + size2w * size2w    ] * 255);
		u8a[i * 4 + 2] = math_floor(esrgan2x[i + size2w * size2w * 2] * 255);
		u8a[i * 4 + 3] = 255;
	}

	result = image_from_bytes(u8a, size2w, size2h);
	return result;
}

function upscale_node_esrgan(source: image_t): image_t {
	let result: image_t = null;
	let size1w: i32 = source.width;
	let size1h: i32 = source.height;
	let tile_size: i32 = 512;
	let tile_size2x: i32 = math_floor(tile_size * 2);

	if (size1w >= tile_size2x || size1h >= tile_size2x) { // Split into tiles
		let size2w: i32 = math_floor(size1w * 2);
		let size2h: i32 = math_floor(size1h * 2);
		result = image_create_render_target(size2w, size2h);
		let tile_source: image_t = image_create_render_target(tile_size + 32 * 2, tile_size + 32 * 2);
		for (let x: i32 = 0; x < math_floor(size1w / tile_size); ++x) {
			for (let y: i32 = 0; y < math_floor(size1h / tile_size); ++y) {
				g2_begin(tile_source);
				g2_draw_scaled_image(source, 32 - x * tile_size, 32 - y * tile_size, -source.width, source.height);
				g2_draw_scaled_image(source, 32 - x * tile_size, 32 - y * tile_size, source.width, -source.height);
				g2_draw_scaled_image(source, 32 - x * tile_size, 32 - y * tile_size, -source.width, -source.height);
				g2_draw_scaled_image(source, 32 - x * tile_size + tile_size, 32 - y * tile_size + tile_size, source.width, source.height);
				g2_draw_scaled_image(source, 32 - x * tile_size + tile_size, 32 - y * tile_size + tile_size, -source.width, source.height);
				g2_draw_scaled_image(source, 32 - x * tile_size + tile_size, 32 - y * tile_size + tile_size, source.width, -source.height);
				g2_draw_scaled_image(source, 32 - x * tile_size, 32 - y * tile_size, source.width, source.height);
				g2_end();
				let tile_result: image_t = upscale_node_do_tile(tile_source);
				g2_begin(result);
				g2_draw_sub_image(tile_result, x * tile_size2x, y * tile_size2x, 64, 64, tile_size2x, tile_size2x);
				g2_end();
				image_unload(tile_result);
			}
		}
		image_unload(tile_source);
	}
	else {
		result = upscale_node_do_tile(source); // Single tile
	}
	return result;
}

let upscale_node_def: ui_node_t = {
	id: 0,
	name: _tr("Upscale"),
	type: "upscale_node",
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
			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
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
	buttons: [],
	width: 0
};
