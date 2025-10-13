

let text_to_image_node_result: gpu_texture_t = null;

function text_to_image_node_init() {
    array_push(nodes_material_neural, text_to_image_node_def);
    map_set(parser_material_node_vectors, "NEURAL_TEXT_TO_IMAGE", text_to_image_node_vector);
	map_set(ui_nodes_custom_buttons, "text_to_image_node_button", text_to_image_node_button);
}

function text_to_image_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	if (text_to_image_node_result == null) {
    	return("float3(0.0, 0.0, 0.0)");
	}
	let tex_name: string = parser_material_node_name(node);
	map_set(data_cached_images, tex_name, text_to_image_node_result);
    let tex: bind_tex_t = parser_material_make_bind_tex(tex_name, tex_name);
    let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
    return texstore + ".rgb";
}

function text_to_image_node_run_qwen(dir: string, prompt: string): string[] {
	let argv: string[] = [
		dir + "/sd",
		"--diffusion-model", dir + "/qwen-image-Q8_0.gguf",
		"--vae", dir + "/qwen_image_vae.safetensors",
		"--qwen2vl", dir + "/Qwen2.5-VL-7B-Instruct-Q8_0.gguf",
		"--sampling-method", "euler",
		"--offload-to-cpu",
		"-W", "512",
		"-H", "512",
		"--steps", "40",
		"-s", "-1",
		"-o", dir + "/output.png",
		"-p", "'" + prompt + "'",
		null
	];
	return argv;
}

function text_to_image_node_run_wan(dir: string, prompt: string): string[] {
	let argv: string[] = [
		dir + "/sd",
		"-M", "vid_gen",
		"--diffusion-model", dir + "/Wan2.2-T2V-A14B-LowNoise-Q8_0.gguf",
		"--high-noise-diffusion-model", dir + "/Wan2.2-T2V-A14B-HighNoise-Q8_0.gguf",
		"--vae", dir + "/wan_2.1_vae.safetensors",
		"--t5xxl", dir + "/umt5-xxl-encoder-Q8_0.gguf",
		// "--cfg-scale", "3.5",
		"--sampling-method", "euler",
		"--steps", "40",
		// "--high-noise-cfg-scale", "3.5",
		"--high-noise-sampling-method", "euler",
		"--high-noise-steps", "20",
		"-W", "512",
		"-H", "512",
		// "--diffusion-fa",
		"--offload-to-cpu",
		// "--flow-shift", "3.0",
		"-s", "-1",
		"-o", dir + "/output.png",
		"-p", "'" + prompt + "'",
		null
	];
	return argv;
}

function text_to_image_node_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	let models: string[] = ["Qwen Image", "Wan"];
	let model: i32 = ui_combo(ui_handle(__ID__), models, tr("Model"));

	// let tiling: bool = node.buttons[0].default_value[0] == 0 ? false : true;
	let prompt: string = ui_text_area(ui_handle(__ID__), ui_align_t.LEFT, true, tr("prompt"), true);
	node.buttons[1].height = string_split(prompt, "\n").length + 2;

	// ui_button("Download");

	if (iron_exec_async_done == 0) {
		ui_button("Cancel...");
	}
	else if (ui_button("Run")) {
		let dir: string;
		if (path_is_protected()) {
			dir = iron_internal_save_path() + "models";
		}
		else {
			dir = iron_internal_files_location() + path_sep + "models";
		}

		let argv: string[];
		if (model == 0) {
			argv = text_to_image_node_run_qwen(dir, prompt);
		}
		else {
			argv = text_to_image_node_run_wan(dir, prompt);
		}
		iron_exec_async(argv[0], argv.buffer);
		sys_notify_on_update(text_to_image_node_check_result, dir);
	}
}

function text_to_image_node_check_result(dir: string) {
	if (iron_exec_async_done == 1) {
		let file: string = dir + path_sep + "output.png";
		if (iron_file_exists(file)) {
			text_to_image_node_result = iron_load_texture(file);
		}
		sys_remove_update(text_to_image_node_check_result);
	}
}

let text_to_image_node_def: ui_node_t = {
	id: 0,
	name: _tr("Text to Image"),
	type: "NEURAL_TEXT_TO_IMAGE",
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
			name: "text_to_image_node_button",
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
	width: 0,
	flags: 0
};
