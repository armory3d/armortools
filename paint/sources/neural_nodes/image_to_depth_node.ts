
function image_to_depth_node_init() {
	array_push(nodes_material_neural, image_to_depth_node_def);
	map_set(parser_material_node_values, "NEURAL_IMAGE_TO_DEPTH", neural_node_value);
	map_set(ui_nodes_custom_buttons, "image_to_depth_node_button", image_to_depth_node_button);
}

function image_to_depth_node_button(node_id: i32) {
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let node: ui_node_t          = ui_get_node(canvas.nodes, node_id);

	let models: string[] = [ "Marigold" ];
	let model: i32       = ui_combo(ui_handle(__ID__), models, tr("Model"));

	if (neural_node_button()) {
		let from_node: ui_node_t = neural_from_node(node.inputs[0]);
		let input: gpu_texture_t = ui_nodes_get_node_preview_image(from_node);
		if (input != null) {
			let dir: string = neural_node_dir();
			iron_write_png(dir + path_sep + "input.png", gpu_get_texture_pixels(input), input.width, input.height, 0);

			let argv: string[] = [
				dir + "/sd_vulkan",
				"-m",
				dir + "/marigold-depth-v1-1.q8_0.gguf",
				"--sampling-method",
				"ddim_trailing",
				"--steps",
				"10",
				"-s",
				"-1",
				"-W",
				"768",
				"-H",
				"768",
				"-p",
				" ",
				"-i",
				dir + "/input.png",
				"-o",
				dir + "/output.png",
				null
			];
			iron_exec_async(argv[0], argv.buffer);
			sys_notify_on_update(neural_node_check_result, node);
		}
	}
}

let image_to_depth_node_def: ui_node_t = {
	id : 0,
	name : _tr("Image to Depth"),
	type : "NEURAL_IMAGE_TO_DEPTH",
	x : 0,
	y : 0,
	color : 0xff4982a0,
	inputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Color"),
		type : "RGBA",
		color : 0xffc7c729,
		default_value : f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	outputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Depth"),
		type : "VALUE",
		color : 0xffa1a1a1,
		default_value : f32_array_create_x(1.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [ {
		name : "image_to_depth_node_button",
		type : "CUSTOM",
		output : -1,
		default_value : f32_array_create_x(0),
		data : null,
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 2
	} ],
	width : 0,
	flags : 0
};
