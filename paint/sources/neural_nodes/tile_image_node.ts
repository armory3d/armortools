
function tile_image_node_init() {
	array_push(nodes_material_neural, tile_image_node_def);
	map_set(parser_material_node_vectors, "NEURAL_TILE_IMAGE", neural_node_vector);
	map_set(ui_nodes_custom_buttons, "tile_image_node_button", tile_image_node_button);
}

function tile_image_node_button(node_id: i32) {
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let node: ui_node_t          = ui_get_node(canvas.nodes, node_id);

	let models: string[] = [ "Qwen Image Edit" ];
	let model: i32       = ui_combo(ui_handle(__ID__), models, tr("Model"));

	if (neural_node_button()) {
		let from_node: ui_node_t = neural_from_node(node.inputs[0]);
		let input: gpu_texture_t = ui_nodes_get_node_preview_image(from_node);
		if (input != null) {

			// sys_notify_on_next_frame(function(input: gpu_texture_t) {

				let tile: gpu_texture_t = gpu_create_render_target(512, 512);
				draw_begin(tile);
				draw_scaled_image(input, -256, -256, 512, 512);
				draw_scaled_image(input, 256, -256, 512, 512);
				draw_scaled_image(input, -256, 256, 512, 512);
				draw_scaled_image(input, 256, 256, 512, 512);
				draw_end();

				let dir: string = neural_node_dir();
				iron_write_png(dir + path_sep + "input.png", gpu_get_texture_pixels(tile), tile.width, tile.height, 0);

				let prompt: string   = "Blend seams";
				let negative: string = "";

				let argv: string[] = [
					dir + "/sd_qie",
					"--diffusion-model",
					dir + "/Qwen-Image-Edit-2509-Q8_0.gguf",
					"--vae",
					dir + "/qwen_image_vae.safetensors",
					"--qwen2vl",
					dir + "/Qwen2.5-VL-7B-Instruct-Q8_0.gguf",
					"--qwen2vl_vision",
					dir + "/Qwen2.5-VL-7B-Instruct.mmproj-Q8_0.gguf",
					"--sampling-method",
					"euler",
					"--offload-to-cpu",
					"--cfg-scale",
					"2.5",
					"--flow-shift",
					"3",
					"--steps",
					"20",
					"-s",
					"-1",
					"-W",
					"512",
					"-H",
					"512",
					"-p",
					prompt,
					"-n",
					"'" + negative + "'",
					"-r",
					dir + "/input.png",
					"-o",
					dir + "/output.png",
					null
				];

				iron_exec_async(argv[0], argv.buffer);
				sys_notify_on_update(neural_node_check_result, node);

			// }, input);
		}
	}
}

let tile_image_node_def: ui_node_t = {
	id : 0,
	name : _tr("Tile Image"),
	type : "NEURAL_TILE_IMAGE",
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
		name : "tile_image_node_button",
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
