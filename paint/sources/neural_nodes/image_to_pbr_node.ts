
let image_to_pbr_node_result_base: gpu_texture_t      = null;
let image_to_pbr_node_result_normal: gpu_texture_t    = null;
let image_to_pbr_node_result_occlusion: gpu_texture_t = null;
let image_to_pbr_node_result_height: gpu_texture_t    = null;
let image_to_pbr_node_result_roughness: gpu_texture_t = null;
let image_to_pbr_node_result_metallic: gpu_texture_t  = null;

function image_to_pbr_node_init() {
	array_push(nodes_material_neural, image_to_pbr_node_def);
	map_set(parser_material_node_vectors, "NEURAL_IMAGE_TO_PBR", image_to_pbr_node_vector);
	map_set(parser_material_node_values, "NEURAL_IMAGE_TO_PBR", image_to_pbr_node_value);
	map_set(ui_nodes_custom_buttons, "image_to_pbr_node_button", image_to_pbr_node_button);
}

function image_to_pbr_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let result: gpu_texture_t = null;
	if (socket == node.outputs[0]) { // base color
		result = image_to_pbr_node_result_base;
	}
	else if (socket == node.outputs[4]) { // normal map
		result = image_to_pbr_node_result_normal;
	}

	if (result == null) {
		return "float3(0.0, 0.0, 0.0)";
	}
	let tex_name: string = string_join(parser_material_node_name(node), i32_to_string(socket.id));
	map_set(data_cached_images, tex_name, result);
	let tex: bind_tex_t  = parser_material_make_bind_tex(tex_name, tex_name);
	let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
	return texstore + ".rgb";
}

function image_to_pbr_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	let result: gpu_texture_t = null;
	if (socket == node.outputs[1]) { // occlusion
		result = image_to_pbr_node_result_occlusion;
	}
	else if (socket == node.outputs[2]) { // roughness
		result = image_to_pbr_node_result_roughness;
	}
	else if (socket == node.outputs[3]) { // metallic
		result = image_to_pbr_node_result_metallic;
	}
	else if (socket == node.outputs[5]) { // height
		result = image_to_pbr_node_result_height;
	}

	if (result == null) {
		return "0.0";
	}

	let tex_name: string = string_join(parser_material_node_name(node), i32_to_string(socket.id));
	map_set(data_cached_images, tex_name, result);
	let tex: bind_tex_t  = parser_material_make_bind_tex(tex_name, tex_name);
	let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
	return texstore + ".r";
}

function image_to_pbr_node_run_sd(model: string, prompt: string, done: (tex: gpu_texture_t) => void) {
	let dir: string = neural_node_dir();

	let argv: string[] = [
		dir + "/" + neural_node_sd_bin(),
		"-m",
		dir + "/" + model,
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
		prompt,
		"-i",
		dir + "/input.png",
		"-o",
		dir + "/output.png",
		null
	];

	iron_exec_async(argv[0], argv.buffer);
	sys_notify_on_update(image_to_pbr_node_check_result, done);
}

function image_to_pbr_node_button(node_id: i32) {
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let node: ui_node_t          = ui_get_node(canvas.nodes, node_id);
	let node_name: string = parser_material_node_name(node);
	let h: ui_handle_t = ui_handle(node_name);

	let models: string[] = [ "Marigold" ];
	let model: i32       = ui_combo(ui_nest(h, 0), models, tr("Model"));

	if (neural_node_button(node, models[model])) {
		let from_node: ui_node_t = neural_from_node(node.inputs[0], 0);
		let input: gpu_texture_t = ui_nodes_get_node_preview_image(from_node);
		if (input != null) {
			let dir: string = neural_node_dir();

			/// if IRON_BGRA
			let input_buf: buffer_t = export_arm_bgra_swap(gpu_get_texture_pixels(input)); // Vulkan non-rt textures need a flip
			/// else
			let input_buf: buffer_t = gpu_get_texture_pixels(input);
			/// end
			iron_write_png(dir + PATH_SEP + "input.png", input_buf, input.width, input.height, 0);

			image_to_pbr_node_run_sd("marigold-normals-v1-1.q8_0.gguf", "_normals", function(tex: gpu_texture_t) {
				image_to_pbr_node_result_normal = tex;
				image_to_pbr_node_run_sd("marigold-depth-v1-1.q8_0.gguf", "_height", function(tex: gpu_texture_t) {
					image_to_pbr_node_result_height = tex;
					image_to_pbr_node_run_sd("marigold-iid-lighting-v1-1.q8_0.gguf", "_base", function(tex: gpu_texture_t) {
						image_to_pbr_node_result_base = tex;
						image_to_pbr_node_run_sd("marigold-iid-appearance-v1-1.q8_0.gguf", "_roughness", function(tex: gpu_texture_t) {
						// image_to_pbr_node_run_sd("marigold-iid-lighting-v1-1.q8_0.gguf", "_diffuse_shading", function(tex: gpu_texture_t) {
							image_to_pbr_node_result_roughness = tex;

							sys_notify_on_next_frame(function(_: any) {
								let occmap: render_target_t;
								{
									let t: render_target_t = render_target_create();
									t.name                 = "occmap";
									t.width                = 2048;
									t.height               = 2048;
									t.format               = "RGBA32";
									render_path_create_render_target(t);
									occmap = t;
								}
								{
									let t: render_target_t = render_target_create();
									t.name                 = "_height_map";
									t.width                = 768;
									t.height               = 768;
									t.format               = "RGBA32";
									t._image = image_to_pbr_node_result_height;
									map_set(render_path_render_targets, t.name, t);
								}
								{
									let t: render_target_t = render_target_create();
									t.name                 = "_normal_map";
									t.width                = 768;
									t.height               = 768;
									t.format               = "RGBA32";
									t._image = image_to_pbr_node_result_normal;
									map_set(render_path_render_targets, t.name, t);
								}

								render_path_load_shader("Scene/depth_to_ao_pass/depth_to_ao_pass");
								render_path_set_target("occmap");
								render_path_bind_target("_height_map", "height_map");
								render_path_bind_target("_normal_map", "normal_map");
								render_path_draw_shader("Scene/depth_to_ao_pass/depth_to_ao_pass");
								image_to_pbr_node_result_occlusion = occmap._image;
								// blur
							});
						});
					});
				});
			});
		}
	}
}

function image_to_pbr_node_check_result(done: (tex: gpu_texture_t) => void) {
	iron_delay_idle_sleep();
	if (iron_exec_async_done == 1) {
		let dir: string = neural_node_dir();
		let file: string = dir + PATH_SEP + "output.png";
		if (iron_file_exists(file)) {
			let tex: gpu_texture_t = iron_load_texture(file);
			done(tex);
		}
		sys_remove_update(image_to_pbr_node_check_result);
	}
}

let image_to_pbr_node_def: ui_node_t = {
	id : 0,
	name : _tr("Image to PBR"),
	type : "NEURAL_IMAGE_TO_PBR",
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
	outputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("Base Color"),
			type : "RGBA",
			color : 0xffc7c729,
			default_value : f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Occlusion"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Roughness"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Metallic"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Normal Map"),
			type : "VECTOR",
			color : 0xffc7c729,
			default_value : f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Height"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	buttons : [ {
		name : "image_to_pbr_node_button",
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
