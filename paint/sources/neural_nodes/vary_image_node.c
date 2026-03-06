void vary_image_node_init() {
	any_array_push(nodes_material_neural, vary_image_node_def);
	any_map_set(parser_material_node_vectors, "NEURAL_VARY_IMAGE", neural_node_vector);
	any_map_set(ui_nodes_custom_buttons, "vary_image_node_button", vary_image_node_button);
}

void vary_image_node_button(i32 node_id) {
	ui_node_canvas_t *canvas    = ui_nodes_get_canvas(true);
	ui_node_t        *node      = ui_get_node(canvas->nodes, node_id);
	string_t         *node_name = parser_material_node_name(node, null);
	ui_handle_t      *h         = ui_handle(node_name);
	string_t_array_t *models    = any_array_create_from_raw(
        (any[]){
            "Stable Diffusion",
            "Qwen Image Edit",
        },
        2);
	i32       model                  = ui_combo(ui_nest(h, 0), models, tr("Model", null), false, UI_ALIGN_LEFT, true);
	string_t *prompt                 = ui_text_area(ui_nest(h, 1), UI_ALIGN_LEFT, true, tr("prompt", null), true);
	node->buttons->buffer[0]->height = string_split(prompt, "\n")->length + 2;

	if (neural_node_button(node, models->buffer[model])) {
		sys_notify_on_next_frame(&vary_image_node_button_240006, node);
	}
}

void vary_image_node_button_240006(ui_node_t *node) {
	ui_node_t     *from_node = neural_from_node(node->inputs->buffer[0], 0);
	gpu_texture_t *input     = ui_nodes_get_node_preview_image(from_node);
	if (input != null) {

		gpu_texture_t *mask = gpu_create_render_target(512, 512, GPU_TEXTURE_FORMAT_RGBA32);
		draw_begin(mask, true, 0xffffffff);
		draw_end();

		#ifdef IRON_BGRA
		buffer_t *input_buf = export_arm_bgra_swap(gpu_get_texture_pixels(input)); // Vulkan non-rt textures need a flip
		#else
		buffer_t *input_buf = gpu_get_texture_pixels(input);
		#endif

		string_t *dir = neural_node_dir();
		iron_write_png(string_join(string_join(dir, PATH_SEP), "input.png"), input_buf, input->width, input->height, 0);
		iron_write_png(string_join(string_join(dir, PATH_SEP), "mask.png"), gpu_get_texture_pixels(mask), mask->width, mask->height, 0);

		string_t    *node_name = parser_material_node_name(node, null);
		ui_handle_t *h         = ui_handle(node_name);
		i32          model     = ui_nest(h, 0)->i;
		string_t    *prompt    = ui_nest(h, 1)->text;

		if (string_equals(prompt, "")) {
			prompt = ".";
		}

		string_t_array_t *argv;
		if (model == 0) {
			argv = any_array_create_from_raw(
			    (any[]){
			        string_join(string_join(dir, "/"), neural_node_sd_bin()),
			        "-m",
			        string_join(dir, "/v1-5-pruned-emaonly.safetensors"),
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
			        string_join(dir, "/input.png"),
			        "--mask",
			        string_join(dir, "/mask.png"),
			        "-o",
			        string_join(dir, "/output.png"),
			        null,
			    },
			    25);
		}
		else {
			argv = any_array_create_from_raw(
			    (any[]){
			        string_join(string_join(dir, "/"), neural_node_sd_bin()),
			        "--diffusion-model",
			        string_join(dir, "/qwen-image-edit-2511-Q4_K_S.gguf"),
			        "--vae",
			        string_join(dir, "/Qwen_Image-VAE.safetensors"),
			        "--llm",
			        string_join(dir, "/Qwen2.5-VL-7B-Instruct-Q4_K_S.gguf"),
			        "--llm_vision",
			        string_join(dir, "/mmproj-F16.gguf"),
			        "--offload-to-cpu",
			        "--diffusion-fa",
			        "--qwen-image-zero-cond-t",
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
			        string_join(dir, "/input.png"),
			        "-o",
			        string_join(dir, "/output.png"),
			        null,
			    },
			    27);
		}

		iron_exec_async(argv->buffer[0], argv->buffer);
		sys_notify_on_update(neural_node_check_result, node);
	}
}
