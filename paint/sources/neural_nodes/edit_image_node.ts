
let edit_image_node_result: gpu_texture_t = null;

function edit_image_node_init() {
	array_push(nodes_material_neural, edit_image_node_def);
	map_set(parser_material_node_vectors, "NEURAL_EDIT_IMAGE", edit_image_node_vector);
	map_set(ui_nodes_custom_buttons, "edit_image_node_button", edit_image_node_button);
}

function edit_image_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	if (edit_image_node_result == null) {
		return "float3(0.0, 0.0, 0.0)";
	}
	let tex_name: string = parser_material_node_name(node);
	map_set(data_cached_images, tex_name, edit_image_node_result);
	let tex: bind_tex_t  = parser_material_make_bind_tex(tex_name, tex_name);
	let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
	return texstore + ".rgb";
}

function edit_image_node_button(node_id: i32) {
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let node: ui_node_t          = ui_get_node(canvas.nodes, node_id);

	let models: string[] = [ "Qwen Image Edit" ];
	let model: i32       = ui_combo(ui_handle(__ID__), models, tr("Model"));

	let prompt: string     = ui_text_area(ui_handle(__ID__), ui_align_t.LEFT, true, tr("prompt"), true);
	node.buttons[0].height = string_split(prompt, "\n").length + 2;

	if (iron_exec_async_done == 0) {
		ui_button("Cancel...");
	}
	else if (ui_button("Run")) {
		let inp: ui_node_socket_t = node.inputs[0];
		let from_node: ui_node_t  = null;
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
				dir + "/sd_qie", "--diffusion-model", dir + "/Qwen-Image-Edit-2509-Q4_K_S.gguf", "--vae", dir + "/qwen_image_vae.safetensors", "--qwen2vl",
				dir + "/Qwen2.5-VL-7B-Instruct-Q8_0.gguf", "--qwen2vl_vision", dir + "/Qwen2.5-VL-7B-Instruct.mmproj-Q8_0.gguf", "--sampling-method", "euler",
				"--offload-to-cpu", "--diffusion-fa", "--steps", "50", "-s", "-1", "-W", "512", "-H", "512", "-p", "'" + prompt + "'",
				// "-n",
				// "'" + negative + "'",
				"-r", dir + "/input.png", "-o", dir + "/output.png", null
			];

			iron_exec_async(argv[0], argv.buffer);
			sys_notify_on_update(edit_image_node_check_result, dir);
		}
	}
}

function edit_image_node_check_result(dir: string) {
	if (iron_exec_async_done == 1) {
		let file: string = dir + path_sep + "output.png";
		if (iron_file_exists(file)) {
			edit_image_node_result = iron_load_texture(file);
		}
		sys_remove_update(edit_image_node_check_result);
	}
}

let edit_image_node_def: ui_node_t = {
	id : 0,
	name : _tr("Edit Image"),
	type : "NEURAL_EDIT_IMAGE",
	x : 0,
	y : 0,
	color : 0xff4982a0,
	inputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Color"),
		type : "RGBA",
		color : 0xffc7c729,
		default_value : f32_array_create_xyzw(1.0, 1.0, 1.0, 1.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	outputs : [ {
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
	buttons : [ {
		name : "edit_image_node_button",
		type : "CUSTOM",
		output : -1,
		default_value : f32_array_create_x(0),
		data : null,
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 0
	} ],
	width : 0,
	flags : 0
};
