
let upscale_node_result: gpu_texture_t = null;

function upscale_node_init() {
	array_push(nodes_material_neural, upscale_node_def);
	map_set(parser_material_node_vectors, "NEURAL_UPSCALE", upscale_node_vector);
	map_set(ui_nodes_custom_buttons, "upscale_node_button", upscale_node_button);
}

function upscale_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	if (upscale_node_result == null) {
		return("float3(0.0, 0.0, 0.0)");
	}
	let tex_name: string = parser_material_node_name(node);
	map_set(data_cached_images, tex_name, upscale_node_result);
	let tex: bind_tex_t = parser_material_make_bind_tex(tex_name, tex_name);
	let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
	return texstore + ".rgb";
}

function upscale_node_button(node_id: i32) {
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let node: ui_node_t = ui_get_node(canvas.nodes, node_id);

	let models: string[] = ["RealESRGAN_X2", "RealESRGAN_X4"];
	let model: i32 = ui_combo(ui_handle(__ID__), models, tr("Model"));

	if (iron_exec_async_done == 0) {
		ui_button("Cancel...");
	}
	else if (ui_button("Run")) {
		let inp: ui_node_socket_t = node.inputs[1];
		let from_node: ui_node_t = null;
		for (let i: i32 = 0; i < canvas.links.length; ++i) {
			let l: ui_node_link_t = canvas.links[i];
			if (l.to_id == inp.node_id) {
				from_node = ui_get_node(canvas.nodes, l.from_id);
				break;
			}
		}

		let input: gpu_texture_t = ui_nodes_get_node_preview_image(from_node);
		if (input != null) {
			let dir: string;
			if (path_is_protected()) {
				dir = iron_internal_save_path() + "models";
			}
			else {
				dir = iron_internal_files_location() + path_sep + "models";
			}
			iron_write_png(dir + path_sep + "input.png", gpu_get_texture_pixels(input), input.width, input.height, 0);
			let argv: string[] = [
				dir + "/sd",
				"-M", "upscale",
				"--upscale-model", dir + "/RealESRGAN_x4plus.pth",
				"-i", dir + "/input.png",
				"-o", dir + "/output.png",
				null
			];
			iron_exec_async(argv[0], argv.buffer);
			sys_notify_on_update(upscale_node_check_result, dir);
		}
	}
}

function upscale_node_check_result(dir: string) {
	if (iron_exec_async_done == 1) {
		let file: string = dir + path_sep + "output.png";
		if (iron_file_exists(file)) {
			upscale_node_result = iron_load_texture(file);
		}
		sys_remove_update(upscale_node_check_result);
	}
}

let upscale_node_def: ui_node_t = {
	id: 0,
	name: _tr("Upscale"),
	type: "NEURAL_UPSCALE",
	x: 0,
	y: 0,
	color: 0xff4982a0,
	inputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Vector"),
			type: "VECTOR",
			color: 0xff6363c7,
			default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
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
			name: "upscale_node_button",
			type: "CUSTOM",
			output: -1,
			default_value: f32_array_create_x(0),
			data: null,
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 2
		}
	],
	width: 0,
	flags: 0
};
