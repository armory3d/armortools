
#include "../global.h"

void shader_node_init() {
	any_array_push(nodes_material_input, shader_node_def);
	any_map_set(parser_material_node_values, "SHADER_GPU", shader_node_value);
}

char *shader_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	buffer_t *shader = node->buttons->buffer[0]->default_value;
	char *str    = sys_buffer_to_string(shader);
	return string_equals(str, "") ? "0.0" : str;
}
