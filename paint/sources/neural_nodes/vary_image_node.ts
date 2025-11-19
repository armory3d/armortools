
function vary_image_node_init() {
	array_push(nodes_material_neural, vary_image_node_def);
	map_set(parser_material_node_vectors, "NEURAL_VARY_IMAGE", neural_node_vector);
	map_set(ui_nodes_custom_buttons, "vary_image_node_button", vary_image_node_button);
}

function vary_image_node_button(node_id: i32) {
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let node: ui_node_t          = ui_get_node(canvas.nodes, node_id);
	let node_name: string        = parser_material_node_name(node);
	let h: ui_handle_t           = ui_handle(node_name);

	let models: string[] = [ "Stable Diffusion", "Qwen Image Edit" ];
	let model: i32       = ui_combo(ui_nest(h, 0), models, tr("Model"));

	let prompt: string     = ui_text_area(ui_nest(h, 1), ui_align_t.LEFT, true, tr("prompt"), true);
	node.buttons[0].height = string_split(prompt, "\n").length + 2;

	if (neural_node_button(node, models[model])) {

		sys_notify_on_next_frame(function(node: ui_node_t) {
			let from_node: ui_node_t = neural_from_node(node.inputs[0], 0);
			let input: gpu_texture_t = ui_nodes_get_node_preview_image(from_node);
			if (input != null) {

				let mask: gpu_texture_t = gpu_create_render_target(512, 512);
				draw_begin(mask, true, 0xffffffff);
				draw_end();

				/// if IRON_BGRA
				let input_buf: buffer_t = export_arm_bgra_swap(gpu_get_texture_pixels(input)); // Vulkan non-rt textures need a flip
				/// else
				let input_buf: buffer_t = gpu_get_texture_pixels(input);
				/// end

				let dir: string = neural_node_dir();
				iron_write_png(dir + path_sep + "input.png", input_buf, input.width, input.height, 0);
				iron_write_png(dir + path_sep + "mask.png", gpu_get_texture_pixels(mask), mask.width, mask.height, 0);

				let node_name: string = parser_material_node_name(node);
				let h: ui_handle_t    = ui_handle(node_name);
				let model: i32        = ui_nest(h, 0).i;
				let prompt: string    = ui_nest(h, 1).text;

				if (prompt == "") {
					prompt = ".";
				}

				let argv: string[];
				if (model == 0) {
					argv = [
						dir + "/sd_vulkan",
						"-m",
						dir + "/v1-5-pruned-emaonly.safetensors",
						"--offload-to-cpu",
						"--steps",
						"30",
						"-s",
						"-1",
						"-W",
						"512",
						"-H",
						"512",
						"--cfg-scale",
						"0.0",
						"--strength",
						"0.2",
						"-p",
						prompt,
						"-i",
						dir + "/input.png",
						"--mask",
						dir + "/mask.png",
						"-o",
						dir + "/output.png",
						null
					];
				}
				else {
					argv = [
						dir + "/sd_vulkan",
						"--diffusion-model",
						dir + "/Qwen-Image-Edit-2509-Q4_K_S.gguf",
						"--vae",
						dir + "/Qwen_Image-VAE.safetensors",
						"--qwen2vl",
						dir + "/Qwen2.5-VL-7B-Instruct-Q4_K_S.gguf",
						"--qwen2vl_vision",
						dir + "/mmproj-F16.gguf",
						"--offload-to-cpu",
						"--diffusion-fa",
						"--steps",
						"50",
						"-s",
						"-1",
						"-W",
						"512",
						"-H",
						"512",
						// "--cfg-scale",
						// "1.0",
						// "--strength",
						// "0.2",
						"-p",
						"vary contents of the image",
						// prompt,
						"-r",
						dir + "/input.png",
						"-o",
						dir + "/output.png",
						null
					];
				}

				iron_exec_async(argv[0], argv.buffer);
				sys_notify_on_update(neural_node_check_result, node);
			}
		}, node);
	}
}

let vary_image_node_def: ui_node_t = {
	id : 0,
	name : _tr("Vary Image"),
	type : "NEURAL_VARY_IMAGE",
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
		name : "vary_image_node_button",
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
