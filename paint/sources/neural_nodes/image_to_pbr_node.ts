
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
	let dir: string;
	if (path_is_protected()) {
		dir = iron_internal_save_path() + "models";
	}
	else {
		dir = iron_internal_files_location() + path_sep + "models";
	}

	let argv: string[] = [
		dir + "/sd",
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
		"'" + prompt + "'",
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

	let models: string[] = [ "Marigold" ];
	let model: i32       = ui_combo(ui_handle(__ID__), models, tr("Model"));

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

			image_to_pbr_node_run_sd("marigold-normals-v1-1.safetensors", " ", function(tex: gpu_texture_t) {
				image_to_pbr_node_result_normal = tex;
				image_to_pbr_node_run_sd("marigold-depth-v1-1.safetensors", " ", function(tex: gpu_texture_t) {
					image_to_pbr_node_result_height = tex;
					image_to_pbr_node_run_sd("marigold-iid-appearance-v1-1.safetensors", " ", function(tex: gpu_texture_t) {
						image_to_pbr_node_result_base = tex;
						image_to_pbr_node_run_sd("marigold-iid-appearance-v1-1.safetensors", "_roughness", function(tex: gpu_texture_t) {
							image_to_pbr_node_result_roughness = tex;
						});
					});
				});
			});
		}
	}
}

function image_to_pbr_node_check_result(done: (tex: gpu_texture_t) => void) {
	if (iron_exec_async_done == 1) {

		let dir: string;
		if (path_is_protected()) {
			dir = iron_internal_save_path() + "models";
		}
		else {
			dir = iron_internal_files_location() + path_sep + "models";
		}

		let file: string = dir + path_sep + "output.png";
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
