
function text_to_image_node_init() {
	array_push(nodes_material_neural, text_to_image_node_def);
	map_set(parser_material_node_vectors, "NEURAL_TEXT_TO_IMAGE", neural_node_vector);
	map_set(ui_nodes_custom_buttons, "text_to_image_node_button", text_to_image_node_button);
}

function text_to_image_node_sd_args(dir: string, prompt: string): string[] {
	let argv: string[] = [
		dir + "/sd",
		"-m",
		dir + "/v1-5-pruned-emaonly.safetensors",
		"--offload-to-cpu",
		"-W",
		"512",
		"-H",
		"512",
		"--steps",
		"40",
		"-s",
		"-1",
		"-o",
		dir + "/output.png",
		"-p",
		"'" + prompt + "'",
		null
	];
	return argv;
}

function text_to_image_node_qwen_args(dir: string, prompt: string): string[] {
	let argv: string[] = [
		dir + "/sd",
		"--diffusion-model",
		dir + "/Qwen_Image-Q4_K_S.gguf",
		"--vae",
		dir + "/Qwen_Image-VAE.safetensors",
		"--qwen2vl",
		dir + "/Qwen2.5-VL-7B-Instruct-Q4_K_S.gguf",
		"--sampling-method",
		"euler",
		"--offload-to-cpu",
		"-W",
		"512",
		"-H",
		"512",
		"--steps",
		"20",
		"-s",
		"-1",
		"-o",
		dir + "/output.png",
		"-p",
		"'" + prompt + "'",
		null
	];
	return argv;
}

function text_to_image_node_wan_args(dir: string, prompt: string): string[] {
	let argv: string[] = [
		dir + "/sd",
		"-M",
		"vid_gen",
		"--diffusion-model",
		dir + "/Wan2.2-T2V-A14B-LowNoise-Q4_K_S.gguf",
		"--high-noise-diffusion-model",
		dir + "/Wan2.2-T2V-A14B-HighNoise-Q4_K_S.gguf",
		"--vae",
		dir + "/Wan2.1_VAE.safetensors",
		"--t5xxl",
		dir + "/umt5-xxl-encoder-Q4_K_S.gguf",
		"--sampling-method",
		"euler",
		"--steps",
		"20",
		"--high-noise-sampling-method",
		"euler",
		"--high-noise-steps",
		"10",
		"-W",
		"512",
		"-H",
		"512",
		"--offload-to-cpu",
		"-s",
		"-1",
		"-o",
		dir + "/output.png",
		"-p",
		"'" + prompt + "'",
		null
	];
	return argv;
}

function text_to_image_node_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	let models: string[] = [ "Stable Diffusion", "Qwen Image", "Wan" ];
	let model: i32       = ui_combo(ui_handle(__ID__), models, tr("Model"));

	let prompt: string     = ui_text_area(ui_handle(__ID__), ui_align_t.LEFT, true, tr("prompt"), true);
	node.buttons[0].height = string_split(prompt, "\n").length + 2;

	if (neural_node_button()) {
		let dir: string = neural_node_dir();

		let argv: string[];
		if (model == 0) {
			argv = text_to_image_node_sd_args(dir, prompt);
		}
		else if (model == 1) {
			argv = text_to_image_node_qwen_args(dir, prompt);
		}
		else {
			argv = text_to_image_node_wan_args(dir, prompt);
		}
		iron_exec_async(argv[0], argv.buffer);
		sys_notify_on_update(neural_node_check_result, node);
	}
}

let text_to_image_node_def: ui_node_t = {
	id : 0,
	name : _tr("Text to Image"),
	type : "NEURAL_TEXT_TO_IMAGE",
	x : 0,
	y : 0,
	color : 0xff4982a0,
	inputs : [],
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
		name : "text_to_image_node_button",
		type : "CUSTOM",
		output : -1,
		default_value : f32_array_create_x(0),
		data : null,
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 1
	} ],
	width : 0,
	flags : 0
};
