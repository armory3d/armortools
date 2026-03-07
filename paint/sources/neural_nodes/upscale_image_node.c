void upscale_image_node_init() {
	any_array_push(nodes_material_neural, upscale_image_node_def);
	any_map_set(parser_material_node_vectors, "NEURAL_UPSCALE_IMAGE", neural_node_vector);
	any_map_set(ui_nodes_custom_buttons, "upscale_image_node_button", upscale_image_node_button);
}

void upscale_image_node_button(i32 node_id) {
	ui_node_canvas_t *canvas    = ui_nodes_get_canvas(true);
	ui_node_t        *node      = ui_get_node(canvas->nodes, node_id);
	char         *node_name = parser_material_node_name(node, NULL);
	ui_handle_t      *h         = ui_handle(node_name);
	string_t_array_t *models    = any_array_create_from_raw(
        (any[]){
            "Real-ESRGAN",
        },
        1);
	i32 model = ui_combo(ui_nest(h, 0), models, tr("Model", NULL), false, UI_ALIGN_LEFT, true);
	if (neural_node_button(node, models->buffer[model])) {
		ui_node_t     *from_node = neural_from_node(node->inputs->buffer[0], 0);
		gpu_texture_t *input     = ui_nodes_get_node_preview_image(from_node);
		if (input != NULL) {
			#ifdef IRON_BGRA
			buffer_t *input_buf = export_arm_bgra_swap(gpu_get_texture_pixels(input)); // Vulkan non-rt textures need a flip
			#else
			buffer_t *input_buf = gpu_get_texture_pixels(input);
			#endif

			char *dir = neural_node_dir();
			iron_write_png(string_join(string_join(dir, PATH_SEP), "input.png"), input_buf, input->width, input->height, 0);

			string_t_array_t *argv = any_array_create_from_raw(
			    (any[]){
			        string_join(string_join(dir, "/"), neural_node_sd_bin()),
			        "-M",
			        "upscale",
			        "--upscale-model",
			        string_join(dir, "/RealESRGAN_x4plus.pth"),
			        "-i",
			        string_join(dir, "/input.png"),
			        "-o",
			        string_join(dir, "/output.png"),
			        NULL,
			    },
			    10);
			iron_exec_async(argv->buffer[0], argv->buffer);
			sys_notify_on_update(neural_node_check_result, node);
		}
	}
}
