
#include "global.h"

void ui_viewnodes_init() {
	gc_unroot(ui_nodes_preview_image);
	ui_nodes_preview_image = ui_nodes_get_node_preview_image;
	gc_root(ui_nodes_preview_image);
	gc_unroot(ui_nodes_on_link_drag);
	ui_nodes_on_link_drag = ui_viewnodes_on_link_drag;
	gc_root(ui_nodes_on_link_drag);
	gc_unroot(ui_nodes_on_node_remove);
	ui_nodes_on_node_remove = ui_viewnodes_on_node_remove;
	gc_root(ui_nodes_on_node_remove);
	gc_unroot(ui_nodes_on_node_changed);
	ui_nodes_on_node_changed = ui_viewnodes_on_node_changed;
	gc_root(ui_nodes_on_node_changed);
	gc_unroot(ui_nodes_on_socket_released);
	ui_nodes_on_socket_released = ui_viewnodes_on_socket_released;
	gc_root(ui_nodes_on_socket_released);
	gc_unroot(ui_nodes_on_canvas_released);
	ui_nodes_on_canvas_released = ui_viewnodes_on_canvas_released;
	gc_root(ui_nodes_on_canvas_released);
	gc_unroot(ui_nodes_on_canvas_control);
	ui_nodes_on_canvas_control = ui_viewnodes_on_canvas_control;
	gc_root(ui_nodes_on_canvas_control);
	ui_nodes_grid_snap = config_raw->grid_snap;
	nodes_material_init();
}

void ui_viewnodes_on_node_remove(ui_node_t *n) {
	gpu_texture_t *img = any_imap_get(context_raw->node_preview_map, n->id);
	if (img != NULL) {
		gpu_delete_texture(img);
		imap_delete(context_raw->node_preview_map, n->id);
	}
}

void ui_viewnodes_on_node_changed(ui_node_t *n) {
	gc_unroot(ui_nodes_node_changed);
	ui_nodes_node_changed = n;
	gc_root(ui_nodes_node_changed);
}

void ui_viewnodes_on_link_drag_on_node_search_done() {
	ui_nodes_t *ui_nodes = ui_nodes_get_nodes();

	i32 node_selected_id = _ui_nodes_on_link_drag_node->id;
	if (ui_nodes->nodes_selected_id->length > 0) {
		node_selected_id = ui_nodes->nodes_selected_id->buffer[0];
	}

	ui_node_link_t    *link_drag = _ui_nodes_on_link_drag_link_drag;
	ui_node_t_array_t *nodes     = ui_nodes_get_canvas(true)->nodes;
	ui_node_t         *n         = ui_get_node(nodes, node_selected_id);
	if (link_drag->to_id == -1 && n->inputs->length > 0) {
		link_drag->to_id = n->id;
		char *from_type  = _ui_nodes_on_link_drag_node->outputs->buffer[link_drag->from_socket]->type;
		// Connect to the first socket
		link_drag->to_socket = 0;
		// Try to find the first type-matching socket and use it if present
		for (i32 i = 0; i < n->inputs->length; ++i) {
			ui_node_socket_t *socket = n->inputs->buffer[i];
			if (string_equals(socket->type, from_type)) {
				link_drag->to_socket = array_index_of(n->inputs, socket);
				break;
			}
		}
		any_array_push(ui_nodes_get_canvas(true)->links, link_drag);
	}
	else if (link_drag->from_id == -1 && n->outputs->length > 0) {
		link_drag->from_id     = n->id;
		link_drag->from_socket = 0;
		any_array_push(ui_nodes_get_canvas(true)->links, link_drag);
	}
}

void ui_viewnodes_on_link_drag(i32 link_drag_id, bool is_new_link) {
	if (!is_new_link) {
		return;
	}

	ui_node_link_t_array_t *links     = ui_nodes_get_canvas(true)->links;
	ui_node_link_t         *link_drag = ui_get_link(links, link_drag_id);
	ui_node_t_array_t      *nodes     = ui_nodes_get_canvas(true)->nodes;
	ui_node_t              *node      = ui_get_node(nodes, link_drag->from_id > -1 ? link_drag->from_id : link_drag->to_id);
	i32                     link_x    = ui->_window_x + UI_NODE_X(node);
	i32                     link_y    = ui->_window_y + UI_NODE_Y(node);
	if (link_drag->from_id > -1) {
		link_x += UI_NODE_W(node);
		link_y += UI_OUTPUT_Y(node, link_drag->from_socket);
	}
	else {
		link_y += ui_nodes_INPUT_Y(ui_nodes_get_canvas(true), node, link_drag->to_socket) + UI_OUTPUTS_H(node, -1) + UI_BUTTONS_H(node);
	}

	if (math_abs(mouse_x - link_x) > 5 || math_abs(mouse_y - link_y) > 5) { // Link length

		gc_unroot(_ui_nodes_on_link_drag_link_drag);
		_ui_nodes_on_link_drag_link_drag = link_drag;
		gc_root(_ui_nodes_on_link_drag_link_drag);
		gc_unroot(_ui_nodes_on_link_drag_node);
		_ui_nodes_on_link_drag_node = node;
		gc_root(_ui_nodes_on_link_drag_node);
		ui_nodes_node_search(-1, -1, &ui_viewnodes_on_link_drag_on_node_search_done);
	}
	// Selecting which node socket to preview
	else if (link_drag->from_id > -1) {
		i32_imap_set(context_raw->node_preview_socket_map, node->id, link_drag->from_socket);
		gc_unroot(ui_nodes_node_changed);
		ui_nodes_node_changed = node;
		gc_root(ui_nodes_node_changed);
	}
}

void ui_viewnodes_on_socket_released_group_edit_box() {
	ui_node_socket_t *socket     = _ui_nodes_on_socket_released_socket;
	ui_node_t        *node       = _ui_nodes_on_socket_released_node;
	string_t_array_t *type_combo = any_array_create_from_raw(
	    (void *[]){
	        tr("Color"),
	        tr("Vector"),
	        tr("Value"),
	    },
	    3);
	i32 type = ui_combo(_ui_nodes_htype, type_combo, tr("Type"), true, UI_ALIGN_LEFT, true);
	if (_ui_nodes_htype->changed) {
		_ui_nodes_hname->text = type == 0 ? tr("Color") : type == 1 ? tr("Vector") : tr("Value");
	}
	char        *name          = ui_text_input(_ui_nodes_hname, tr("Name"), UI_ALIGN_LEFT, true, false);
	f32          min           = ui_float_input(_ui_nodes_hmin, tr("Min"), UI_ALIGN_LEFT, 1000.0);
	f32          max           = ui_float_input(_ui_nodes_hmax, tr("Max"), UI_ALIGN_LEFT, 1000.0);
	f32_array_t *default_value = NULL;
	if (type == 0) {
		ui_row4();
		ui_float_input(_ui_nodes_hval0, tr("R"), UI_ALIGN_LEFT, 1000.0);
		ui_float_input(_ui_nodes_hval1, tr("G"), UI_ALIGN_LEFT, 1000.0);
		ui_float_input(_ui_nodes_hval2, tr("B"), UI_ALIGN_LEFT, 1000.0);
		ui_float_input(_ui_nodes_hval3, tr("A"), UI_ALIGN_LEFT, 1000.0);
		default_value = f32_array_create_xyzw(_ui_nodes_hval0->f, _ui_nodes_hval1->f, _ui_nodes_hval2->f, _ui_nodes_hval3->f);
	}
	else if (type == 1) {
		ui_row3();
		_ui_nodes_hval0->f = ui_float_input(_ui_nodes_hval0, tr("X"), UI_ALIGN_LEFT, 1000.0);
		_ui_nodes_hval1->f = ui_float_input(_ui_nodes_hval1, tr("Y"), UI_ALIGN_LEFT, 1000.0);
		_ui_nodes_hval2->f = ui_float_input(_ui_nodes_hval2, tr("Z"), UI_ALIGN_LEFT, 1000.0);
		default_value      = f32_array_create_xyz(_ui_nodes_hval0->f, _ui_nodes_hval1->f, _ui_nodes_hval2->f);
	}
	else {
		f32 f         = ui_float_input(_ui_nodes_hval0, tr("default_value"), UI_ALIGN_LEFT, 1000.0);
		default_value = f32_array_create_x(f);
	}
	if (ui_icon_button(tr("OK"), ICON_CHECK, UI_ALIGN_CENTER)) { // || ui.isReturnDown
		socket->name          = string_copy(name);
		socket->type          = type == 0 ? "RGBA" : type == 1 ? "VECTOR" : "VALUE";
		socket->color         = nodes_material_get_socket_color(socket->type);
		socket->min           = min;
		socket->max           = max;
		socket->default_value = default_value;
		ui_box_hide();
		nodes_material_sync_sockets(node);
		ui_nodes_hwnd->redraws = 2;
	}
}

void ui_viewnodes_on_socket_released_group_edit(void *_) {
	ui_end_input();
	ui_box_show_custom(&ui_viewnodes_on_socket_released_group_edit_box, 400, 250, NULL, true, tr("Socket"));
}

void ui_viewnodes_on_socket_released_group_menu_draw() {
	ui_node_socket_t *socket = _ui_nodes_on_socket_released_socket;
	ui_node_t        *node   = _ui_nodes_on_socket_released_node;
	if (ui_menu_button(tr("Edit"), "", ICON_EDIT)) {
		_ui_nodes_htype->i    = string_equals(socket->type, "RGBA") ? 0 : string_equals(socket->type, "VECTOR") ? 1 : 2;
		_ui_nodes_hname->text = string_copy(socket->name);
		_ui_nodes_hmin->f     = socket->min;
		_ui_nodes_hmax->f     = socket->max;
		if (string_equals(socket->type, "RGBA") || string_equals(socket->type, "VECTOR")) {
			_ui_nodes_hval0->f = socket->default_value->buffer[0];
			_ui_nodes_hval1->f = socket->default_value->buffer[1];
			_ui_nodes_hval2->f = socket->default_value->buffer[2];
			if (string_equals(socket->type, "RGBA")) {
				_ui_nodes_hval3->f = socket->default_value->buffer[3];
			}
		}
		else {
			_ui_nodes_hval0->f = socket->default_value->buffer[0];
		}
		sys_notify_on_next_frame(&ui_viewnodes_on_socket_released_group_edit, NULL);
	}
	if (ui_menu_button(tr("Delete"), "", ICON_DELETE)) {
		i32               i      = 0;
		ui_node_canvas_t *canvas = ui_nodes_get_canvas(true);
		// Remove links connected to the socket
		while (i < canvas->links->length) {
			ui_node_link_t *l = canvas->links->buffer[i];
			if ((l->from_id == node->id && l->from_socket == array_index_of(node->outputs, socket)) ||
			    (l->to_id == node->id && l->to_socket == array_index_of(node->inputs, socket))) {
				array_splice(canvas->links, i, 1);
			}
			else {
				i++;
			}
		}
		// Remove socket
		array_remove(node->inputs, socket);
		array_remove(node->outputs, socket);
		nodes_material_sync_sockets(node);
	}
}

void ui_viewnodes_on_socket_released_group_menu(void *_) {
	ui_menu_draw(&ui_viewnodes_on_socket_released_group_menu_draw, -1, -1);
}

void ui_viewnodes_on_socket_released(i32 socket_id) {
	ui_node_canvas_t *canvas = ui_nodes_get_canvas(true);
	ui_node_socket_t *socket = ui_get_socket(canvas->nodes, socket_id);
	ui_node_t        *node   = ui_get_node(canvas->nodes, socket->node_id);
	if (ui->input_released_r) {
		if (string_equals(node->type, "GROUP_INPUT") || string_equals(node->type, "GROUP_OUTPUT")) {

			gc_unroot(_ui_nodes_on_socket_released_socket);
			_ui_nodes_on_socket_released_socket = socket;
			gc_root(_ui_nodes_on_socket_released_socket);
			gc_unroot(_ui_nodes_on_socket_released_node);
			_ui_nodes_on_socket_released_node = node;
			gc_root(_ui_nodes_on_socket_released_node);
			sys_notify_on_next_frame(&ui_viewnodes_on_socket_released_group_menu, NULL);
		}
		else {
			ui_viewnodes_on_canvas_released();
		}
	}
	// Selecting which node socket to preview
	// else {
	// 	let i: i32 = array_index_of(node.outputs, socket);
	// 	if (i > -1) {
	// 		i32_imap_set(context_raw.node_preview_socket_map, node.id, i);
	// 		ui_nodes_node_changed = node;
	// 	}
	// }
}

void ui_viewnodes_on_canvas_capture_output(void *_) {
	ui_nodes_capture_output();
}

void ui_viewnodes_on_canvas_released_duplicate(void *_) {
	ui_nodes_hwnd->redraws   = 2;
	ui_is_copy               = true;
	ui_is_paste              = true;
	ui_nodes_is_node_menu_op = true;
}

void ui_viewnodes_on_canvas_delete_on_next_frame(void *_) {
	ui_nodes_hwnd->redraws   = 2;
	ui->is_delete_down       = true;
	ui_nodes_is_node_menu_op = true;
}

void ui_viewnodes_on_canvas_delete(void *_) {
	sys_notify_on_end_frame(&ui_viewnodes_on_canvas_delete_on_next_frame, NULL);
}

void ui_viewnodes_on_canvas_paste(void *_) {
	ui_nodes_hwnd->redraws   = 2;
	ui_is_paste              = true;
	ui_nodes_is_node_menu_op = true;
}

void ui_viewnodes_on_canvas_copy(void *_) {
	ui_is_copy               = true;
	ui_nodes_is_node_menu_op = true;
}

void ui_viewnodes_on_canvas_cut(void *_) {
	ui_nodes_hwnd->redraws   = 2;
	ui_is_copy               = true;
	ui_is_cut                = true;
	ui_nodes_is_node_menu_op = true;
}

void ui_viewnodes_on_canvas_context_menu() {
	ui_node_t *selected = _ui_nodes_on_canvas_released_selected;
	ui->_y += 1;
	bool is_protected = selected == NULL || string_equals(selected->type, "OUTPUT_MATERIAL_PBR") || string_equals(selected->type, "GROUP_INPUT") ||
	                    string_equals(selected->type, "GROUP_OUTPUT") || string_equals(selected->type, "brush_output_node");
	ui->enabled = !is_protected;
	if (ui_menu_button(tr("Cut"), "ctrl+x", ICON_CUT)) {
		sys_notify_on_next_frame(&ui_viewnodes_on_canvas_cut, NULL);
	}
	if (ui_menu_button(tr("Copy"), "ctrl+c", ICON_COPY)) {
		sys_notify_on_next_frame(&ui_viewnodes_on_canvas_copy, NULL);
	}
	ui->enabled = !string_equals(ui_clipboard, "");
	if (ui_menu_button(tr("Paste"), "ctrl+v", ICON_PASTE)) {
		sys_notify_on_next_frame(&ui_viewnodes_on_canvas_paste, NULL);
	}
	ui->enabled = !is_protected;
	if (ui_menu_button(tr("Delete"), "delete", ICON_DELETE)) {
		sys_notify_on_next_frame(&ui_viewnodes_on_canvas_delete, NULL);
	}
	if (ui_menu_button(tr("Duplicate"), "", ICON_DUPLICATE)) {
		sys_notify_on_next_frame(&ui_viewnodes_on_canvas_released_duplicate, NULL);
	}
	if (selected != NULL && string_equals(selected->type, "RGB")) {
		if (ui_menu_button(tr("Add Swatch"), "", ICON_PALETTE)) {
			f32_array_t    *color      = selected->outputs->buffer[0]->default_value;
			swatch_color_t *new_swatch = project_make_swatch(color_from_floats(color->buffer[0], color->buffer[1], color->buffer[2], color->buffer[3]));
			context_set_swatch(new_swatch);
			any_array_push(project_raw->swatches, new_swatch);
			ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 1;
		}
	}
	if (ui_menu_button(tr("Capture Output"), "", ICON_PHOTO)) {
		sys_notify_on_next_frame(&ui_viewnodes_on_canvas_capture_output, NULL);
	}
	if (ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL) {
		ui_menu_separator();
		if (ui_menu_button(tr("2D View"), "", ICON_NONE)) {
			ui_base_show_2d_view(VIEW_2D_TYPE_NODE);
		}
	}
	ui->enabled = true;
}

void ui_viewnodes_on_canvas_released() {
	if (ui->input_released_r && context_in_nodes() && math_abs(ui->input_x - ui->input_started_x) < 2 && math_abs(ui->input_y - ui->input_started_y) < 2) {

		// Node selection
		ui_nodes_t       *nodes    = ui_nodes_get_nodes();
		ui_node_canvas_t *canvas   = ui_nodes_get_canvas(true);
		ui_node_t        *selected = NULL;
		for (i32 i = 0; i < canvas->nodes->length; ++i) {
			ui_node_t *node = canvas->nodes->buffer[i];
			if (ui_input_in_rect(ui->_window_x + UI_NODE_X(node), ui->_window_y + UI_NODE_Y(node), UI_NODE_W(node), UI_NODE_H(canvas, node))) {
				selected = node;
				break;
			}
		}
		if (selected == NULL) {
			nodes->nodes_selected_id = i32_array_create_from_raw((i32[]){}, 0);
		}
		else if (i32_array_index_of(nodes->nodes_selected_id, selected->id) == -1) {
			nodes->nodes_selected_id = i32_array_create_from_raw(
			    (i32[]){
			        selected->id,
			    },
			    1);
		}

		// Node context menu
		if (!ui_nodes_socket_released) {

			gc_unroot(_ui_nodes_on_canvas_released_selected);
			_ui_nodes_on_canvas_released_selected = selected;
			gc_root(_ui_nodes_on_canvas_released_selected);
			ui_menu_draw(&ui_viewnodes_on_canvas_context_menu, -1, -1);
		}
	}

	if (ui->input_released) {
		ui_nodes_t       *nodes  = ui_nodes_get_nodes();
		ui_node_canvas_t *canvas = ui_nodes_get_canvas(true);
		for (i32 i = 0; i < canvas->nodes->length; ++i) {
			ui_node_t *node = canvas->nodes->buffer[i];
			if (ui_input_in_rect(ui->_window_x + UI_NODE_X(node), ui->_window_y + UI_NODE_Y(node), UI_NODE_W(node), UI_NODE_H(canvas, node))) {
				if (nodes->nodes_selected_id->length > 0 && node->id == nodes->nodes_selected_id->buffer[0]) {
					ui_view2d_hwnd->redraws = 2;
					if (sys_time() - context_raw->select_time < 0.2)
						ui_base_show_2d_view(VIEW_2D_TYPE_NODE);
					context_raw->select_time = sys_time();
				}
				break;
			}
		}
	}
}

ui_canvas_control_t *ui_viewnodes_on_canvas_control() {
	ui_canvas_control_t *control = ui_nodes_get_canvas_control(ui_nodes_controls_down);
	ui_nodes_controls_down       = control->controls_down;
	return control;
}

ui_canvas_control_t *ui_nodes_get_canvas_control(bool controls_down) {
	if (config_raw->wrap_mouse && controls_down) {
		if (ui->input_x < ui->_window_x) {
			ui->input_x = ui->_window_x + ui->_window_w;
			iron_mouse_set_position(math_floor(ui->input_x), math_floor(ui->input_y));
		}
		else if (ui->input_x > ui->_window_x + ui->_window_w) {
			ui->input_x = ui->_window_x;
			iron_mouse_set_position(math_floor(ui->input_x), math_floor(ui->input_y));
		}
		else if (ui->input_y < ui->_window_y) {
			ui->input_y = ui->_window_y + ui->_window_h;
			iron_mouse_set_position(math_floor(ui->input_x), math_floor(ui->input_y));
		}
		else if (ui->input_y > ui->_window_y + ui->_window_h) {
			ui->input_y = ui->_window_y;
			iron_mouse_set_position(math_floor(ui->input_x), math_floor(ui->input_y));
		}
	}
	if (operator_shortcut(any_map_get(config_keymap, "action_pan"), SHORTCUT_TYPE_STARTED) ||
	    operator_shortcut(any_map_get(config_keymap, "action_zoom"), SHORTCUT_TYPE_STARTED) || ui->input_started_r || ui->input_wheel_delta != 0.0) {
		controls_down = true;
	}
	else if (!operator_shortcut(any_map_get(config_keymap, "action_pan"), SHORTCUT_TYPE_DOWN) &&
	         !operator_shortcut(any_map_get(config_keymap, "action_zoom"), SHORTCUT_TYPE_DOWN) && !ui->input_down_r && ui->input_wheel_delta == 0.0) {
		controls_down = false;
	}
	if (!controls_down) {
		ui_canvas_control_t *cc = GC_ALLOC_INIT(ui_canvas_control_t, {.pan_x = 0, .pan_y = 0, .zoom = 0, .controls_down = controls_down});
		return cc;
	}

	bool pan        = ui->input_down_r || operator_shortcut(any_map_get(config_keymap, "action_pan"), SHORTCUT_TYPE_DOWN);
	f32  zoom_delta = operator_shortcut(any_map_get(config_keymap, "action_zoom"), SHORTCUT_TYPE_DOWN) ? ui_nodes_get_zoom_delta() / (float)100.0 : 0.0;
	ui_canvas_control_t *control = GC_ALLOC_INIT(ui_canvas_control_t, {.pan_x         = pan ? ui->input_dx : 0.0,
	                                                                   .pan_y         = pan ? ui->input_dy : 0.0,
	                                                                   .zoom          = ui->input_wheel_delta != 0.0 ? -ui->input_wheel_delta / 10 : zoom_delta,
	                                                                   .controls_down = controls_down});
	if (ui->combo_selected_handle != NULL) {
		control->zoom = 0.0;
	}
	if (control->zoom != 0.0) {
		ui_nodes_grid_redraw = true;
	}

	return control;
}

f32 ui_nodes_get_zoom_delta() {
	return config_raw->zoom_direction == ZOOM_DIRECTION_VERTICAL              ? -ui->input_dy
	       : config_raw->zoom_direction == ZOOM_DIRECTION_VERTICAL_INVERTED   ? -ui->input_dy
	       : config_raw->zoom_direction == ZOOM_DIRECTION_HORIZONTAL          ? ui->input_dx
	       : config_raw->zoom_direction == ZOOM_DIRECTION_HORIZONTAL_INVERTED ? ui->input_dx
	                                                                          : -(ui->input_dy - ui->input_dx);
}

bool ui_nodes_is_tab_selected() {
	return ui_nodes_htab->i > 0 && ui_nodes_htab->i % 2 == 1 && // [tab0, tab1, x, tab2, x, +]
	       ui_nodes_tabs->length >= ui_nodes_htab->i / (float)2;
}

i32 ui_nodes_tab_index() {
	return (int)(ui_nodes_htab->i / (float)2);
}

ui_node_canvas_t *ui_nodes_get_canvas(bool groups) {
	if (ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL) {
		if (groups && ui_nodes_group_stack->length > 0) {
			return ui_nodes_group_stack->buffer[ui_nodes_group_stack->length - 1]->canvas;
		}
		else if (ui_nodes_is_tab_selected()) {
			return ui_nodes_tabs->buffer[ui_nodes_tab_index()]->canvas;
		}
		else {
			return context_raw->material->canvas;
		}
	}
	else {
		return context_raw->brush->canvas;
	}
}

ui_nodes_t *ui_nodes_get_nodes() {
	if (ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL) {
		if (ui_nodes_group_stack->length > 0) {
			return ui_nodes_group_stack->buffer[ui_nodes_group_stack->length - 1]->nodes;
		}
		else if (ui_nodes_is_tab_selected()) {
			return ui_nodes_tabs->buffer[ui_nodes_tab_index()]->nodes;
		}
		else {
			return context_raw->material->nodes;
		}
	}
	else {
		return context_raw->brush->nodes;
	}
}

void ui_nodes_update(void *_) {
	if (!ui_nodes_show || !base_ui_enabled) {
		return;
	}

	ui_nodes_wx = math_floor(sys_w()) + ui_toolbar_w(true);
	ui_nodes_wy = ui_header_h * 2;

	if (ui_view2d_show) {
		ui_nodes_wy += sys_h() - config_raw->layout->buffer[LAYOUT_SIZE_NODES_H];
	}

	i32 ww = config_raw->layout->buffer[LAYOUT_SIZE_NODES_W];
	if (!ui_base_show) {
		ww += config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] + ui_toolbar_w(true);
		ui_nodes_wx -= ui_toolbar_w(true);
		ui_nodes_wy = 0;
	}
	if (!base_view3d_show) {
		ww += base_view3d_w();
	}

	i32 mx = mouse_x;
	i32 my = mouse_y;
	if (mx < ui_nodes_wx || mx > ui_nodes_wx + ww || my < ui_nodes_wy) {
		return;
	}
	if (ui->is_typing || !ui->input_enabled) {
		return;
	}

	ui_nodes_t *nodes = ui_nodes_get_nodes();
	if (nodes->nodes_selected_id->length > 0 && ui->is_key_pressed) {
		if (ui->key_code == KEY_CODE_LEFT) {
			for (i32 i = 0; i < nodes->nodes_selected_id->length; ++i) {
				i32                n      = nodes->nodes_selected_id->buffer[i];
				ui_node_t_array_t *_nodes = ui_nodes_get_canvas(true)->nodes;
				ui_get_node(_nodes, n)->x -= 1;
			}
		}
		else if (ui->key_code == KEY_CODE_RIGHT) {
			for (i32 i = 0; i < nodes->nodes_selected_id->length; ++i) {
				i32                n      = nodes->nodes_selected_id->buffer[i];
				ui_node_t_array_t *_nodes = ui_nodes_get_canvas(true)->nodes;
				ui_get_node(_nodes, n)->x += 1;
			}
		}
		if (ui->key_code == KEY_CODE_UP) {
			for (i32 i = 0; i < nodes->nodes_selected_id->length; ++i) {
				i32                n      = nodes->nodes_selected_id->buffer[i];
				ui_node_t_array_t *_nodes = ui_nodes_get_canvas(true)->nodes;
				ui_get_node(_nodes, n)->y -= 1;
			}
		}
		else if (ui->key_code == KEY_CODE_DOWN) {
			for (i32 i = 0; i < nodes->nodes_selected_id->length; ++i) {
				i32                n      = nodes->nodes_selected_id->buffer[i];
				ui_node_t_array_t *_nodes = ui_nodes_get_canvas(true)->nodes;
				ui_get_node(_nodes, n)->y += 1;
			}
		}
	}

	// Node search popup
	if (operator_shortcut(any_map_get(config_keymap, "node_search"), SHORTCUT_TYPE_STARTED)) {
		ui_nodes_node_search(-1, -1, NULL);
	}
	if (ui_nodes_node_search_spawn != NULL) {
		ui->input_x = mouse_x; // Fix inputDX after popup removal
		ui->input_y = mouse_y;
		gc_unroot(ui_nodes_node_search_spawn);
		ui_nodes_node_search_spawn = NULL;
	}

	if (operator_shortcut(any_map_get(config_keymap, "view_reset"), SHORTCUT_TYPE_STARTED)) {
		nodes->pan_x = 0.0;
		nodes->pan_y = 0.0;
		nodes->zoom  = 1.0;
	}

	if (operator_shortcut(any_map_get(config_keymap, "node_overview"), SHORTCUT_TYPE_STARTED)) {
		nodes->zoom = nodes->zoom == 1.0 ? 0.2 : 1.0;
		nodes->uiw  = ui_nodes_ww;
		nodes->uih  = ui_nodes_wh;
	}
}

void ui_nodes_canvas_changed() {
	ui_nodes_recompile_mat       = true;
	ui_nodes_recompile_mat_final = true;
}

void ui_nodes_node_search_menu() {
	ui_menu_h                  = UI_ELEMENT_H() * 8;
	ui_handle_t *search_handle = ui_handle(__ID__);
	char        *search        = to_lower_case(ui_text_input(search_handle, "", UI_ALIGN_LEFT, true, true));
	ui->changed                = false;
	if (_ui_nodes_node_search_first) {
		_ui_nodes_node_search_first = false;
		search_handle->text         = "";
		ui_start_text_edit(search_handle, UI_ALIGN_LEFT); // Focus search bar
	}

	if (search_handle->changed) {
		ui_nodes_node_search_offset = 0;
	}

	if (ui->is_key_pressed) { // Move selection
		if (ui->key_code == KEY_CODE_DOWN && ui_nodes_node_search_offset < 6) {
			ui_nodes_node_search_offset++;
		}
		if (ui->key_code == KEY_CODE_UP && ui_nodes_node_search_offset > 0) {
			ui_nodes_node_search_offset--;
		}
	}
	bool enter                     = keyboard_down("enter");
	i32  count                     = 0;
	i32  BUTTON_COL                = ui->ops->theme->BUTTON_COL;
	bool FILL_BUTTON_BG            = ui->ops->theme->FILL_BUTTON_BG;
	ui->ops->theme->FILL_BUTTON_BG = true;

	node_list_t_array_t *node_list = ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL ? nodes_material_list : nodes_brush_list;

	for (i32 i = 0; i < node_list->length; ++i) {
		ui_node_t_array_t *list = node_list->buffer[i];
		for (i32 i = 0; i < list->length; ++i) {
			ui_node_t *n = list->buffer[i];
			if (string_index_of(to_lower_case(tr(n->name)), search) >= 0) {
				ui->ops->theme->BUTTON_COL = count == ui_nodes_node_search_offset ? ui->ops->theme->HIGHLIGHT_COL : ui->ops->theme->SEPARATOR_COL;
				if (ui_button(tr(n->name), UI_ALIGN_LEFT, "") || (enter && count == ui_nodes_node_search_offset)) {
					ui_nodes_push_undo(NULL);
					ui_nodes_t       *nodes  = ui_nodes_get_nodes();
					ui_node_canvas_t *canvas = ui_nodes_get_canvas(true);
					gc_unroot(ui_nodes_node_search_spawn);
					ui_nodes_node_search_spawn = ui_nodes_make_node(n, nodes, canvas); // Spawn selected node
					gc_root(ui_nodes_node_search_spawn);
					any_array_push(canvas->nodes, ui_nodes_node_search_spawn);
					nodes->nodes_selected_id = i32_array_create_from_raw(
					    (i32[]){
					        ui_nodes_node_search_spawn->id,
					    },
					    1);
					nodes->nodes_drag = true;

					ui_nodes_hwnd->redraws = 2;
					if (enter) {
						ui->changed = true;
						count       = 6; // Trigger break
					}
					if (_ui_nodes_node_search_done != NULL) {
						_ui_nodes_node_search_done();
					}
				}
				if (++count > 6) {
					break;
				}
			}
		}
		if (count > 6) {
			break;
		}
	}
	if (enter && count == 0) { // Hide popup on enter when node is not found
		ui->changed         = true;
		search_handle->text = "";
	}
	ui->ops->theme->BUTTON_COL     = BUTTON_COL;
	ui->ops->theme->FILL_BUTTON_BG = FILL_BUTTON_BG;
}

void ui_nodes_node_search(i32 x, i32 y, void (*done)(void)) {
	_ui_nodes_node_search_first = true;
	gc_unroot(_ui_nodes_node_search_done);
	_ui_nodes_node_search_done = done;
	gc_root(_ui_nodes_node_search_done);
	ui_menu_draw(&ui_nodes_node_search_menu, x, y);
}

i32 ui_nodes_get_node_x() {
	return math_floor((mouse_x - ui_nodes_wx - UI_NODES_PAN_X()) / (float)UI_NODES_SCALE());
}

i32 ui_nodes_get_node_y() {
	return math_floor((mouse_y - ui_nodes_wy - UI_NODES_PAN_Y()) / (float)UI_NODES_SCALE());
}

gpu_texture_t *ui_nodes_draw_grid(f32 zoom) {
	i32 ww = config_raw->layout->buffer[LAYOUT_SIZE_NODES_W];
	if (!ui_base_show) {
		ww += config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] + ui_toolbar_w(true);
	}
	if (!base_view3d_show) {
		ww += base_view3d_w();
	}
	i32 wh   = sys_h();
	f32 step = ui_nodes_grid_cell_w * zoom;
	i32 mult = 5 * UI_SCALE();
	i32 w    = math_floor(ww + step * mult);
	i32 h    = math_floor(wh + step * mult);
	if (w < 1) {
		w = 1;
	}
	if (h < 1) {
		h = 1;
	}
	gpu_texture_t *grid = gpu_create_render_target(w, h, GPU_TEXTURE_FORMAT_RGBA32);
	draw_begin(grid, true, ui->ops->theme->SEPARATOR_COL);

	i32 sep_col      = ui->ops->theme->SEPARATOR_COL;
	i32 line_primary = sep_col - 0x00050505;
	if (line_primary < 0xff000000) {
		line_primary = sep_col + 0x00050505;
	}

	i32 line_secondary = sep_col - 0x00090909;
	if (line_secondary < 0xff000000) {
		line_secondary = sep_col + 0x00090909;
	}

	draw_set_color(line_primary);
	step = ui_nodes_grid_small_cell_w * zoom;
	for (i32 i = 0; i < math_floor(h / (float)step) + 1; ++i) {
		draw_line(0, i * step, w, i * step, 1.0);
	}
	for (i32 i = 0; i < math_floor(w / (float)step) + 1; ++i) {
		draw_line(i * step, 0, i * step, h, 1.0);
	}
	draw_set_color(line_secondary);
	step = ui_nodes_grid_cell_w * zoom;
	for (i32 i = 0; i < math_floor(h / (float)step) + 1; ++i) {
		draw_line(0, i * step, w, i * step, 1.0);
	}
	for (i32 i = 0; i < math_floor(w / (float)step) + 1; ++i) {
		draw_line(i * step, 0, i * step, h, 1.0);
	}
	draw_end();
	return grid;
}

void ui_nodes_recompile() {
	if (ui_nodes_recompile_mat) {
		if (ui_nodes_canvas_type == CANVAS_TYPE_BRUSH) {
			make_material_parse_brush();
			util_render_make_brush_preview();
			ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
		}
		else {
			slot_material_t *_material = context_raw->material;
			if (ui_nodes_is_tab_selected()) {
				context_raw->material = ui_nodes_tabs->buffer[ui_nodes_tab_index()];
			}
			layers_is_fill_material() ? layers_update_fill_layers() : util_render_make_material_preview();
			context_raw->material = _material;

			if (ui_view2d_show && ui_view2d_type == VIEW_2D_TYPE_NODE) {
				ui_view2d_hwnd->redraws = 2;
			}
		}

		ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
		if (context_raw->split_view) {
			context_raw->ddirty = 2;
		}

		ui_nodes_recompile_mat = false;
	}
	else if (ui_nodes_recompile_mat_final) {
		make_material_parse_paint_material(true);

		if (ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL && layers_is_fill_material()) {
			layers_update_fill_layers();
			util_render_make_material_preview();
		}

		bool decal = context_is_decal();
		if (decal) {
			util_render_make_decal_preview();
		}

		ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
		ui_nodes_recompile_mat_final                      = false;
	}
}

void ui_nodes_get_linked_nodes(ui_node_t_array_t *linked_nodes, ui_node_t *n, ui_node_canvas_t *c) {
	if (array_index_of(linked_nodes, n) == -1 && (n->flags & UI_NODE_FLAG_PREVIEW)) {
		any_array_push(linked_nodes, n);
	}
	for (i32 i = 0; i < c->links->length; ++i) {
		ui_node_link_t *l = c->links->buffer[i];
		if (l->from_id == n->id) {
			ui_node_t *nn = ui_get_node(c->nodes, l->to_id);
			ui_nodes_get_linked_nodes(linked_nodes, nn, c);
		}
	}
}

void ui_nodes_render_color_picker_callback(swatch_color_t *color) {
	_ui_nodes_render_tmp(color->base);
	ui_nodes_hwnd->redraws = 2;
	bool material_live     = config_raw->material_live;
	if (material_live) {
		ui_nodes_canvas_changed();
	}
}

void ui_nodes_render(void *_) {
	ui_nodes_recompile();

	ui_nodes_t *ui_nodes = ui_nodes_get_nodes();

	// Remove dragged link when mouse is released out of the node viewport
	ui_node_canvas_t *c = ui_nodes_get_canvas(true);
	if (ui_nodes_release_link && ui_nodes->link_drag_id != -1) {
		array_remove(c->links, ui_get_link(c->links, ui_nodes->link_drag_id));
		ui_nodes->link_drag_id = -1;
	}
	ui_nodes_release_link = ui->input_released;

	if (!ui_nodes_show) {
		return;
	}

	ui->input_enabled = base_ui_enabled;

	if (ui_nodes_last_zoom != ui_nodes->zoom) {
		ui_nodes_last_zoom   = ui_nodes->zoom;
		ui_nodes_grid_redraw = true;
	}

	if (ui_nodes_grid_redraw) {
		if (ui_nodes_grid != NULL) {
			gpu_delete_texture(ui_nodes_grid);
		}
		gc_unroot(ui_nodes_grid);
		ui_nodes_grid = ui_nodes_draw_grid(ui_nodes->zoom);
		gc_root(ui_nodes_grid);
		ui_nodes_grid_redraw = false;
	}

	// Selected node preview
	if (ui_nodes->nodes_selected_id->length > 0 && ui_nodes->nodes_selected_id->buffer[0] != ui_nodes_last_node_selected_id) {
		ui_nodes_last_node_selected_id = ui_nodes->nodes_selected_id->buffer[0];
		ui_node_t *sel                 = ui_get_node(c->nodes, ui_nodes->nodes_selected_id->buffer[0]);
		ui_nodes_make_node_preview(sel);
	}
	else if (ui_nodes->nodes_selected_id->length == 0) {
		ui_nodes_last_node_selected_id = -1;
	}

	// Update node previews
	if (ui_nodes_node_changed != NULL && !ui->input_down) {
		ui_node_t_array_t *linked_nodes = any_array_create_from_raw((void *[]){}, 0);
		ui_nodes_get_linked_nodes(linked_nodes, ui_nodes_node_changed, c);

		ui_node_t *sel = ui_nodes->nodes_selected_id->length > 0 ? ui_get_node(c->nodes, ui_nodes->nodes_selected_id->buffer[0]) : NULL;
		if (sel != NULL && array_index_of(linked_nodes, sel) == -1) {
			any_array_push(linked_nodes, sel);
		}

		for (i32 i = 0; i < linked_nodes->length; ++i) {
			ui_nodes_make_node_preview(linked_nodes->buffer[i]);
		}
		gc_unroot(ui_nodes_node_changed);
		ui_nodes_node_changed = NULL;
	}

	// Start with UI
	ui_begin(ui);

	// Make window
	ui_nodes_ww = config_raw->layout->buffer[LAYOUT_SIZE_NODES_W];
	ui_nodes_wx = math_floor(sys_w()) + ui_toolbar_w(true);
	ui_nodes_wy = 0;

	if (!ui_base_show) {
		ui_nodes_ww += config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] + ui_toolbar_w(true);
		ui_nodes_wx -= ui_toolbar_w(true);
	}
	if (!base_view3d_show) {
		ui_nodes_ww += base_view3d_w();
	}

	i32 ew      = math_floor(UI_ELEMENT_W() * 0.7);
	ui_nodes_wh = sys_h();
	if (config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 1) {
		ui_nodes_wh += ui_header_h * 2;
	}

	if (ui_view2d_show) {
		ui_nodes_wh = config_raw->layout->buffer[LAYOUT_SIZE_NODES_H];
		ui_nodes_wy = sys_h() - config_raw->layout->buffer[LAYOUT_SIZE_NODES_H] + ui_header_h;
		if (config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 1) {
			ui_nodes_wy += ui_header_h;
		}
		else {
			ui_nodes_wy -= ui_header_h;
		}
		if (!ui_base_show) {
			ui_nodes_wy -= ui_header_h * 2;
		}
	}

	if (!base_view3d_show && ui_view2d_show) {
		ui_nodes_wx = base_view3d_w();
		ui_nodes_wy = 0;
		ui_nodes_ww = config_raw->layout->buffer[LAYOUT_SIZE_NODES_W];
		ui_nodes_wh = sys_h();
		if (config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 1) {
			ui_nodes_wh += ui_header_h * 2;
		}
	}

	if (ui_window(ui_nodes_hwnd, ui_nodes_wx, ui_nodes_wy, ui_nodes_ww, ui_nodes_wh, false)) {

		if (!config_raw->touch_ui) {
			bool expand = !base_view3d_show && config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] == 0;
			ui_tab(ui_nodes_htab, expand ? string("%s          ", tr("Nodes")) : tr("Nodes"), false, -1, !base_view3d_show);

			// Additional tabs
			if (ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL) {
				if (ui_nodes_tabs == NULL) {
					gc_unroot(ui_nodes_tabs);
					ui_nodes_tabs = any_array_create_from_raw((void *[]){}, 0);
					gc_root(ui_nodes_tabs);
				}

				for (i32 i = 0; i < ui_nodes_tabs->length; ++i) {
					ui_tab(ui_nodes_htab, ui_nodes_tabs->buffer[i]->canvas->name, false, -1, false);
					if (ui_tab(ui_nodes_htab, tr("x"), false, -1, false)) {
						array_splice(ui_nodes_tabs, i, 1);
						ui_nodes_htab->i = 0;
					}
				}

				if (ui_tab(ui_nodes_htab, tr("+"), false, -1, false)) {
					any_array_push(ui_nodes_tabs, context_raw->material);
				}
			}
		}

		// Grid
		draw_set_color(0xffffffff);
		f32 step = ui_nodes_grid_cell_w * ui_nodes->zoom;
		f32 x    = math_fmod(UI_NODES_PAN_X(), step) - step;
		f32 y    = math_fmod(UI_NODES_PAN_Y(), step) - step;
		draw_image(ui_nodes_grid, x, y);

		// Undo
		if (ui->input_started || ui->is_key_pressed) {
			gc_unroot(ui_nodes_last_canvas);
			ui_nodes_last_canvas = util_clone_canvas(ui_nodes_get_canvas(true));
			gc_root(ui_nodes_last_canvas);
		}

		// Nodes
		bool _input_enabled      = ui->input_enabled;
		i32  header_h            = UI_ELEMENT_H() * 2 + UI_ELEMENT_OFFSET() * 2;
		bool header_hover        = ui->input_y < ui->_window_y + header_h;
		ui->input_enabled        = _input_enabled && !ui_nodes_show_menu && !header_hover;
		ui->window_border_right  = config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W];
		ui->window_border_top    = ui_header_h * 2;
		ui->window_border_bottom = config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H];

		ui_node_canvas(ui_nodes, c);
		ui->input_enabled = _input_enabled;

		if (ui_nodes->color_picker_callback != NULL) {
			context_raw->color_picker_previous_tool = context_raw->tool;
			context_select_tool(TOOL_TYPE_PICKER);
			gc_unroot(_ui_nodes_render_tmp);
			_ui_nodes_render_tmp = ui_nodes->color_picker_callback;
			gc_root(_ui_nodes_render_tmp);
			context_raw->color_picker_callback = &ui_nodes_render_color_picker_callback;
			;
			ui_nodes->color_picker_callback = NULL;
		}

		// Remove nodes with unknown id for this canvas type
		if (ui_is_paste) {
			node_list_t_array_t *node_list = ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL ? nodes_material_list : nodes_brush_list;

			i32 i = 0;
			while (i++ < c->nodes->length) {
				ui_node_t *canvas_node = c->nodes->buffer[i - 1];
				if (string_array_index_of(ui_nodes_exclude_remove, canvas_node->type) >= 0) {
					continue;
				}
				bool found = false;
				for (i32 i = 0; i < node_list->length; ++i) {
					ui_node_t_array_t *list = node_list->buffer[i];
					for (i32 i = 0; i < list->length; ++i) {
						ui_node_t *list_node = list->buffer[i];
						if (string_equals(canvas_node->type, list_node->type)) {
							found = true;
							break;
						}
					}
					if (found) {
						break;
					}
				}
				if (string_equals(canvas_node->type, "GROUP") && !ui_nodes_can_place_group(canvas_node->name)) {
					found = false;
				}
				if (!found) {
					ui_remove_node(canvas_node, c);
					i32_array_remove(ui_nodes->nodes_selected_id, canvas_node->id);
					i--;
				}
			}
		}

		if (ui_nodes_is_node_menu_op) {
			ui_is_copy         = false;
			ui_is_cut          = false;
			ui_is_paste        = false;
			ui->is_delete_down = false;
		}

		// Recompile material on change
		if (ui->changed) {
			ui_nodes_recompile_mat = (ui->input_dx != 0 || ui->input_dy != 0 || !ui_nodes_uichanged_last) && config_raw->material_live; // Instant preview
		}
		else if (ui_nodes_uichanged_last) {
			ui_nodes_canvas_changed();
			ui_nodes_push_undo(ui_nodes_last_canvas);
		}
		ui_nodes_uichanged_last = ui->changed;

		// Node previews
		if (context_raw->selected_node_preview && ui_nodes->nodes_selected_id->length > 0) {
			ui_node_t     *sel = ui_get_node(c->nodes, ui_nodes->nodes_selected_id->buffer[0]);
			gpu_texture_t *img = ui_nodes_get_node_preview_image(sel);
			if (img != NULL && !(sel->flags & UI_NODE_FLAG_PREVIEW)) {
				f32 tw = 128 * UI_SCALE();
				f32 th = tw * (img->height / (float)img->width);
				f32 tx = ui_nodes_ww - tw - 8 * UI_SCALE();
				f32 ty = ui_nodes_wh - th - 8 * UI_SCALE();
				if (img == any_imap_get(context_raw->node_preview_map, sel->id)) {
					ui_draw_shadow(tx, ty, tw, th);
				}
				bool single_channel = string_equals(sel->type, "LAYER_MASK");
				if (single_channel) {
					draw_set_pipeline(ui_view2d_pipe);
					gpu_set_int(ui_view2d_channel_loc, 1);
				}
				draw_set_color(0xffffffff);
				draw_scaled_image(img, tx, ty, tw, th);
				if (single_channel) {
					draw_set_pipeline(NULL);
				}
			}
		}

		ui_nodes_draw_menubar();

		ui->window_border_right  = 0;
		ui->window_border_top    = 0;
		ui->window_border_bottom = 0;
	}

	ui_end();

	if (ui_nodes_show_menu) {
		node_list_t_array_t *list     = ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL ? nodes_material_list : nodes_brush_list;
		ui_node_t_array_t   *category = list->buffer[ui_nodes_menu_category];
		bool                 is_group_category =
		    ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL && string_equals(nodes_material_categories->buffer[ui_nodes_menu_category], "Group");
		i32 py    = ui_nodes_popup_y;
		i32 menuw = math_floor(ew * 2.3);
		draw_begin(NULL, false, 0);
		ui_begin_region(ui, math_floor(ui_nodes_popup_x), math_floor(py), menuw);
		i32 _FILL_BUTTON_BG            = ui->ops->theme->FILL_BUTTON_BG;
		ui->ops->theme->FILL_BUTTON_BG = false;
		i32 _ELEMENT_OFFSET            = ui->ops->theme->ELEMENT_OFFSET;
		ui->ops->theme->ELEMENT_OFFSET = 0;
		i32 _ELEMENT_H                 = ui->ops->theme->ELEMENT_H;
		ui->ops->theme->ELEMENT_H      = config_raw->touch_ui ? (28 + 2) : 28;
		ui_menu_h                      = category->length * UI_ELEMENT_H();
		if (is_group_category) {
			ui_menu_h += project_material_groups->length * UI_ELEMENT_H();
		}
		ui_menu_begin();
		for (i32 i = 0; i < category->length; ++i) {
			ui_node_t *n = category->buffer[i];
			if (ui_menu_button(tr(n->name), "", ICON_NONE)) {
				ui_nodes_push_undo(NULL);
				ui_node_canvas_t *canvas = ui_nodes_get_canvas(true);
				ui_nodes_t       *nodes  = ui_nodes_get_nodes();
				ui_node_t        *node   = ui_nodes_make_node(n, nodes, canvas);
				any_array_push(canvas->nodes, node);
				nodes->nodes_selected_id = i32_array_create_from_raw(
				    (i32[]){
				        node->id,
				    },
				    1);
				nodes->nodes_drag = true;
			}
			// Next column
			if (ui->_y - ui_nodes_wy + UI_ELEMENT_H() / (float)2 > ui_nodes_wh) {
				ui->_x += menuw;
				ui->_y = py;
			}
		}
		if (is_group_category) {
			for (i32 i = 0; i < project_material_groups->length; ++i) {
				node_group_t *g  = project_material_groups->buffer[i];
				ui->enabled      = ui_nodes_can_place_group(g->canvas->name);
				f32_array_t *row = f32_array_create_from_raw(
				    (f32[]){
				        5 / (float)6,
				        1 / (float)6,
				    },
				    2);
				ui_row(row);
				if (ui_button(string("%s%s", config_button_spacing, g->canvas->name), UI_ALIGN_LEFT, "")) {
					ui_nodes_push_undo(NULL);
					ui_node_canvas_t *canvas = ui_nodes_get_canvas(true);
					ui_nodes_t       *nodes  = ui_nodes_get_nodes();
					ui_node_t        *node   = ui_nodes_make_group_node(g->canvas, nodes, canvas);
					any_array_push(canvas->nodes, node);
					nodes->nodes_selected_id = i32_array_create_from_raw(
					    (i32[]){
					        node->id,
					    },
					    1);
					nodes->nodes_drag = true;
				}
				ui->enabled = !project_is_material_group_in_use(g);
				if (ui_button("x", UI_ALIGN_CENTER, "")) {
					history_delete_material_group(g);
					array_remove(project_material_groups, g);
				}
				ui->enabled = true;
			}
		}
		ui_nodes_hide_menu =
		    ui->combo_selected_handle == NULL && !ui_nodes_show_menu_first && (ui->changed || ui->input_released || ui->input_released_r || ui->is_escape_down);
		ui_nodes_show_menu_first       = false;
		ui->ops->theme->FILL_BUTTON_BG = _FILL_BUTTON_BG;
		ui->ops->theme->ELEMENT_OFFSET = _ELEMENT_OFFSET;
		ui->ops->theme->ELEMENT_H      = _ELEMENT_H;
		ui_menu_end();
		ui_end_region();
		draw_end();
	}

	if (ui_nodes_hide_menu) {
		ui_nodes_show_menu       = false;
		ui_nodes_show_menu_first = true;
	}

	ui->input_enabled = true;
}

gpu_texture_t *ui_nodes_get_node_preview_image(ui_node_t *n) {
	if (n == NULL) {
		return NULL;
	}
	gpu_texture_t *img = NULL;
	if (string_equals(n->type, "LAYER") || string_equals(n->type, "LAYER_MASK")) {
		i32 id = n->buttons->buffer[0]->default_value->buffer[0];
		if (id < project_layers->length) {
			img = project_layers->buffer[id]->texpaint_preview;
		}
	}
	else if (string_equals(n->type, "MATERIAL")) {
		i32 id = n->buttons->buffer[0]->default_value->buffer[0];
		if (id < project_materials->length) {
			img = project_materials->buffer[id]->image;
		}
	}
	else if (string_equals(n->type, "OUTPUT_MATERIAL_PBR")) {
		img = context_raw->material->image;
	}
	else if (string_equals(n->type, "brush_output_node")) {
		img = context_raw->brush->image;
	}
	else if (string_equals(n->type, "TEX_IMAGE") && parser_material_get_input_link(n->inputs->buffer[0]) == NULL) {
		i32 i = n->buttons->buffer[0]->default_value->buffer[0];
		if (i <= 9000) { // 9999 - Texture deleted
			char *filepath    = parser_material_enum_data(base_enum_texts(n->type)->buffer[i]);
			i32   asset_index = -1;
			for (i32 i = 0; i < project_assets->length; ++i) {
				if (string_equals(project_assets->buffer[i]->file, filepath)) {
					asset_index = i;
					break;
				}
			}
			if (asset_index > -1) {
				img = project_get_image(project_assets->buffer[asset_index]);
			}
		}
	}
	else if (starts_with(n->type, "NEURAL_") && !string_equals(n->type, "NEURAL_IMAGE_TO_PBR")) {
		img = any_imap_get(neural_node_results, n->id);
	}
	else if (ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL) {
		img = any_imap_get(context_raw->node_preview_map, n->id);
	}
	return img;
}

void ui_nodes_draw_menubar_search(void *_) {
	ui_nodes_node_search(math_floor(ui_nodes_node_search_x), math_floor(ui_nodes_node_search_y), NULL);
}

void ui_nodes_draw_menubar() {
	ui_node_canvas_t *c     = ui_nodes_get_canvas(true);
	i32               ew    = math_floor(UI_ELEMENT_W() * 0.7);
	i32               top_y = ui_menu_top_y();
	draw_set_color(ui->ops->theme->WINDOW_BG_COL);
	draw_filled_rect(0, top_y, ui_nodes_ww, UI_ELEMENT_H() + UI_ELEMENT_OFFSET() * 2);
	draw_set_color(0xffffffff);

	i32 start_y = top_y + UI_ELEMENT_OFFSET();
	ui->_x      = 0;
	ui->_y      = 2 + start_y;
	ui->_w      = ew;

	// Editable canvas name
	ui_handle_t *h = ui_handle(__ID__);
	h->text        = string_copy(c->name);
	ui->_w         = math_floor(math_min(draw_string_width(ui->ops->font, ui->font_size, h->text) + 15 * UI_SCALE(), 100 * UI_SCALE()));
	char *new_name = ui_text_input(h, "", UI_ALIGN_LEFT, true, false);
	ui->_x += ui->_w + 3;
	ui->_y = 2 + start_y;
	ui->_w = ew;
	if (h->changed) { // Check whether renaming is possible and update group links
		if (ui_nodes_group_stack->length > 0) {
			bool can_rename = true;
			for (i32 i = 0; i < project_material_groups->length; ++i) {
				node_group_t *m = project_material_groups->buffer[i];
				if (string_equals(m->canvas->name, new_name)) {
					can_rename = false; // Name already used
				}
			}
			if (can_rename) {
				char *old_name                     = c->name;
				c->name                            = string_copy(new_name);
				ui_node_canvas_t_array_t *canvases = any_array_create_from_raw((void *[]){}, 0);
				for (i32 i = 0; i < project_materials->length; ++i) {
					slot_material_t *m = project_materials->buffer[i];
					any_array_push(canvases, m->canvas);
				}
				for (i32 i = 0; i < project_material_groups->length; ++i) {
					node_group_t *m = project_material_groups->buffer[i];
					any_array_push(canvases, m->canvas);
				}
				for (i32 i = 0; i < canvases->length; ++i) {
					ui_node_canvas_t *canvas = canvases->buffer[i];
					for (i32 i = 0; i < canvas->nodes->length; ++i) {
						ui_node_t *n = canvas->nodes->buffer[i];
						if (string_equals(n->type, "GROUP") && string_equals(n->name, old_name)) {
							n->name = string_copy(c->name);
						}
					}
				}
			}
		}
		else {
			c->name = string_copy(new_name);
		}
	}
	i32  _BUTTON_COL           = ui->ops->theme->BUTTON_COL;
	bool _SHADOWS              = ui->ops->theme->SHADOWS;
	ui->ops->theme->BUTTON_COL = ui->ops->theme->WINDOW_BG_COL;
	ui->ops->theme->SHADOWS    = false;
	string_t_array_t *cats     = ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL ? nodes_material_categories : nodes_brush_categories;
	for (i32 i = 0; i < cats->length; ++i) {
		if ((ui_menubar_button(tr(cats->buffer[i]))) || (ui->is_hovered && ui_nodes_show_menu)) {
			ui_nodes_show_menu     = true;
			ui_nodes_menu_category = i;
			ui_nodes_popup_x       = ui_nodes_wx + ui->_x;
			ui_nodes_popup_y       = ui_nodes_wy + ui->_y;
			if (config_raw->touch_ui) {
				ui_nodes_show_menu_first = true;
				i32 menuw                = math_floor(ew * 2.3);
				ui_nodes_popup_x -= menuw / (float)2;
				ui_nodes_popup_x += ui->_w / (float)2;
			}
		}
		ui->_x += ui->_w + 3;
		ui->_y = 2 + start_y;
	}
	if (config_raw->touch_ui) {
		i32 _w = ui->_w;
		ui->_w = math_floor(36 * UI_SCALE());
		ui->_y = 4 * UI_SCALE() + start_y;
		if (ui_menubar_icon_button(ICON_SEARCH)) {
			ui_nodes_node_search(math_floor(ui->_window_x + ui->_x), math_floor(ui->_window_y + ui->_y), NULL);
		}
		ui->_w = _w;
	}
	else {
		if (ui_menubar_button(tr("Search"))) {
			ui_nodes_node_search_x = ui->_window_x + ui->_x;
			ui_nodes_node_search_y = ui->_window_y + ui->_y;
			// Allow for node menu to be closed first
			sys_notify_on_next_frame(&ui_nodes_draw_menubar_search, NULL);
		}
	}
	if (ui->is_hovered) {
		ui_tooltip(string("%s (%s)", tr("Search for nodes"), any_map_get(config_keymap, "node_search")));
	}
	ui->_x += ui->_w + 3;
	ui->_y                     = 2 + start_y;
	ui->ops->theme->BUTTON_COL = _BUTTON_COL;
	ui->ops->theme->SHADOWS    = _SHADOWS;
	// Close node group
	if (ui_nodes_group_stack->length > 0 && ui_menubar_button(tr("Close"))) {
		array_pop(ui_nodes_group_stack);
	}
}

bool ui_nodes_contains_node_group_recursive(node_group_t *group, char *group_name) {
	if (string_equals(group->canvas->name, group_name)) {
		return true;
	}
	for (i32 i = 0; i < group->canvas->nodes->length; ++i) {
		ui_node_t *n = group->canvas->nodes->buffer[i];
		if (string_equals(n->type, "GROUP")) {
			node_group_t *g = project_get_material_group_by_name(n->name);
			if (g != NULL && ui_nodes_contains_node_group_recursive(g, group_name)) {
				return true;
			}
		}
	}
	return false;
}

bool ui_nodes_can_place_group(char *group_name) {
	// Prevent Recursive node groups
	// The group to place must not contain the current group or a group that contains the current group
	if (ui_nodes_group_stack->length > 0) {
		for (i32 i = 0; i < ui_nodes_group_stack->length; ++i) {
			node_group_t *g = ui_nodes_group_stack->buffer[i];
			if (ui_nodes_contains_node_group_recursive(project_get_material_group_by_name(group_name), g->canvas->name))
				return false;
		}
	}
	// Group was deleted / renamed
	bool group_exists = false;
	for (i32 i = 0; i < project_material_groups->length; ++i) {
		node_group_t *group = project_material_groups->buffer[i];
		if (string_equals(group_name, group->canvas->name)) {
			group_exists = true;
		}
	}
	if (!group_exists) {
		return false;
	}
	return true;
}

void ui_nodes_push_undo(ui_node_canvas_t *last_canvas) {
	if (last_canvas == NULL) {
		last_canvas = ui_nodes_get_canvas(true);
	}
	i32 canvas_group = -1;
	if (ui_nodes_group_stack->length > 0) {
		canvas_group = array_index_of(project_material_groups, ui_nodes_group_stack->buffer[ui_nodes_group_stack->length - 1]);
	}
	ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
	history_edit_nodes(last_canvas, ui_nodes_canvas_type, canvas_group);
}

void ui_nodes_accept_asset_drop(i32 index) {
	ui_nodes_push_undo(NULL);
	node_group_t *g = ui_nodes_group_stack->length > 0 ? ui_nodes_group_stack->buffer[ui_nodes_group_stack->length - 1] : NULL;
	ui_node_t    *n = ui_nodes_canvas_type == CANVAS_TYPE_MATERIAL ? nodes_material_create_node("TEX_IMAGE", g) : nodes_brush_create_node("TEX_IMAGE");
	n->buttons->buffer[0]->default_value->buffer[0] = index;
	ui_nodes_get_nodes()->nodes_selected_id         = i32_array_create_from_raw(
        (i32[]){
            n->id,
        },
        1);
}

void ui_nodes_accept_layer_drop(i32 index) {
	ui_nodes_push_undo(NULL);
	if (slot_layer_is_group(project_layers->buffer[index])) {
		return;
	}
	node_group_t *g                                 = ui_nodes_group_stack->length > 0 ? ui_nodes_group_stack->buffer[ui_nodes_group_stack->length - 1] : NULL;
	ui_node_t    *n                                 = nodes_material_create_node(slot_layer_is_mask(context_raw->layer) ? "LAYER_MASK" : "LAYER", g);
	n->buttons->buffer[0]->default_value->buffer[0] = index;
	ui_nodes_get_nodes()->nodes_selected_id         = i32_array_create_from_raw(
        (i32[]){
            n->id,
        },
        1);
}

void ui_nodes_accept_material_drop(i32 index) {
	ui_nodes_push_undo(NULL);
	node_group_t *g                                 = ui_nodes_group_stack->length > 0 ? ui_nodes_group_stack->buffer[ui_nodes_group_stack->length - 1] : NULL;
	ui_node_t    *n                                 = nodes_material_create_node("MATERIAL", g);
	n->buttons->buffer[0]->default_value->buffer[0] = index;
	ui_nodes_get_nodes()->nodes_selected_id         = i32_array_create_from_raw(
        (i32[]){
            n->id,
        },
        1);
}

void ui_nodes_accept_swatch_drop(swatch_color_t *swatch) {
	ui_nodes_push_undo(NULL);
	node_group_t *g                         = ui_nodes_group_stack->length > 0 ? ui_nodes_group_stack->buffer[ui_nodes_group_stack->length - 1] : NULL;
	ui_node_t    *n                         = nodes_material_create_node("RGB", g);
	n->outputs->buffer[0]->default_value    = f32_array_create_xyzw(color_get_rb(swatch->base) / (float)255, color_get_gb(swatch->base) / (float)255,
	                                                                color_get_bb(swatch->base) / (float)255, color_get_ab(swatch->base) / (float)255);
	ui_nodes_get_nodes()->nodes_selected_id = i32_array_create_from_raw(
	    (i32[]){
	        n->id,
	    },
	    1);
}

ui_node_t *ui_nodes_make_node(ui_node_t *n, ui_nodes_t *nodes, ui_node_canvas_t *canvas) {
	ui_node_t *node = GC_ALLOC_INIT(ui_node_t, {0});
	node->id        = ui_next_node_id(canvas->nodes);
	node->name      = string_copy(n->name);
	node->type      = string_copy(n->type);
	node->x         = ui_nodes_get_node_x();
	node->y         = ui_nodes_get_node_y();
	node->color     = n->color;
	node->inputs    = any_array_create_from_raw((void *[]){}, 0);
	node->outputs   = any_array_create_from_raw((void *[]){}, 0);
	node->buttons   = any_array_create_from_raw((void *[]){}, 0);
	node->width     = 0;
	node->flags     = config_raw->node_previews ? UI_NODE_FLAG_PREVIEW : UI_NODE_FLAG_NONE;
	i32 count       = 0;
	for (i32 i = 0; i < n->inputs->length; ++i) {
		ui_node_socket_t *soc = GC_ALLOC_INIT(ui_node_socket_t, {0});
		soc->id               = ui_get_socket_id(canvas->nodes) + count;
		count++;
		soc->node_id       = node->id;
		soc->name          = string_copy(n->inputs->buffer[i]->name);
		soc->type          = string_copy(n->inputs->buffer[i]->type);
		soc->color         = n->inputs->buffer[i]->color;
		soc->default_value = f32_array_create_from_array(n->inputs->buffer[i]->default_value);
		soc->min           = n->inputs->buffer[i]->min;
		soc->max           = n->inputs->buffer[i]->max;
		soc->precision     = n->inputs->buffer[i]->precision;
		soc->display       = n->inputs->buffer[i]->display;
		any_array_push(node->inputs, soc);
	}
	for (i32 i = 0; i < n->outputs->length; ++i) {
		ui_node_socket_t *soc = GC_ALLOC_INIT(ui_node_socket_t, {0});
		soc->id               = ui_get_socket_id(canvas->nodes) + count;
		count++;
		soc->node_id       = node->id;
		soc->name          = string_copy(n->outputs->buffer[i]->name);
		soc->type          = string_copy(n->outputs->buffer[i]->type);
		soc->color         = n->outputs->buffer[i]->color;
		soc->default_value = f32_array_create_from_array(n->outputs->buffer[i]->default_value);
		soc->min           = n->outputs->buffer[i]->min;
		soc->max           = n->outputs->buffer[i]->max;
		soc->precision     = n->outputs->buffer[i]->precision;
		soc->display       = n->outputs->buffer[i]->display;
		any_array_push(node->outputs, soc);
	}
	for (i32 i = 0; i < n->buttons->length; ++i) {
		ui_node_button_t *but = GC_ALLOC_INIT(ui_node_button_t, {0});
		but->name             = string_copy(n->buttons->buffer[i]->name);
		but->type             = string_copy(n->buttons->buffer[i]->type);
		but->output           = n->buttons->buffer[i]->output;
		but->default_value    = f32_array_create_from_array(n->buttons->buffer[i]->default_value);
		if (n->buttons->buffer[i]->data != NULL) {
			but->data = u8_array_create_from_array(n->buttons->buffer[i]->data);
		}
		but->min       = n->buttons->buffer[i]->min;
		but->max       = n->buttons->buffer[i]->max;
		but->precision = n->buttons->buffer[i]->precision;
		but->height    = n->buttons->buffer[i]->height;
		any_array_push(node->buttons, but);
	}
	gc_unroot(ui_nodes_node_changed);
	ui_nodes_node_changed = node;
	gc_root(ui_nodes_node_changed);
	return node;
}

ui_node_t *ui_nodes_make_group_node(ui_node_canvas_t *group_canvas, ui_nodes_t *nodes, ui_node_canvas_t *canvas) {
	ui_node_t_array_t *category = nodes_material_list->buffer[5];
	ui_node_t         *n        = category->buffer[0];
	ui_node_t         *node     = util_clone_canvas_node(n);
	node->name                  = string_copy(group_canvas->name);
	node->id                    = ui_next_node_id(canvas->nodes);
	node->x                     = ui_nodes_get_node_x();
	node->y                     = ui_nodes_get_node_y();
	ui_node_t *group_input      = NULL;
	ui_node_t *group_output     = NULL;
	for (i32 i = 0; i < project_material_groups->length; ++i) {
		node_group_t *g     = project_material_groups->buffer[i];
		char         *cname = g->canvas->name;
		if (string_equals(cname, node->name)) {
			for (i32 i = 0; i < g->canvas->nodes->length; ++i) {
				ui_node_t *n = g->canvas->nodes->buffer[i];
				if (string_equals(n->type, "GROUP_INPUT")) {
					group_input = n;
				}
				else if (string_equals(n->type, "GROUP_OUTPUT")) {
					group_output = n;
				}
			}
			break;
		}
	}
	if (group_input != NULL && group_output != NULL) {
		for (i32 i = 0; i < group_input->outputs->length; ++i) {
			ui_node_socket_t *soc = group_input->outputs->buffer[i];
			any_array_push(node->inputs, nodes_material_create_socket(nodes, node, soc->name, soc->type, canvas, soc->min, soc->max, soc->default_value));
		}
		for (i32 i = 0; i < group_output->inputs->length; ++i) {
			ui_node_socket_t *soc = group_output->inputs->buffer[i];
			any_array_push(node->outputs, nodes_material_create_socket(nodes, node, soc->name, soc->type, canvas, soc->min, soc->max, soc->default_value));
		}
	}
	return node;
}

void ui_nodes_make_node_preview(ui_node_t *node) {
	context_raw->node_preview_name = string_copy(node->name);

	if (string_equals(node->type, "LAYER") || string_equals(node->type, "LAYER_MASK") || string_equals(node->type, "MATERIAL") ||
	    string_equals(node->type, "OUTPUT_MATERIAL_PBR")) {
		return;
	}

	if (ui_nodes_canvas_type == CANVAS_TYPE_BRUSH) {
		return;
	}

	ui_node_t_array_t *nodes = ui_nodes_get_canvas(false)->nodes;
	if (array_index_of(nodes, node) == -1) {
		return;
	}

	gpu_texture_t *img = any_imap_get(context_raw->node_preview_map, node->id);
	if (img == NULL) {
		img = gpu_create_render_target(util_render_node_preview_size, util_render_node_preview_size, GPU_TEXTURE_FORMAT_RGBA32);
		any_imap_set(context_raw->node_preview_map, node->id, img);
	}

	ui_nodes_hwnd->redraws = 2;
	util_render_make_node_preview(ui_nodes_get_canvas(false), node, img, NULL, NULL);
}

bool ui_nodes_has_group(ui_node_canvas_t *c) {
	for (i32 i = 0; i < c->nodes->length; ++i) {
		ui_node_t *n = c->nodes->buffer[i];
		if (string_equals(n->type, "GROUP")) {
			return true;
		}
	}
	return false;
}

void ui_nodes_traverse_group(ui_node_canvas_t_array_t *mgroups, ui_node_canvas_t *c) {
	for (i32 i = 0; i < c->nodes->length; ++i) {
		ui_node_t *n = c->nodes->buffer[i];
		if (string_equals(n->type, "GROUP")) {
			if (ui_nodes_get_group(mgroups, n->name) == NULL) {
				ui_node_canvas_t_array_t *canvases = any_array_create_from_raw((void *[]){}, 0);
				for (i32 i = 0; i < project_material_groups->length; ++i) {
					node_group_t *g = project_material_groups->buffer[i];
					any_array_push(canvases, g->canvas);
				}
				ui_node_canvas_t *group = ui_nodes_get_group(canvases, n->name);
				any_array_push(mgroups, util_clone_canvas(group));
				ui_nodes_traverse_group(mgroups, group);
			}
		}
	}
}

ui_node_canvas_t *ui_nodes_get_group(ui_node_canvas_t_array_t *canvases, char *name) {
	for (i32 i = 0; i < canvases->length; ++i) {
		ui_node_canvas_t *c = canvases->buffer[i];
		if (string_equals(c->name, name)) {
			return c;
		}
	}
	return NULL;
}

void ui_nodes_capture_output() {
	ui_nodes_t       *ui_nodes = ui_nodes_get_nodes();
	ui_node_canvas_t *c        = ui_nodes_get_canvas(true);
	ui_node_t        *sel      = ui_get_node(c->nodes, ui_nodes->nodes_selected_id->buffer[0]);
	gpu_texture_t    *img      = ui_nodes_get_node_preview_image(sel);
	if (img == NULL) {
		return;
	}

	if (project_raw->packed_assets == NULL) {
		project_raw->packed_assets = any_array_create_from_raw((void *[]){}, 0);
	}

	i32   num = 0;
	char *abs = "/packed/node_preview0.png";
	for (i32 i = 0; i < project_raw->packed_assets->length; ++i) {
		packed_asset_t *pa = project_raw->packed_assets->buffer[i];
		if (string_equals(pa->name, abs)) {
			i = 0;
			num++;
			abs = string("/packed/node_preview%d.png", num);
		}
	}

	packed_asset_t *pa = GC_ALLOC_INIT(packed_asset_t, {.name = abs, .bytes = iron_encode_png(gpu_get_texture_pixels(img), img->width, img->height, 0)});
	any_array_push(project_raw->packed_assets, pa);
	any_map_set(data_cached_images, abs, img);
	import_texture_run(abs, true);
}
