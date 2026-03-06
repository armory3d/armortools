void image_to_3d_mesh_node_init() {
	any_array_push(nodes_material_neural, image_to_3d_mesh_node_def);
	any_map_set(parser_material_node_values, "NEURAL_IMAGE_TO_3D_MESH", neural_node_value);
	any_map_set(ui_nodes_custom_buttons, "image_to_3d_mesh_node_button", image_to_3d_mesh_node_button);
}

buffer_t *image_to_3d_mesh_node_remove_background(buffer_t *buffer) {
	for (i32 i = 0; i < math_floor((buffer->length) / (float)4); ++i) {
		if (buffer->buffer[i * 4] <= 16 && buffer->buffer[i * 4 + 1] <= 16 && buffer->buffer[i * 4 + 2] <= 16) {
			buffer->buffer[i * 4 + 3] = 0;
		}
	}
	return buffer;
}

void image_to_3d_mesh_node_button(i32 node_id) {
	ui_node_canvas_t *canvas    = ui_nodes_get_canvas(true);
	ui_node_t        *node      = ui_get_node(canvas->nodes, node_id);
	string_t         *node_name = parser_material_node_name(node, null);
	ui_handle_t      *h         = ui_handle(node_name);
	string_t_array_t *models    = any_array_create_from_raw(
        (any[]){
            "Hunyuan3D",
        },
        1);
	i32 model = ui_combo(ui_nest(h, 0), models, tr("Model", null), false, UI_ALIGN_LEFT, true);
	if (neural_node_button(node, models->buffer[model])) {
		ui_node_t     *from_node = neural_from_node(node->inputs->buffer[0], 0);
		gpu_texture_t *input     = ui_nodes_get_node_preview_image(from_node);
		if (input != null) {
			string_t *dir = neural_node_dir();

			#ifdef IRON_BGRA
			buffer_t *input_buf = image_to_3d_mesh_node_remove_background(export_arm_bgra_swap(gpu_get_texture_pixels(input)));
			#else
			buffer_t *input_buf = image_to_3d_mesh_node_remove_background(gpu_get_texture_pixels(input));
			#endif
			iron_write_png(string_join(string_join(dir, PATH_SEP), "input.png"), input_buf, input->width, input->height, 0);

			dir                    = string_copy(string_replace_all(dir, "\\", "/"));
			string_t_array_t *argv = any_array_create_from_raw(
			    (any[]){
			        string_join(dir, "/Hunyuan3D_win64/python/python.exe"),
			        "-s",
			        "-B",
			        "-c",
			        string_join(
			            string_join(string_join(string_join(string_join(string_join("from hy3dshape.pipelines import Hunyuan3DDiTFlowMatchingPipeline; "
			                                                                        "shape_pipeline = Hunyuan3DDiTFlowMatchingPipeline.from_pretrained('",
			                                                                        dir),
			                                                            "/Hunyuan3D_win64/Hunyuan3D-2.1'); mesh = shape_pipeline(image='"),
			                                                dir),
			                                    "/input.png', octree_resolution=512, num_chunks=64000)[0]; mesh.export('"),
			                        dir),
			            "/output.obj')"),
			        null,
			    },
			    6);

			iron_exec_async(argv->buffer[0], argv->buffer);
			sys_notify_on_update(image_to_3d_mesh_node_check_result, node);
		}
	}
}

void image_to_3d_mesh_node_check_result(ui_node_t *node) {
	gc_unroot(neural_node_current);
	neural_node_current = node;
	gc_root(neural_node_current);
	iron_delay_idle_sleep();
	if (iron_exec_async_done == 1) {
		string_t *file = string_join(string_join(neural_node_dir(), PATH_SEP), "output.obj");
		if (iron_file_exists(file)) {
			// let result: gpu_texture_t = ;
			// map_set(neural_node_results, node.id, result);
			ui_nodes_hwnd->redraws  = 2;
			ui_view2d_hwnd->redraws = 2;
			project_import_mesh_box(file, true, true, null);
		}
		sys_remove_update(image_to_3d_mesh_node_check_result);
	}
}
