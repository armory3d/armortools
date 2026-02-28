
function edit_image_node_init() {
	array_push(nodes_material_neural, edit_image_node_def);
	map_set(parser_material_node_vectors, "NEURAL_EDIT_IMAGE", neural_node_vector);
	map_set(ui_nodes_custom_buttons, "edit_image_node_button", edit_image_node_button);
}

function edit_image_node_button(node_id: i32) {
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let node: ui_node_t          = ui_get_node(canvas.nodes, node_id);
	let node_name: string        = parser_material_node_name(node);
	let h: ui_handle_t           = ui_handle(node_name);

	let models: string[] = [ "Qwen Image Edit" ];
	let model: i32       = ui_combo(ui_nest(h, 0), models, tr("Model"));

	let prompt: string     = ui_text_area(ui_nest(h, 1), ui_align_t.LEFT, true, tr("prompt"), true);
	node.buttons[0].height = string_split(prompt, "\n").length + 2;

	if (neural_node_button(node, models[model])) {
		let from_node: ui_node_t = neural_from_node(node.inputs[0], 0);
		let input: gpu_texture_t = ui_nodes_get_node_preview_image(from_node);
		if (input != null) {
			/// if IRON_BGRA
			let input_buf: buffer_t = export_arm_bgra_swap(gpu_get_texture_pixels(input)); // Vulkan non-rt textures need a flip
			/// else
			let input_buf: buffer_t = gpu_get_texture_pixels(input);
			/// end

			let dir: string = neural_node_dir();
			iron_write_png(dir + path_sep + "input.png", input_buf, input.width, input.height, 0);

			if (prompt == "") {
				prompt = ".";
			}

			let argv: string[] = [
				dir + "/" + neural_node_sd_bin(),
				"--diffusion-model",
				dir + "/qwen-image-edit-2511-Q4_K_S.gguf",
				"--vae",
				dir + "/Qwen_Image-VAE.safetensors",
				"--llm",
				dir + "/Qwen2.5-VL-7B-Instruct-Q4_K_S.gguf",
				"--llm_vision",
				dir + "/mmproj-F16.gguf",
				"--offload-to-cpu",
				"--diffusion-fa",
				"--steps",
				"30",
				"-s",
				"-1",
				"--cfg-scale", "2.5",
				"--flow-shift", "3",
				"--qwen-image-zero-cond-t",
				"-W",
				"512",
				"-H",
				"512",
				"-p",
				prompt,
				"-r",
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

let edit_image_node_def: ui_node_t = {
	id : 0,
	name : _tr("Edit Image"),
	type : "NEURAL_EDIT_IMAGE",
	x : 0,
	y : 0,
	color : 0xff4982a0,
	inputs : [ //
		{
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
		}
	],
	outputs : [ //
		{
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
		}
	],
	buttons : [ //
		{
			name : "edit_image_node_button",
			type : "CUSTOM",
			output : -1,
			default_value : f32_array_create_x(0),
			data : null,
			min : 0.0,
			max : 1.0,
			precision : 100,
			height : 0
		}
	],
	width : 0,
	flags : 0
};
