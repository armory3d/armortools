
#include "../global.h"

string_array_t *text_to_image_node_sd_args(char *dir, char *prompt) {
	string_array_t *argv = any_array_create_from_raw(
	    (void *[]){
	        string("%s/%s", dir, neural_node_sd_bin()),
	        "-m",
	        string("%s/v1-5-pruned-emaonly.safetensors", dir),
	        "--offload-to-cpu",
	        "-W",
	        "512",
	        "-H",
	        "512",
	        "--steps",
	        "40",
	        "-s",
	        "-1",
	        "-o",
	        string("%s/output.png", dir),
	        "-p",
	        string("'%s'", prompt),
	        NULL,
	    },
	    17);
	return argv;
}

string_array_t *text_to_image_node_zimage_args(char *dir, char *prompt) {
	string_array_t *argv = any_array_create_from_raw(
	    (void *[]){
	        string("%s/%s", dir, neural_node_sd_bin()),
	        "--diffusion-model",
	        string("%s/z_image_turbo-Q4_K.gguf", dir),
	        "--vae",
	        string("%s/ae.safetensors", dir),
	        "--llm",
	        string("%s/Qwen3-4B-Instruct-2507-Q4_K_S.gguf", dir),
	        "--diffusion-fa",
	        "--offload-to-cpu",
	        "--cfg-scale",
	        "1.0",
	        "-W",
	        "512",
	        "-H",
	        "512",
	        "--steps",
	        "40",
	        "-s",
	        "-1",
	        "-o",
	        string("%s/output.png", dir),
	        "-p",
	        string("'%s'", prompt),
	        NULL,
	    },
	    24);
	return argv;
}

string_array_t *text_to_image_node_qwen_args(char *dir, char *prompt) {
	string_array_t *argv = any_array_create_from_raw(
	    (void *[]){
	        string("%s/%s", dir, neural_node_sd_bin()),
	        "--diffusion-model",
	        string("%s/qwen-image-2512-Q4_K_S.gguf", dir),
	        "--vae",
	        string("%s/Qwen_Image-VAE.safetensors", dir),
	        "--llm",
	        string("%s/Qwen2.5-VL-7B-Instruct-Q4_K_S.gguf", dir),
	        "--llm_vision",
	        string("%s/mmproj-F16.gguf", dir),
	        "--sampling-method",
	        "euler",
	        "--offload-to-cpu",
	        "-W",
	        "512",
	        "-H",
	        "512",
	        "--steps",
	        "20",
	        "-s",
	        "-1",
	        "-o",
	        string("%s/output.png", dir),
	        "-p",
	        string("'%s'", prompt),
	        NULL,
	    },
	    25);
	return argv;
}

string_array_t *text_to_image_node_wan_args(char *dir, char *prompt) {
	string_array_t *argv = any_array_create_from_raw(
	    (void *[]){
	        string("%s/%s", dir, neural_node_sd_bin()),
	        "-M",
	        "vid_gen",
	        "--diffusion-model",
	        string("%s/Wan2.2-T2V-A14B-LowNoise-Q4_K_S.gguf", dir),
	        "--high-noise-diffusion-model",
	        string("%s/Wan2.2-T2V-A14B-HighNoise-Q4_K_S.gguf", dir),
	        "--vae",
	        string("%s/Wan2.1_VAE.safetensors", dir),
	        "--t5xxl",
	        string("%s/umt5-xxl-encoder-Q4_K_S.gguf", dir),
	        "--sampling-method",
	        "euler",
	        "--steps",
	        "20",
	        "--high-noise-sampling-method",
	        "euler",
	        "--high-noise-steps",
	        "10",
	        "-W",
	        "512",
	        "-H",
	        "512",
	        "--offload-to-cpu",
	        "-s",
	        "-1",
	        "-o",
	        string("%s/output.png", dir),
	        "-p",
	        prompt,
	        NULL,
	    },
	    31);
	return argv;
}

void text_to_image_node_button(i32 node_id) {
	ui_node_t      *node      = ui_get_node(ui_nodes_get_canvas(true)->nodes, node_id);
	char           *node_name = parser_material_node_name(node, NULL);
	ui_handle_t    *h         = ui_handle(node_name);
	string_array_t *models    = any_array_create_from_raw(
        (void *[]){
            "Stable Diffusion",
            "Z-Image-Turbo",
            "Qwen Image",
            "Wan",
        },
        4);
	i32   model                      = ui_combo(ui_nest(h, 0), models, tr("Model"), false, UI_ALIGN_LEFT, true);
	char *prompt                     = ui_text_area(ui_nest(h, 1), UI_ALIGN_LEFT, true, tr("prompt"), true);
	node->buttons->buffer[0]->height = string_split(prompt, "\n")->length + 2;

	if (neural_node_button(node, models->buffer[model])) {
		char *dir = neural_node_dir();

		if (string_equals(prompt, "")) {
			prompt = ".";
		}

		string_array_t *argv;
		if (model == 0) {
			argv = text_to_image_node_sd_args(dir, prompt);
		}
		else if (model == 1) {
			argv = text_to_image_node_zimage_args(dir, prompt);
		}
		else if (model == 2) {
			argv = text_to_image_node_qwen_args(dir, prompt);
		}
		else {
			argv = text_to_image_node_wan_args(dir, prompt);
		}

		if (node->buttons->buffer[1]->default_value->buffer[0] > 0.0) {
			array_insert(argv, argv->length - 1, "--circular");
		}

		iron_exec_async(argv->buffer[0], argv->buffer);
		sys_notify_on_update(neural_node_check_result, node);
	}
}

void text_to_image_node_init() {

	ui_node_t *text_to_image_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                              .name    = _tr("Text to Image"),
	                              .type    = "NEURAL_TEXT_TO_IMAGE",
	                              .x       = 0,
	                              .y       = 0,
	                              .color   = 0xff4982a0,
	                              .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Color"),
	                                                                       .type          = "RGBA",
	                                                                       .color         = 0xffc7c729,
	                                                                       .default_value = f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  1),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = "text_to_image_node_button",
	                                                                       .type          = "CUSTOM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 1}),
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Tiled"),
	                                                                       .type          = "BOOL",
	                                                                       .output        = 0,
	                                                                       .default_value = f32_array_create_x(0),
	                                                                       .data          = NULL,
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  2),
	                              .width = 0,
	                              .flags = 0});

	any_array_push(nodes_material_neural, text_to_image_node_def);
	any_map_set(parser_material_node_vectors, "NEURAL_TEXT_TO_IMAGE", neural_node_vector);
	any_map_set(ui_nodes_custom_buttons, "text_to_image_node_button", text_to_image_node_button);
}
