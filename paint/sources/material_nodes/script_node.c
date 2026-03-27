
#include "../global.h"

void script_node_init() {
	any_array_push(nodes_material_input, script_node_def);
	any_map_set(parser_material_node_values, "SCRIPT_CPU", script_node_value);
	any_map_set(ui_nodes_custom_buttons, "script_node_button", script_node_button);
}

char *script_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	if (parser_material_script_links == NULL) {
		parser_material_script_links = any_map_create();
		gc_root(parser_material_script_links);
	}
	buffer_t *script = node->buttons->buffer[0]->default_value;
	char     *str    = sys_buffer_to_string(script);
	char     *link   = parser_material_node_name(node, NULL);
	any_map_set(parser_material_script_links, link, str);
	node_shader_add_constant(parser_material_kong, string("%s: float", link), string("_%s", link));
	return string("constants.%s", link);
}

ui_node_t *script_node_draw_snippets_node = NULL;

void script_node_draw_snippets() {
	if (ui_menu_button("sys_time()", "", ICON_DRAFT)) {
		ui_node_t *node                                 = script_node_draw_snippets_node;
		node->buttons->buffer[0]->default_value->buffer = "sys_time()";
	}
}

void script_node_button(i32 node_id) {
	ui_node_t   *node      = ui_get_node(ui_nodes_get_canvas(true)->nodes, node_id);
	char        *node_name = parser_material_node_name(node, NULL);
	ui_handle_t *h         = ui_handle(node_name);

	if (ui_button(tr("Snippets"), UI_ALIGN_CENTER, "")) {
		script_node_draw_snippets_node = node;
		ui_menu_draw(&script_node_draw_snippets, -1, -1);
	}
}
