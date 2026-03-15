
#include "../global.h"

void image_to_pbr_node_init() {
	any_array_push(nodes_material_neural, image_to_pbr_node_def);
	any_map_set(parser_material_node_vectors, "NEURAL_IMAGE_TO_PBR", image_to_pbr_node_vector);
	any_map_set(parser_material_node_values, "NEURAL_IMAGE_TO_PBR", image_to_pbr_node_value);
	any_map_set(ui_nodes_custom_buttons, "image_to_pbr_node_button", image_to_pbr_node_button);
}

char *image_to_pbr_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	gpu_texture_t *result = NULL;
	if (socket == node->outputs->buffer[0]) { // base color
		result = image_to_pbr_node_result_base;
	}
	else if (socket == node->outputs->buffer[4]) { // normal map
		result = image_to_pbr_node_result_normal;
	}

	if (result == NULL) {
		return "float3(0.0, 0.0, 0.0)";
	}
	char *tex_name = string("%s%s", parser_material_node_name(node, NULL), i32_to_string(socket->id));
	any_map_set(data_cached_images, tex_name, result);
	bind_tex_t *tex      = parser_material_make_bind_tex(tex_name, tex_name);
	char       *texstore = parser_material_texture_store(node, tex, tex_name, COLOR_SPACE_AUTO);
	return string("%s.rgb", texstore);
}

char *image_to_pbr_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	gpu_texture_t *result = NULL;
	if (socket == node->outputs->buffer[1]) { // occlusion
		result = image_to_pbr_node_result_occlusion;
	}
	else if (socket == node->outputs->buffer[2]) { // roughness
		result = image_to_pbr_node_result_roughness;
	}
	else if (socket == node->outputs->buffer[3]) { // metallic
		result = image_to_pbr_node_result_metallic;
	}
	else if (socket == node->outputs->buffer[5]) { // height
		result = image_to_pbr_node_result_height;
	}

	if (result == NULL) {
		return "0.0";
	}

	char *tex_name = string("%s%s", parser_material_node_name(node, NULL), i32_to_string(socket->id));
	any_map_set(data_cached_images, tex_name, result);
	bind_tex_t *tex      = parser_material_make_bind_tex(tex_name, tex_name);
	char       *texstore = parser_material_texture_store(node, tex, tex_name, COLOR_SPACE_AUTO);
	return string("%s.r", texstore);
}

void image_to_pbr_node_run_sd(char *model, char *prompt, void (*done)(gpu_texture_t *)) {
	char             *dir  = neural_node_dir();
	string_t_array_t *argv = any_array_create_from_raw(
	    (void *[]){
	        string("%s/%s", dir, neural_node_sd_bin()),
	        "-m",
	        string("%s/%s", dir, model),
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
	        string("%s/input.png", dir),
	        "-o",
	        string("%s/output.png", dir),
	        NULL,
	    },
	    20);

	iron_exec_async(argv->buffer[0], argv->buffer);
	sys_notify_on_update(image_to_pbr_node_check_result, done);
}

void image_to_pbr_node_all_done(void *_) {
	render_target_t *occmap;
	{
		render_target_t *t = render_target_create();
		t->name            = "occmap";
		t->width           = 2048;
		t->height          = 2048;
		t->format          = "RGBA32";
		render_path_create_render_target(t);
		occmap = t;
	}
	{
		render_target_t *t = render_target_create();
		t->name            = "_height_map";
		t->width           = 768;
		t->height          = 768;
		t->format          = "RGBA32";
		t->_image          = image_to_pbr_node_result_height;
		any_map_set(render_path_render_targets, t->name, t);
	}
	{
		render_target_t *t = render_target_create();
		t->name            = "_normal_map";
		t->width           = 768;
		t->height          = 768;
		t->format          = "RGBA32";
		t->_image          = image_to_pbr_node_result_normal;
		any_map_set(render_path_render_targets, t->name, t);
	}

	render_path_load_shader("Scene/depth_to_ao_pass/depth_to_ao_pass");
	render_path_set_target("occmap", NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
	render_path_bind_target("_height_map", "height_map");
	render_path_bind_target("_normal_map", "normal_map");
	render_path_draw_shader("Scene/depth_to_ao_pass/depth_to_ao_pass");
	gc_unroot(image_to_pbr_node_result_occlusion);
	image_to_pbr_node_result_occlusion = occmap->_image;
	gc_root(image_to_pbr_node_result_occlusion);
	// blur
}

void image_to_pbr_node_roughness_done(gpu_texture_t *tex) {
	gc_unroot(image_to_pbr_node_result_roughness);
	image_to_pbr_node_result_roughness = tex;
	gc_root(image_to_pbr_node_result_roughness);
	// image_to_pbr_node_run_sd("marigold-iid-lighting-v1-1.q8_0.gguf", "_diffuse_shading", function(tex: gpu_texture_t) {
	sys_notify_on_next_frame(&image_to_pbr_node_all_done, NULL);
}

void image_to_pbr_node_base_done(gpu_texture_t *tex) {
	gc_unroot(image_to_pbr_node_result_base);
	image_to_pbr_node_result_base = tex;
	gc_root(image_to_pbr_node_result_base);
	image_to_pbr_node_run_sd("marigold-iid-appearance-v1-1.q8_0.gguf", "_roughness", &image_to_pbr_node_roughness_done);
}

void image_to_pbr_node_height_done(gpu_texture_t *tex) {
	gc_unroot(image_to_pbr_node_result_height);
	image_to_pbr_node_result_height = tex;
	gc_root(image_to_pbr_node_result_height);
	image_to_pbr_node_run_sd("marigold-iid-lighting-v1-1.q8_0.gguf", "_base", &image_to_pbr_node_base_done);
}

void image_to_pbr_node_normals_done(gpu_texture_t *tex) {
	gc_unroot(image_to_pbr_node_result_normal);
	image_to_pbr_node_result_normal = tex;
	gc_root(image_to_pbr_node_result_normal);
	image_to_pbr_node_run_sd("marigold-depth-v1-1.q8_0.gguf", "_height", &image_to_pbr_node_height_done);
}

void image_to_pbr_node_button(i32 node_id) {
	ui_node_canvas_t *canvas    = ui_nodes_get_canvas(true);
	ui_node_t        *node      = ui_get_node(canvas->nodes, node_id);
	char             *node_name = parser_material_node_name(node, NULL);
	ui_handle_t      *h         = ui_handle(node_name);
	string_t_array_t *models    = any_array_create_from_raw(
        (void *[]){
            "Marigold",
        },
        1);
	i32 model = ui_combo(ui_nest(h, 0), models, tr("Model"), false, UI_ALIGN_LEFT, true);
	if (neural_node_button(node, models->buffer[model])) {
		ui_node_t     *from_node = neural_from_node(node->inputs->buffer[0], 0);
		gpu_texture_t *input     = ui_nodes_get_node_preview_image(from_node);
		if (input != NULL) {
			char *dir = neural_node_dir();

#ifdef IRON_BGRA
			buffer_t *input_buf = export_arm_bgra_swap(gpu_get_texture_pixels(input)); // Vulkan non-rt textures need a flip
#else
			buffer_t *input_buf = gpu_get_texture_pixels(input);
#endif
			iron_write_png(string("%s%sinput.png", dir, PATH_SEP), input_buf, input->width, input->height, 0);

			image_to_pbr_node_run_sd("marigold-normals-v1-1.q8_0.gguf", "_normals", &image_to_pbr_node_normals_done);
		}
	}
}

void image_to_pbr_node_check_result(void (*done)(gpu_texture_t *)) {
	iron_delay_idle_sleep();
	if (iron_exec_async_done == 1) {
		char *dir  = neural_node_dir();
		char *file = string("%s%soutput.png", dir, PATH_SEP);
		if (iron_file_exists(file)) {
			gpu_texture_t *tex = iron_load_texture(file);
			done(tex);
		}
		sys_remove_update(image_to_pbr_node_check_result);
	}
}
