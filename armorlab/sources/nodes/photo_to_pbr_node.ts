
type photo_to_pbr_node_t = {
	base?: logic_node_t;
};

let photo_to_pbr_node_temp: image_t = null;
let photo_to_pbr_node_images: image_t[] = null;
let photo_to_pbr_node_model_names: string[] = ["base", "occlusion", "roughness", "metallic", "normal", "height"];

let photo_to_pbr_node_cached_source: image_t = null;
let photo_to_pbr_node_border_w: i32 = 64;
let photo_to_pbr_node_tile_w: i32 = 2048;
let photo_to_pbr_node_tile_with_border_w: i32 = photo_to_pbr_node_tile_w + photo_to_pbr_node_border_w * 2;

function photo_to_pbr_node_create(arg: any): photo_to_pbr_node_t {
	let n: photo_to_pbr_node_t = {};
	n.base = logic_node_create();
	n.base.get_as_image = photo_to_pbr_node_get_as_image;

	if (photo_to_pbr_node_temp == null) {
		photo_to_pbr_node_temp = image_create_render_target(photo_to_pbr_node_tile_with_border_w, photo_to_pbr_node_tile_with_border_w);
	}

	photo_to_pbr_node_init();

	return n;
}

function photo_to_pbr_node_init() {
	if (photo_to_pbr_node_images == null) {
		photo_to_pbr_node_images = [];
		for (let i: i32 = 0; i < photo_to_pbr_node_model_names.length; ++i) {
			array_push(photo_to_pbr_node_images, image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y()));
		}
	}
}

function photo_to_pbr_node_get_as_image(self: photo_to_pbr_node_t, from: i32): image_t {

	let source: image_t;
	if (photo_to_pbr_node_cached_source != null) {
		source = photo_to_pbr_node_cached_source;
	}
	else {
		source = logic_node_input_get_as_image(self.base.inputs[0]);
	}

	photo_to_pbr_node_cached_source = source;

	console_progress(tr("Processing") + " - " + tr("Photo to PBR"));
	iron_g4_swap_buffers();

	let tile_floats: f32_array_t[] = [];
	let tiles_x: i32 = math_floor(config_get_texture_res_x() / photo_to_pbr_node_tile_w);
	let tiles_y: i32 = math_floor(config_get_texture_res_y() / photo_to_pbr_node_tile_w);
	let num_tiles: i32 = tiles_x * tiles_y;
	for (let i: i32 = 0; i < num_tiles; ++i) {
		let x: i32 = i % tiles_x;
		let y: i32 = math_floor(i / tiles_x);

		g2_begin(photo_to_pbr_node_temp);
		g2_draw_scaled_image(source, photo_to_pbr_node_border_w - x * photo_to_pbr_node_tile_w, photo_to_pbr_node_border_w - y * photo_to_pbr_node_tile_w, -config_get_texture_res_x(), config_get_texture_res_y());
		g2_draw_scaled_image(source, photo_to_pbr_node_border_w - x * photo_to_pbr_node_tile_w, photo_to_pbr_node_border_w - y * photo_to_pbr_node_tile_w, config_get_texture_res_x(), -config_get_texture_res_y());
		g2_draw_scaled_image(source, photo_to_pbr_node_border_w - x * photo_to_pbr_node_tile_w, photo_to_pbr_node_border_w - y * photo_to_pbr_node_tile_w, -config_get_texture_res_x(), -config_get_texture_res_y());
		g2_draw_scaled_image(source, photo_to_pbr_node_border_w - x * photo_to_pbr_node_tile_w + photo_to_pbr_node_tile_w, photo_to_pbr_node_border_w - y * photo_to_pbr_node_tile_w + photo_to_pbr_node_tile_w, config_get_texture_res_x(), config_get_texture_res_y());
		g2_draw_scaled_image(source, photo_to_pbr_node_border_w - x * photo_to_pbr_node_tile_w + photo_to_pbr_node_tile_w, photo_to_pbr_node_border_w - y * photo_to_pbr_node_tile_w + photo_to_pbr_node_tile_w, -config_get_texture_res_x(), config_get_texture_res_y());
		g2_draw_scaled_image(source, photo_to_pbr_node_border_w - x * photo_to_pbr_node_tile_w + photo_to_pbr_node_tile_w, photo_to_pbr_node_border_w - y * photo_to_pbr_node_tile_w + photo_to_pbr_node_tile_w, config_get_texture_res_x(), -config_get_texture_res_y());
		g2_draw_scaled_image(source, photo_to_pbr_node_border_w - x * photo_to_pbr_node_tile_w, photo_to_pbr_node_border_w - y * photo_to_pbr_node_tile_w, config_get_texture_res_x(), config_get_texture_res_y());
		g2_end();

		let bytes_img: buffer_t = image_get_pixels(photo_to_pbr_node_temp);
		let u8a: buffer_t = bytes_img;
		let f32a: f32_array_t = f32_array_create(3 * photo_to_pbr_node_tile_with_border_w * photo_to_pbr_node_tile_with_border_w);
		for (let i: i32 = 0; i < (photo_to_pbr_node_tile_with_border_w * photo_to_pbr_node_tile_with_border_w); ++i) {
			f32a[i] = (u8a[i * 4] / 255 - 0.5) / 0.5;
			f32a[i + photo_to_pbr_node_tile_with_border_w * photo_to_pbr_node_tile_with_border_w    ] = (u8a[i * 4 + 1] / 255 - 0.5) / 0.5;
			f32a[i + photo_to_pbr_node_tile_with_border_w * photo_to_pbr_node_tile_with_border_w * 2] = (u8a[i * 4 + 2] / 255 - 0.5) / 0.5;
		}

		let model_blob: buffer_t = data_get_blob("models/photo_to_" + photo_to_pbr_node_model_names[from] + ".quant.onnx");
		let buf: buffer_t = iron_ml_inference(model_blob, [f32a.buffer], null, null, config_raw.gpu_inference);
		let ar: f32_array_t = f32_array_create_from_buffer(buf);
		u8a = u8_array_create(4 * photo_to_pbr_node_tile_w * photo_to_pbr_node_tile_w);
		let offset_g: i32 = (from == channel_type_t.BASE_COLOR || from == channel_type_t.NORMAL_MAP) ? photo_to_pbr_node_tile_with_border_w * photo_to_pbr_node_tile_with_border_w : 0;
		let offset_b: i32 = (from == channel_type_t.BASE_COLOR || from == channel_type_t.NORMAL_MAP) ? photo_to_pbr_node_tile_with_border_w * photo_to_pbr_node_tile_with_border_w * 2 : 0;
		for (let i: i32 = 0; i < (photo_to_pbr_node_tile_w * photo_to_pbr_node_tile_w); ++i) {
			let x: i32 = photo_to_pbr_node_border_w + i % photo_to_pbr_node_tile_w;
			let y: i32 = photo_to_pbr_node_border_w + math_floor(i / photo_to_pbr_node_tile_w);
			u8a[i * 4    ] = math_floor((ar[y * photo_to_pbr_node_tile_with_border_w + x           ] * 0.5 + 0.5) * 255);
			u8a[i * 4 + 1] = math_floor((ar[y * photo_to_pbr_node_tile_with_border_w + x + offset_g] * 0.5 + 0.5) * 255);
			u8a[i * 4 + 2] = math_floor((ar[y * photo_to_pbr_node_tile_with_border_w + x + offset_b] * 0.5 + 0.5) * 255);
			u8a[i * 4 + 3] = 255;
		}
		array_push(tile_floats, ar);

		// Use border pixels to blend seams
		if (i > 0) {
			if (x > 0) {
				let ar: f32_array_t = tile_floats[i - 1];
				for (let yy: i32 = 0; yy < photo_to_pbr_node_tile_w; ++yy) {
					for (let xx: i32 = 0; xx < photo_to_pbr_node_border_w; ++xx) {
						let i: i32 = yy * photo_to_pbr_node_tile_w + xx;
						let a: i32 = u8a[i * 4];
						let b: i32 = u8a[i * 4 + 1];
						let c: i32 = u8a[i * 4 + 2];

						let aa: i32 = math_floor((ar[(photo_to_pbr_node_border_w + yy) * photo_to_pbr_node_tile_with_border_w + photo_to_pbr_node_border_w + photo_to_pbr_node_tile_w + xx          ] * 0.5 + 0.5) * 255);
						let bb: i32 = math_floor((ar[(photo_to_pbr_node_border_w + yy) * photo_to_pbr_node_tile_with_border_w + photo_to_pbr_node_border_w + photo_to_pbr_node_tile_w + xx + offset_g] * 0.5 + 0.5) * 255);
						let cc: i32 = math_floor((ar[(photo_to_pbr_node_border_w + yy) * photo_to_pbr_node_tile_with_border_w + photo_to_pbr_node_border_w + photo_to_pbr_node_tile_w + xx + offset_b] * 0.5 + 0.5) * 255);

						let f: f32 = xx / photo_to_pbr_node_border_w;
						let invf: f32 = 1.0 - f;
						a = math_floor(a * f + aa * invf);
						b = math_floor(b * f + bb * invf);
						c = math_floor(c * f + cc * invf);

						u8a[i * 4    ] = a;
						u8a[i * 4 + 1] = b;
						u8a[i * 4 + 2] = c;
					}
				}
			}
			if (y > 0) {
				let ar: f32_array_t = tile_floats[i - tiles_x];
				for (let xx: i32 = 0; xx < photo_to_pbr_node_tile_w; ++xx) {
					for (let yy: i32 = 0; yy < photo_to_pbr_node_border_w; ++yy) {
						let i: i32 = yy * photo_to_pbr_node_tile_w + xx;
						let a: i32 = u8a[i * 4];
						let b: i32 = u8a[i * 4 + 1];
						let c: i32 = u8a[i * 4 + 2];

						let aa: i32 = math_floor((ar[(photo_to_pbr_node_border_w + photo_to_pbr_node_tile_w + yy) * photo_to_pbr_node_tile_with_border_w + photo_to_pbr_node_border_w + xx          ] * 0.5 + 0.5) * 255);
						let bb: i32 = math_floor((ar[(photo_to_pbr_node_border_w + photo_to_pbr_node_tile_w + yy) * photo_to_pbr_node_tile_with_border_w + photo_to_pbr_node_border_w + xx + offset_g] * 0.5 + 0.5) * 255);
						let cc: i32 = math_floor((ar[(photo_to_pbr_node_border_w + photo_to_pbr_node_tile_w + yy) * photo_to_pbr_node_tile_with_border_w + photo_to_pbr_node_border_w + xx + offset_b] * 0.5 + 0.5) * 255);

						let f: f32 = yy / photo_to_pbr_node_border_w;
						let invf: f32 = 1.0 - f;
						a = math_floor(a * f + aa * invf);
						b = math_floor(b * f + bb * invf);
						c = math_floor(c * f + cc * invf);

						u8a[i * 4    ] = a;
						u8a[i * 4 + 1] = b;
						u8a[i * 4 + 2] = c;
					}
				}
			}
		}

		///if (arm_metal || arm_vulkan)
		if (from == channel_type_t.BASE_COLOR) {
			photo_to_pbr_node_bgra_swap(u8a.buffer);
		}
		///end

		let temp2: image_t = image_from_bytes(u8a.buffer, photo_to_pbr_node_tile_w, photo_to_pbr_node_tile_w);
		g2_begin(photo_to_pbr_node_images[from]);
		g2_draw_image(temp2, x * photo_to_pbr_node_tile_w, y * photo_to_pbr_node_tile_w);
		g2_end();
		app_notify_on_next_frame(function(temp2: image_t) {
			image_unload(temp2);
		}, temp2);
	}

	return photo_to_pbr_node_images[from];
}

///if (arm_metal || arm_vulkan)
function photo_to_pbr_node_bgra_swap(buffer: buffer_t) {
	let u8a: buffer_t = buffer;
	for (let i: i32 = 0; i < math_floor(buffer.length / 4); ++i) {
		let r: i32 = u8a[i * 4];
		u8a[i * 4] = u8a[i * 4 + 2];
		u8a[i * 4 + 2] = r;
	}
	return buffer;
}
///end

let photo_to_pbr_node_def: ui_node_t = {
	id: 0,
	name: _tr("Photo to PBR"),
	type: "photo_to_pbr_node",
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
			name: _tr("Base Color"),
			type: "RGBA",
			color: 0xffc7c729,
			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Occlusion"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Roughness"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Metallic"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Normal Map"),
			type: "VECTOR",
			color: 0xffc7c729,
			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Height"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [],
	width: 0
};
