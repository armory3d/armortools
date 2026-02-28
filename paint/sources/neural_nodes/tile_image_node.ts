
function tile_image_node_init() {
	array_push(nodes_material_neural, tile_image_node_def);
	map_set(parser_material_node_vectors, "NEURAL_TILE_IMAGE", neural_node_vector);
	map_set(ui_nodes_custom_buttons, "tile_image_node_button", tile_image_node_button);
}

function tile_image_node_button(node_id: i32) {
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

				let node_name: string = parser_material_node_name(node);
				let h: ui_handle_t    = ui_handle(node_name);
				let model: i32        = ui_nest(h, 0).i;
				let prompt: string    = ui_nest(h, 1).text;

				let tile: gpu_texture_t = gpu_create_render_target(512, 512);
				draw_begin(tile);
				draw_scaled_image(input, -256, -256, 512, 512);
				draw_scaled_image(input, 256, -256, 512, 512);
				draw_scaled_image(input, -256, 256, 512, 512);
				draw_scaled_image(input, 256, 256, 512, 512);
				if (model == 1) {
					draw_set_color(0xffff0000);
					draw_filled_triangle(0, 256, 256, 0, 512, 256);
					draw_filled_triangle(0, 256, 256, 512, 512, 256);
				}
				draw_end();

				// diamond
				let u8a: u8_array_t = u8_array_create(512 * 512);
				for (let i: i32 = 0; i < 512 * 512; ++i) {
					let x: i32 = i % 512;
					let y: i32 = math_floor(i / 512);
					let l: i32 = y < 256 ? y : (511 - y);
					u8a[i]     = (x > 256 - l && x < 256 + l) ? 255 : 0;
					// u8a[i]     = (x > 256 - l && x < 256 + l) ? 128 : 0;
				}
				let mask: gpu_texture_t = gpu_create_texture_from_bytes(u8a, 512, 512, gpu_texture_format_t.R8);

				let dir: string = neural_node_dir();
				iron_write_png(dir + path_sep + "input.png", gpu_get_texture_pixels(tile), tile.width, tile.height, 0);
				iron_write_png(dir + path_sep + "mask.png", gpu_get_texture_pixels(mask), mask.width, mask.height, 1);

				if (prompt == "") {
					prompt = ".";
				}

				let argv: string[];
				if (model == 0) {
					argv = [
						dir + "/" + neural_node_sd_bin(),
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
						"--qwen-image-zero-cond-t",
						"--steps",
						"50",
						"-s",
						"-1",
						"-W",
						"512",
						"-H",
						"512",
						"-p",
						"replace red area by extending image contents. modify red area only",
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
