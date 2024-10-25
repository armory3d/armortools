
type tiling_node_t = {
	base?: logic_node_t;
	result?: image_t;
};

let tiling_node_image: image_t = null;
let tiling_node_prompt: string = "";
let tiling_node_strength: f32 = 0.5;
let tiling_node_auto: bool = true;

function tiling_node_create(raw: ui_node_t, args: f32_array_t): tiling_node_t {
	let n: float_node_t = {};
	n.base = logic_node_create(n);
	n.base.get_as_image = tiling_node_get_as_image;
	n.base.get_cached_image = tiling_node_get_cached_image;
	tiling_node_init();
	return n;
}

function tiling_node_init() {
	if (tiling_node_image == null) {
		tiling_node_image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y());
	}
}

function tiling_node_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	tiling_node_auto = node.buttons[0].default_value[0] == 0 ? false : true;
	if (!tiling_node_auto) {
		let tiling_node_strength_handle: ui_handle_t = ui_handle(__ID__);
		if (tiling_node_strength_handle.init) {
			tiling_node_strength_handle.value = tiling_node_strength;
		}

		tiling_node_strength = ui_slider(tiling_node_strength_handle, tr("strength"), 0, 1, true);
		tiling_node_prompt = ui_text_area(ui_handle(__ID__), ui_align_t.LEFT, true, tr("prompt"), true);
		node.buttons[1].height = 1 + string_split(tiling_node_prompt, "\n").length;
	}
	else {
		node.buttons[1].height = 0;
	}
}

function tiling_node_get_as_image(self: tiling_node_t, from: i32): image_t {
	let source: image_t = logic_node_input_get_as_image(self.base.inputs[0]);
	g2_begin(tiling_node_image);
	g2_draw_scaled_image(source, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
	g2_end();

	console_progress(tr("Processing") + " - " + tr("Tiling"));

	if (tiling_node_auto){
		self.result = inpaint_node_texsynth_inpaint(tiling_node_image, true, null);
	}
	else {
		self.result = tiling_node_sd_tiling(tiling_node_image, -1);
	}
	return self.result;
}

function tiling_node_get_cached_image(self: tiling_node_t): image_t {
	return self.result;
}

function tiling_node_sd_tiling(image: image_t, seed: i32): image_t {
	text_to_photo_node_tiling = false;
	let tile: image_t = image_create_render_target(512, 512);
	g2_begin(tile);
	g2_draw_scaled_image(image, -256, -256, 512, 512);
	g2_draw_scaled_image(image, 256, -256, 512, 512);
	g2_draw_scaled_image(image, -256, 256, 512, 512);
	g2_draw_scaled_image(image, 256, 256, 512, 512);
	g2_end();

	let u8a: u8_array_t = u8_array_create(512 * 512);
	for (let i: i32 = 0; i < 512 * 512; ++i) {
		let x: i32 = i % 512;
		let y: i32 = math_floor(i / 512);
		let l: i32 = y < 256 ? y : (511 - y);
		u8a[i] = (x > 256 - l && x < 256 + l) ? 0 : 255;
	}
	// for (let i: i32 = 0; i < 512 * 512; ++i) u8a[i] = 255;
	// for (let x: i32 = (256 - 32); x < (256 + 32); ++x) {
	// 	for (let y: i32 = 0; y < 512; ++y) {
	// 		u8a[y * 512 + x] = 0;
	// 	}
	// }
	// for (let x: i32 = 0; x < 512; ++x) {
	// 	for (let y: i32 = (256 - 32); y < 256 + 32); ++y) {
	// 		u8a[y * 512 + x] = 0;
	// 	}
	// }
	let mask: image_t = image_from_bytes(u8a, 512, 512, tex_format_t.R8);

	inpaint_node_prompt = tiling_node_prompt;
	inpaint_node_strength = tiling_node_strength;
	if (seed >= 0) {
		random_node_set_seed(seed);
	}
	return inpaint_node_sd_inpaint(tile, mask);
}

let tiling_node_def: ui_node_t = {
	id: 0,
	name: _tr("Tiling"),
	type: "tiling_node",
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
			name: "tiling_node_button",
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
