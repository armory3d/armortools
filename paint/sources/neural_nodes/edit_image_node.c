
#include "../global.h"

void edit_image_node_init() {
	any_array_push(nodes_material_neural, edit_image_node_def);
	any_map_set(parser_material_node_vectors, "NEURAL_EDIT_IMAGE", neural_node_vector);
	any_map_set(ui_nodes_custom_buttons, "edit_image_node_button", edit_image_node_button);
}

void edit_image_node_button(i32 node_id) {
	ui_node_canvas_t *canvas    = ui_nodes_get_canvas(true);
	ui_node_t        *node      = ui_get_node(canvas->nodes, node_id);
	char             *node_name = parser_material_node_name(node, NULL);
	ui_handle_t      *h         = ui_handle(node_name);

	string_t_array_t *models = any_array_create_from_raw(
	    (void *[]){
	        "Qwen Image Edit",
	    },
	    1);
	i32   model                      = ui_combo(ui_nest(h, 0), models, tr("Model"), false, UI_ALIGN_LEFT, true);
	char *prompt                     = ui_text_area(ui_nest(h, 1), UI_ALIGN_LEFT, true, tr("prompt"), true);
	node->buttons->buffer[0]->height = string_split(prompt, "\n")->length + 2;

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
			iron_write_png(string("%s%sinput.png", dir, PATH_SEP), input_buf, input->width, input->height, 0);

			if (string_equals(prompt, "")) {
				prompt = ".";
			}

			string_t_array_t *argv = any_array_create_from_raw(
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
			        "--steps",
			        "30",
			        "-s",
			        "-1",
			        "--cfg-scale",
			        "2.5",
			        "--flow-shift",
			        "3",
			        "--qwen-image-zero-cond-t",
			        "-W",
			        "512",
			        "-H",
			        "512",
			        "-p",
			        prompt,
			        "-r",
			        string("%s/input.png", dir),
			        "-o",
			        string("%s/output.png", dir),
			        NULL,
			    },
			    31);

			iron_exec_async(argv->buffer[0], argv->buffer);
			sys_notify_on_update(neural_node_check_result, node);
		}
	}
}
