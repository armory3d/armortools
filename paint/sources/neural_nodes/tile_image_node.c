
#include "../global.h"

void tile_image_node_init() {
	any_array_push(nodes_material_neural, tile_image_node_def);
	any_map_set(parser_material_node_vectors, "NEURAL_TILE_IMAGE", neural_node_vector);
	any_map_set(ui_nodes_custom_buttons, "tile_image_node_button", tile_image_node_button);
}

void tile_image_node_button_on_next_frame(ui_node_t *node) {
	ui_node_t     *from_node = neural_from_node(node->inputs->buffer[0], 0);
	gpu_texture_t *input     = ui_nodes_get_node_preview_image(from_node);
	if (input != NULL) {
		char          *node_name = parser_material_node_name(node, NULL);
		ui_handle_t   *h         = ui_handle(node_name);
		i32            model     = ui_nest(h, 0)->i;
		char          *prompt    = ui_nest(h, 1)->text;
		gpu_texture_t *tile      = gpu_create_render_target(512, 512, GPU_TEXTURE_FORMAT_RGBA32);
		draw_begin(tile, false, 0);
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
		u8_array_t *u8a = u8_array_create(512 * 512);
		for (i32 i = 0; i < 512 * 512; ++i) {
			i32 x          = i % 512;
			i32 y          = math_floor(i / (float)512);
			i32 l          = y < 256 ? y : (511 - y);
			u8a->buffer[i] = (x > 256 - l && x < 256 + l) ? 255 : 0;
			// u8a[i]     = (x > 256 - l && x < 256 + l) ? 128 : 0;
		}
		gpu_texture_t *mask = gpu_create_texture_from_bytes(u8a, 512, 512, GPU_TEXTURE_FORMAT_R8);
		char          *dir  = neural_node_dir();
		iron_write_png(string("%s%sinput.png", dir, PATH_SEP), gpu_get_texture_pixels(tile), tile->width, tile->height, 0);
		iron_write_png(string("%s%smask.png", dir, PATH_SEP), gpu_get_texture_pixels(mask), mask->width, mask->height, 1);

		if (string_equals(prompt, "")) {
			prompt = ".";
		}

		string_t_array_t *argv;
		if (model == 0) {
			argv = any_array_create_from_raw(
			    (void *[]){
			        string("%s/%s", dir, neural_node_sd_bin()),
			        "-m",
			        string("%s/v1-5-pruned-emaonly.safetensors", dir),
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
			        string("%s/input.png", dir),
			        "--mask",
			        string("%s/mask.png", dir),
			        "-o",
			        string("%s/output.png", dir),
			        NULL,
			    },
			    21);
		}
		else {
			argv = any_array_create_from_raw(
			    (void *[]){
			        string("%s/%s", dir, neural_node_sd_bin()),
			        "--diffusion-model",
			        string("%s/qwen-image-edit-2511-Q4_K_S.gguf", dir),
			        "--vae",
			        string("%s/Qwen_Image-VAE.safetensors", dir),
			        "--llm",
			        string("%s/Qwen2.5-VL-7B-Instruct-Q4_K_S.gguf", dir),
			        "--llm_vision",
			        string("%s/mmproj-F16.gguf", dir),
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
			        string("%s/input.png", dir),
			        "-o",
			        string("%s/output.png", dir),
			        NULL,
			    },
			    27);
		}

		iron_exec_async(argv->buffer[0], argv->buffer);
		sys_notify_on_update(neural_node_check_result, node);
	}
}

void tile_image_node_button(i32 node_id) {
	ui_node_canvas_t *canvas    = ui_nodes_get_canvas(true);
	ui_node_t        *node      = ui_get_node(canvas->nodes, node_id);
	char             *node_name = parser_material_node_name(node, NULL);
	ui_handle_t      *h         = ui_handle(node_name);
	string_t_array_t *models    = any_array_create_from_raw(
        (void *[]){
            "Stable Diffusion",
            "Qwen Image Edit",
        },
        2);
	i32   model                      = ui_combo(ui_nest(h, 0), models, tr("Model"), false, UI_ALIGN_LEFT, true);
	char *prompt                     = ui_text_area(ui_nest(h, 1), UI_ALIGN_LEFT, true, tr("prompt"), true);
	node->buttons->buffer[0]->height = string_split(prompt, "\n")->length + 2;

	if (neural_node_button(node, models->buffer[model])) {
		sys_notify_on_next_frame(&tile_image_node_button_on_next_frame, node);
	}
}
