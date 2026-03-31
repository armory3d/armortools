
#include "../global.h"

void camera_data_node_init() {

	camera_data_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id      = 0,
	                              .name    = _tr("Camera Data"),
	                              .type    = "CAMERA",
	                              .x       = 0,
	                              .y       = 0,
	                              .color   = 0xffb34f5a,
	                              .inputs  = any_array_create_from_raw((void *[]){}, 0),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("View Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("View Z Depth"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("View Distance"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .buttons = any_array_create_from_raw((void *[]){}, 0),
	                              .width   = 0,
	                              .flags   = 0});
	gc_root(camera_data_node_def);

	any_array_push(nodes_material_input, camera_data_node_def);
	any_map_set(parser_material_node_vectors, "CAMERA", camera_data_node_vector);
	any_map_set(parser_material_node_values, "CAMERA", camera_data_node_value);
}

char *camera_data_node_vector(ui_node_t *node, ui_node_socket_t *socket) {
	parser_material_kong->frag_vvec_cam = true;
	return "vvec_cam";
}

char *camera_data_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	if (socket == node->outputs->buffer[1]) { // View Z Depth
		node_shader_add_constant(parser_material_kong, "camera_proj: float2", "_camera_plane_proj");
		parser_material_kong->frag_wvpposition = true;
		return "(constants.camera_proj.y / ((input.wvpposition.z / input.wvpposition.w) - constants.camera_proj.x))";
	}
	else { // View Distance
		node_shader_add_constant(parser_material_kong, "eye: float3", "_camera_pos");
		parser_material_kong->frag_wposition = true;
		return "distance(constants.eye, input.wposition)";
	}
}
