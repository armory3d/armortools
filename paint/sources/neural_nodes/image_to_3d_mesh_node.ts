
function image_to_3d_mesh_node_init() {
	array_push(nodes_material_neural, image_to_3d_mesh_node_def);
	map_set(parser_material_node_values, "NEURAL_IMAGE_TO_3D_MESH", neural_node_value);
	map_set(ui_nodes_custom_buttons, "image_to_3d_mesh_node_button", image_to_3d_mesh_node_button);
}

function image_to_3d_mesh_node_remove_background(buffer: buffer_t): buffer_t {
	for (let i: i32 = 0; i < math_floor((buffer.length) / 4); ++i) {
		if (buffer[i * 4] <= 16 && buffer[i * 4 + 1] <= 16 && buffer[i * 4 + 2] <= 16) {
			buffer[i * 4 + 3] = 0;
		}
	}
	return buffer;
}

function image_to_3d_mesh_node_button(node_id: i32) {
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let node: ui_node_t          = ui_get_node(canvas.nodes, node_id);
	let node_name: string = parser_material_node_name(node);
	let h: ui_handle_t = ui_handle(node_name);

	let models: string[] = [ "Hunyuan3D" ];
	let model: i32       = ui_combo(ui_nest(h, 0), models, tr("Model"));

	if (neural_node_button(node, models[model])) {
		let from_node: ui_node_t = neural_from_node(node.inputs[0], 0);
		let input: gpu_texture_t = ui_nodes_get_node_preview_image(from_node);
		if (input != null) {
			let dir: string = neural_node_dir();

			/// if IRON_BGRA
			let input_buf: buffer_t = image_to_3d_mesh_node_remove_background(export_arm_bgra_swap(gpu_get_texture_pixels(input)));
			/// else
			let input_buf: buffer_t = image_to_3d_mesh_node_remove_background(gpu_get_texture_pixels(input));
			/// end
			iron_write_png(dir + path_sep + "input.png", input_buf, input.width, input.height, 0);

			dir = string_replace_all(dir, "\\", "/");
			let argv: string[] = [
				dir + "/Hunyuan3D_win64/python/python.exe",
				"-s",
				"-B",
				"-c",
				"from hy3dshape.pipelines import Hunyuan3DDiTFlowMatchingPipeline; shape_pipeline = Hunyuan3DDiTFlowMatchingPipeline.from_pretrained('" + dir + "/Hunyuan3D_win64/Hunyuan3D-2.1'); mesh = shape_pipeline(image='" + dir + "/input.png', octree_resolution=512, num_chunks=64000)[0]; mesh.export('" + dir + "/output.obj')",
				null
			];

			iron_exec_async(argv[0], argv.buffer);
			sys_notify_on_update(image_to_3d_mesh_node_check_result, node);
		}
	}
}

function image_to_3d_mesh_node_check_result(node: ui_node_t) {
	neural_node_current = node;
	iron_delay_idle_sleep();
	if (iron_exec_async_done == 1) {
		let file: string = neural_node_dir() + path_sep + "output.obj";
		if (iron_file_exists(file)) {
			// let result: gpu_texture_t = ;
			// map_set(neural_node_results, node.id, result);
			ui_nodes_hwnd.redraws  = 2;
			ui_view2d_hwnd.redraws = 2;
			project_import_mesh_box(file, true, true, null);
		}
		sys_remove_update(image_to_3d_mesh_node_check_result);
	}
}

let image_to_3d_mesh_node_def: ui_node_t = {
	id : 0,
	name : _tr("Image to 3D Mesh"),
	type : "NEURAL_IMAGE_TO_3D_MESH",
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
		name : _tr("Mesh"),
		type : "VALUE",
		color : 0xffa1a1a1,
		default_value : f32_array_create_x(1.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [ {
		name : "image_to_3d_mesh_node_button",
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
