
let ui_nodes_show: bool = false;
let ui_nodes_wx: i32;
let ui_nodes_wy: i32;
let ui_nodes_ww: i32;
let ui_nodes_wh: i32;

let ui_nodes_canvas_type: canvas_type_t = canvas_type_t.MATERIAL;
let ui_nodes_show_menu: bool            = false;
let ui_nodes_show_menu_first: bool      = true;
let ui_nodes_hide_menu: bool            = false;
let ui_nodes_menu_category: i32         = 0;
let ui_nodes_popup_x: f32               = 0.0;
let ui_nodes_popup_y: f32               = 0.0;
let ui_nodes_node_search_x: i32;
let ui_nodes_node_search_y: i32;

let ui_nodes_uichanged_last: bool          = false;
let ui_nodes_recompile_mat: bool           = false; // Mat preview
let ui_nodes_recompile_mat_final: bool     = false;
let ui_nodes_node_search_spawn: ui_node_t  = null;
let ui_nodes_node_search_offset: i32       = 0;
let ui_nodes_last_canvas: ui_node_canvas_t = null;
let ui_nodes_last_node_selected_id: i32    = -1;
let ui_nodes_release_link: bool            = false;
let ui_nodes_is_node_menu_op: bool         = false;

let ui_nodes_grid: gpu_texture_t         = null;
let ui_nodes_grid_redraw: bool           = true;
let ui_nodes_grid_cell_w: i32            = 200;
let ui_nodes_grid_small_cell_w: i32      = 40;
let ui_nodes_hwnd: ui_handle_t           = ui_handle_create();
let ui_nodes_group_stack: node_group_t[] = [];
let ui_nodes_controls_down: bool         = false;
let ui_nodes_tabs: slot_material_t[]     = null;
let ui_nodes_htab: ui_handle_t           = ui_handle_create();
let ui_nodes_last_zoom: f32              = 1.0;

let _ui_nodes_on_link_drag_link_drag: ui_node_link_t;
let _ui_nodes_on_link_drag_node: ui_node_t;
let _ui_nodes_on_socket_released_socket: ui_node_socket_t;
let _ui_nodes_on_socket_released_node: ui_node_t;
let _ui_nodes_htype: ui_handle_t = ui_handle_create();
let _ui_nodes_hname: ui_handle_t = ui_handle_create();
let _ui_nodes_hmin: ui_handle_t  = ui_handle_create();
let _ui_nodes_hmax: ui_handle_t  = ui_handle_create();
let _ui_nodes_hval0: ui_handle_t = ui_handle_create();
let _ui_nodes_hval1: ui_handle_t = ui_handle_create();
let _ui_nodes_hval2: ui_handle_t = ui_handle_create();
let _ui_nodes_hval3: ui_handle_t = ui_handle_create();
let _ui_nodes_on_canvas_released_selected: ui_node_t;
let _ui_nodes_node_search_first: bool;
let _ui_nodes_node_search_done: ()   => void;
let ui_nodes_node_changed: ui_node_t  = null;

function ui_viewnodes_init() {
	ui_nodes_preview_image      = ui_nodes_get_node_preview_image;
	ui_nodes_on_link_drag       = ui_viewnodes_on_link_drag;
	ui_nodes_on_node_remove     = ui_viewnodes_on_node_remove;
	ui_nodes_on_node_changed    = ui_viewnodes_on_node_changed;
	ui_nodes_on_socket_released = ui_viewnodes_on_socket_released;
	ui_nodes_on_canvas_released = ui_viewnodes_on_canvas_released;
	ui_nodes_on_canvas_control  = ui_viewnodes_on_canvas_control;
	ui_nodes_grid_snap          = config_raw.grid_snap;
	nodes_material_init();
}

function ui_viewnodes_on_node_remove(n: ui_node_t) {
	let img: gpu_texture_t = any_imap_get(context_raw.node_preview_map, n.id);
	if (img != null) {
		gpu_delete_texture(img);
		imap_delete(context_raw.node_preview_map, n.id);
	}
}

function ui_viewnodes_on_node_changed(n: ui_node_t) {
	ui_nodes_node_changed = n;
}

function ui_viewnodes_on_link_drag(link_drag_id: i32, is_new_link: bool) {
	if (!is_new_link) {
		return;
	}

	let links: ui_node_link_t[]   = ui_nodes_get_canvas(true).links;
	let link_drag: ui_node_link_t = ui_get_link(links, link_drag_id);
	let nodes: ui_node_t[]        = ui_nodes_get_canvas(true).nodes;
	let node: ui_node_t           = ui_get_node(nodes, link_drag.from_id > -1 ? link_drag.from_id : link_drag.to_id);
	let link_x: i32               = ui._window_x + UI_NODE_X(node);
	let link_y: i32               = ui._window_y + UI_NODE_Y(node);
	if (link_drag.from_id > -1) {
		link_x += UI_NODE_W(node);
		link_y += UI_OUTPUT_Y(node, link_drag.from_socket);
	}
	else {
		link_y += ui_nodes_INPUT_Y(ui_nodes_get_canvas(true), node, link_drag.to_socket) + UI_OUTPUTS_H(node) + UI_BUTTONS_H(node);
	}

	if (math_abs(mouse_x - link_x) > 5 || math_abs(mouse_y - link_y) > 5) { // Link length

		_ui_nodes_on_link_drag_link_drag = link_drag;
		_ui_nodes_on_link_drag_node      = node;

		ui_nodes_node_search(-1, -1, function() {
			let ui_nodes: ui_nodes_t = ui_nodes_get_nodes();

			let node_selected_id: i32 = _ui_nodes_on_link_drag_node.id;
			if (ui_nodes.nodes_selected_id.length > 0) {
				node_selected_id = ui_nodes.nodes_selected_id[0];
			}

			let link_drag: ui_node_link_t = _ui_nodes_on_link_drag_link_drag;
			let nodes: ui_node_t[]        = ui_nodes_get_canvas(true).nodes;
			let n: ui_node_t              = ui_get_node(nodes, node_selected_id);
			if (link_drag.to_id == -1 && n.inputs.length > 0) {
				link_drag.to_id       = n.id;
				let from_type: string = _ui_nodes_on_link_drag_node.outputs[link_drag.from_socket].type;
				// Connect to the first socket
				link_drag.to_socket = 0;
				// Try to find the first type-matching socket and use it if present
				for (let i: i32 = 0; i < n.inputs.length; ++i) {
					let socket: ui_node_socket_t = n.inputs[i];
					if (socket.type == from_type) {
						link_drag.to_socket = array_index_of(n.inputs, socket);
						break;
					}
				}
				array_push(ui_nodes_get_canvas(true).links, link_drag);
			}
			else if (link_drag.from_id == -1 && n.outputs.length > 0) {
				link_drag.from_id     = n.id;
				link_drag.from_socket = 0;
				array_push(ui_nodes_get_canvas(true).links, link_drag);
			}
		});
	}
	// Selecting which node socket to preview
	else if (link_drag.from_id > -1) {
		i32_imap_set(context_raw.node_preview_socket_map, node.id, link_drag.from_socket);
		ui_nodes_node_changed = node;
	}
}

function ui_viewnodes_on_socket_released(socket_id: i32) {
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let socket: ui_node_socket_t = ui_get_socket(canvas.nodes, socket_id);
	let node: ui_node_t          = ui_get_node(canvas.nodes, socket.node_id);
	if (ui.input_released_r) {
		if (node.type == "GROUP_INPUT" || node.type == "GROUP_OUTPUT") {

			_ui_nodes_on_socket_released_socket = socket;
			_ui_nodes_on_socket_released_node   = node;

			sys_notify_on_next_frame(function() {
				ui_menu_draw(function() {
					let socket: ui_node_socket_t = _ui_nodes_on_socket_released_socket;
					let node: ui_node_t          = _ui_nodes_on_socket_released_node;

					if (ui_menu_button(tr("Edit"))) {
						_ui_nodes_htype.i    = socket.type == "RGBA" ? 0 : socket.type == "VECTOR" ? 1 : 2;
						_ui_nodes_hname.text = socket.name;
						_ui_nodes_hmin.f     = socket.min;
						_ui_nodes_hmax.f     = socket.max;
						if (socket.type == "RGBA" || socket.type == "VECTOR") {
							_ui_nodes_hval0.f = socket.default_value[0];
							_ui_nodes_hval1.f = socket.default_value[1];
							_ui_nodes_hval2.f = socket.default_value[2];
							if (socket.type == "RGBA") {
								_ui_nodes_hval3.f = socket.default_value[3];
							}
						}
						else {
							_ui_nodes_hval0.f = socket.default_value[0];
						}

						sys_notify_on_next_frame(function() {
							ui_end_input();

							ui_box_show_custom(function() {
								let socket: ui_node_socket_t = _ui_nodes_on_socket_released_socket;
								let node: ui_node_t          = _ui_nodes_on_socket_released_node;

								if (ui_tab(ui_handle(__ID__), tr("Socket"))) {
									let type_combo: string[] = [ tr("Color"), tr("Vector"), tr("Value") ];
									let type: i32            = ui_combo(_ui_nodes_htype, type_combo, tr("Type"), true);
									if (_ui_nodes_htype.changed) {
										_ui_nodes_hname.text = type == 0 ? tr("Color") : type == 1 ? tr("Vector") : tr("Value");
									}
									let name: string               = ui_text_input(_ui_nodes_hname, tr("Name"));
									let min: f32                   = ui_float_input(_ui_nodes_hmin, tr("Min"));
									let max: f32                   = ui_float_input(_ui_nodes_hmax, tr("Max"));
									let default_value: f32_array_t = null;
									if (type == 0) {
										ui_row4();
										ui_float_input(_ui_nodes_hval0, tr("R"));
										ui_float_input(_ui_nodes_hval1, tr("G"));
										ui_float_input(_ui_nodes_hval2, tr("B"));
										ui_float_input(_ui_nodes_hval3, tr("A"));
										default_value = f32_array_create_xyzw(_ui_nodes_hval0.f, _ui_nodes_hval1.f, _ui_nodes_hval2.f, _ui_nodes_hval3.f);
									}
									else if (type == 1) {
										ui_row3();
										_ui_nodes_hval0.f = ui_float_input(_ui_nodes_hval0, tr("X"));
										_ui_nodes_hval1.f = ui_float_input(_ui_nodes_hval1, tr("Y"));
										_ui_nodes_hval2.f = ui_float_input(_ui_nodes_hval2, tr("Z"));
										default_value     = f32_array_create_xyz(_ui_nodes_hval0.f, _ui_nodes_hval1.f, _ui_nodes_hval2.f);
									}
									else {
										let f: f32    = ui_float_input(_ui_nodes_hval0, tr("default_value"));
										default_value = f32_array_create_x(f);
									}
									if (ui_button(tr("OK"))) { // || ui.isReturnDown
										socket.name          = name;
										socket.type          = type == 0 ? "RGBA" : type == 1 ? "VECTOR" : "VALUE";
										socket.color         = nodes_material_get_socket_color(socket.type);
										socket.min           = min;
										socket.max           = max;
										socket.default_value = default_value;
										ui_box_hide();
										nodes_material_sync_sockets(node);
										ui_nodes_hwnd.redraws = 2;
									}
								}
							}, 400, 250);
						});
					}
					if (ui_menu_button(tr("Delete"))) {
						let i: i32                   = 0;
						let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
						// Remove links connected to the socket
						while (i < canvas.links.length) {
							let l: ui_node_link_t = canvas.links[i];
							if ((l.from_id == node.id && l.from_socket == array_index_of(node.outputs, socket)) ||
							    (l.to_id == node.id && l.to_socket == array_index_of(node.inputs, socket))) {
								array_splice(canvas.links, i, 1);
							}
							else {
								i++;
							}
						}
						// Remove socket
						array_remove(node.inputs, socket);
						array_remove(node.outputs, socket);
						nodes_material_sync_sockets(node);
					}
				});
			});
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

function ui_viewnodes_on_canvas_released() {
	if (ui.input_released_r && context_in_nodes() && math_abs(ui.input_x - ui.input_started_x) < 2 && math_abs(ui.input_y - ui.input_started_y) < 2) {

		// Node selection
		let nodes: ui_nodes_t        = ui_nodes_get_nodes();
		let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
		let selected: ui_node_t      = null;
		for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
			let node: ui_node_t = canvas.nodes[i];
			if (ui_input_in_rect(ui._window_x + UI_NODE_X(node), ui._window_y + UI_NODE_Y(node), UI_NODE_W(node), UI_NODE_H(canvas, node))) {
				selected = node;
				break;
			}
		}
		if (selected == null) {
			nodes.nodes_selected_id = [];
		}
		else if (array_index_of(nodes.nodes_selected_id, selected.id) == -1) {
			nodes.nodes_selected_id = [ selected.id ];
		}

		// Node context menu
		if (!ui_nodes_socket_released) {

			_ui_nodes_on_canvas_released_selected = selected;

			ui_menu_draw(function() {
				let selected: ui_node_t = _ui_nodes_on_canvas_released_selected;

				ui._y += 1;
				let is_protected: bool = selected == null || selected.type == "OUTPUT_MATERIAL_PBR" || selected.type == "GROUP_INPUT" ||
				                         selected.type == "GROUP_OUTPUT" || selected.type == "brush_output_node";
				ui.enabled = !is_protected;
				if (ui_menu_button(tr("Cut"), "ctrl+x")) {
					sys_notify_on_next_frame(function() {
						ui_nodes_hwnd.redraws    = 2;
						ui_is_copy               = true;
						ui_is_cut                = true;
						ui_nodes_is_node_menu_op = true;
					});
				}
				if (ui_menu_button(tr("Copy"), "ctrl+c")) {
					sys_notify_on_next_frame(function() {
						ui_is_copy               = true;
						ui_nodes_is_node_menu_op = true;
					});
				}
				ui.enabled = ui_clipboard != "";
				if (ui_menu_button(tr("Paste"), "ctrl+v")) {
					sys_notify_on_next_frame(function() {
						ui_nodes_hwnd.redraws    = 2;
						ui_is_paste              = true;
						ui_nodes_is_node_menu_op = true;
					});
				}
				ui.enabled = !is_protected;
				if (ui_menu_button(tr("Delete"), "delete")) {
					sys_notify_on_next_frame(function() {
						sys_notify_on_end_frame(function() {
							ui_nodes_hwnd.redraws    = 2;
							ui.is_delete_down        = true;
							ui_nodes_is_node_menu_op = true;
						});
					});
				}
				if (ui_menu_button(tr("Duplicate"))) {
					sys_notify_on_next_frame(function() {
						ui_nodes_hwnd.redraws    = 2;
						ui_is_copy               = true;
						ui_is_paste              = true;
						ui_nodes_is_node_menu_op = true;
					});
				}
				if (selected != null && selected.type == "RGB") {
					if (ui_menu_button(tr("Add Swatch"))) {
						let color: f32_array_t         = selected.outputs[0].default_value;
						let new_swatch: swatch_color_t = make_swatch(color_from_floats(color[0], color[1], color[2], color[3]));
						context_set_swatch(new_swatch);
						array_push(project_raw.swatches, new_swatch);
						ui_base_hwnds[tab_area_t.STATUS].redraws = 1;
					}
				}

				if (ui_menu_button(tr("Capture Output"))) {
					sys_notify_on_next_frame(function() {
						ui_nodes_capture_output();
					});
				}

				if (ui_nodes_canvas_type == canvas_type_t.MATERIAL) {
					ui_menu_separator();
					if (ui_menu_button(tr("2D View"))) {
						ui_base_show_2d_view(view_2d_type_t.NODE);
					}
				}

				ui.enabled = true;
			});
		}
	}

	if (ui.input_released) {
		let nodes: ui_nodes_t        = ui_nodes_get_nodes();
		let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
		for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
			let node: ui_node_t = canvas.nodes[i];
			if (ui_input_in_rect(ui._window_x + UI_NODE_X(node), ui._window_y + UI_NODE_Y(node), UI_NODE_W(node), UI_NODE_H(canvas, node))) {
				if (nodes.nodes_selected_id.length > 0 && node.id == nodes.nodes_selected_id[0]) {
					ui_view2d_hwnd.redraws = 2;
					if (sys_time() - context_raw.select_time < 0.2)
						ui_base_show_2d_view(view_2d_type_t.NODE);
					context_raw.select_time = sys_time();
				}
				break;
			}
		}
	}
}

function ui_viewnodes_on_canvas_control(): ui_canvas_control_t {
	let control: ui_canvas_control_t = ui_nodes_get_canvas_control(ui_nodes_controls_down);
	ui_nodes_controls_down           = control.controls_down;
	return control;
}

function ui_nodes_get_canvas_control(controls_down: bool): ui_canvas_control_t {
	if (config_raw.wrap_mouse && controls_down) {
		if (ui.input_x < ui._window_x) {
			ui.input_x = ui._window_x + ui._window_w;
			iron_mouse_set_position(math_floor(ui.input_x), math_floor(ui.input_y));
		}
		else if (ui.input_x > ui._window_x + ui._window_w) {
			ui.input_x = ui._window_x;
			iron_mouse_set_position(math_floor(ui.input_x), math_floor(ui.input_y));
		}
		else if (ui.input_y < ui._window_y) {
			ui.input_y = ui._window_y + ui._window_h;
			iron_mouse_set_position(math_floor(ui.input_x), math_floor(ui.input_y));
		}
		else if (ui.input_y > ui._window_y + ui._window_h) {
			ui.input_y = ui._window_y;
			iron_mouse_set_position(math_floor(ui.input_x), math_floor(ui.input_y));
		}
	}

	if (operator_shortcut(map_get(config_keymap, "action_pan"), shortcut_type_t.STARTED) ||
	    operator_shortcut(map_get(config_keymap, "action_zoom"), shortcut_type_t.STARTED) || ui.input_started_r || ui.input_wheel_delta != 0.0) {
		controls_down = true;
	}
	else if (!operator_shortcut(map_get(config_keymap, "action_pan"), shortcut_type_t.DOWN) &&
	         !operator_shortcut(map_get(config_keymap, "action_zoom"), shortcut_type_t.DOWN) && !ui.input_down_r && ui.input_wheel_delta == 0.0) {
		controls_down = false;
	}
	if (!controls_down) {
		let cc: ui_canvas_control_t = {pan_x : 0, pan_y : 0, zoom : 0, controls_down : controls_down};
		return cc;
	}

	let pan: bool                    = ui.input_down_r || operator_shortcut(map_get(config_keymap, "action_pan"), shortcut_type_t.DOWN);
	let zoom_delta: f32              = operator_shortcut(map_get(config_keymap, "action_zoom"), shortcut_type_t.DOWN) ? ui_nodes_get_zoom_delta() / 100.0 : 0.0;
	let control: ui_canvas_control_t = {
		pan_x : pan ? ui.input_dx : 0.0,
		pan_y : pan ? ui.input_dy : 0.0,
		zoom : ui.input_wheel_delta != 0.0 ? -ui.input_wheel_delta / 10 : zoom_delta,
		controls_down : controls_down
	};
	if (ui.combo_selected_handle != null) {
		control.zoom = 0.0;
	}
	if (control.zoom != 0.0) {
		ui_nodes_grid_redraw = true;
	}

	return control;
}

function ui_nodes_get_zoom_delta(): f32 {
	return config_raw.zoom_direction == zoom_direction_t.VERTICAL              ? -ui.input_dy
	       : config_raw.zoom_direction == zoom_direction_t.VERTICAL_INVERTED   ? -ui.input_dy
	       : config_raw.zoom_direction == zoom_direction_t.HORIZONTAL          ? ui.input_dx
	       : config_raw.zoom_direction == zoom_direction_t.HORIZONTAL_INVERTED ? ui.input_dx
	                                                                           : -(ui.input_dy - ui.input_dx);
}

function ui_nodes_is_tab_selected(): bool {
	return ui_nodes_htab.i > 0 && ui_nodes_htab.i % 2 == 1 && // [tab0, tab1, x, tab2, x, +]
	       ui_nodes_tabs.length >= ui_nodes_htab.i / 2;
}

function ui_nodes_tab_index(): i32 {
	return (int)(ui_nodes_htab.i / 2);
}

function ui_nodes_get_canvas(groups: bool = false): ui_node_canvas_t {
	if (ui_nodes_canvas_type == canvas_type_t.MATERIAL) {
		if (groups && ui_nodes_group_stack.length > 0) {
			return ui_nodes_group_stack[ui_nodes_group_stack.length - 1].canvas;
		}
		else if (ui_nodes_is_tab_selected()) {
			return ui_nodes_tabs[ui_nodes_tab_index()].canvas;
		}
		else {
			return context_raw.material.canvas;
		}
	}
	else {
		return context_raw.brush.canvas;
	}
}

function ui_nodes_get_nodes(): ui_nodes_t {
	if (ui_nodes_canvas_type == canvas_type_t.MATERIAL) {
		if (ui_nodes_group_stack.length > 0) {
			return ui_nodes_group_stack[ui_nodes_group_stack.length - 1].nodes;
		}
		else if (ui_nodes_is_tab_selected()) {
			return ui_nodes_tabs[ui_nodes_tab_index()].nodes;
		}
		else {
			return context_raw.material.nodes;
		}
	}
	else {
		return context_raw.brush.nodes;
	}
}

function ui_nodes_update() {
	if (!ui_nodes_show || !base_ui_enabled) {
		return;
	}

	ui_nodes_wx = math_floor(sys_w()) + ui_toolbar_w(true);
	ui_nodes_wy = ui_header_h * 2;

	if (ui_view2d_show) {
		ui_nodes_wy += sys_h() - config_raw.layout[layout_size_t.NODES_H];
	}

	let ww: i32 = config_raw.layout[layout_size_t.NODES_W];
	if (!ui_base_show) {
		ww += config_raw.layout[layout_size_t.SIDEBAR_W] + ui_toolbar_w(true);
		ui_nodes_wx -= ui_toolbar_w(true);
		ui_nodes_wy = 0;
	}
	if (!base_view3d_show) {
		ww += base_view3d_w();
	}

	let mx: i32 = mouse_x;
	let my: i32 = mouse_y;
	if (mx < ui_nodes_wx || mx > ui_nodes_wx + ww || my < ui_nodes_wy) {
		return;
	}
	if (ui.is_typing || !ui.input_enabled) {
		return;
	}

	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	if (nodes.nodes_selected_id.length > 0 && ui.is_key_pressed) {
		if (ui.key_code == key_code_t.LEFT) {
			for (let i: i32 = 0; i < nodes.nodes_selected_id.length; ++i) {
				let n: i32              = nodes.nodes_selected_id[i];
				let _nodes: ui_node_t[] = ui_nodes_get_canvas(true).nodes;
				ui_get_node(_nodes, n).x -= 1;
			}
		}
		else if (ui.key_code == key_code_t.RIGHT) {
			for (let i: i32 = 0; i < nodes.nodes_selected_id.length; ++i) {
				let n: i32              = nodes.nodes_selected_id[i];
				let _nodes: ui_node_t[] = ui_nodes_get_canvas(true).nodes;
				ui_get_node(_nodes, n).x += 1;
			}
		}
		if (ui.key_code == key_code_t.UP) {
			for (let i: i32 = 0; i < nodes.nodes_selected_id.length; ++i) {
				let n: i32              = nodes.nodes_selected_id[i];
				let _nodes: ui_node_t[] = ui_nodes_get_canvas(true).nodes;
				ui_get_node(_nodes, n).y -= 1;
			}
		}
		else if (ui.key_code == key_code_t.DOWN) {
			for (let i: i32 = 0; i < nodes.nodes_selected_id.length; ++i) {
				let n: i32              = nodes.nodes_selected_id[i];
				let _nodes: ui_node_t[] = ui_nodes_get_canvas(true).nodes;
				ui_get_node(_nodes, n).y += 1;
			}
		}
	}

	// Node search popup
	if (operator_shortcut(map_get(config_keymap, "node_search"))) {
		ui_nodes_node_search();
	}
	if (ui_nodes_node_search_spawn != null) {
		ui.input_x                 = mouse_x; // Fix inputDX after popup removal
		ui.input_y                 = mouse_y;
		ui_nodes_node_search_spawn = null;
	}

	if (operator_shortcut(map_get(config_keymap, "view_reset"))) {
		nodes.pan_x = 0.0;
		nodes.pan_y = 0.0;
		nodes.zoom  = 1.0;
	}

	if (operator_shortcut(map_get(config_keymap, "node_overview"))) {
		nodes.zoom = nodes.zoom == 1.0 ? 0.2 : 1.0;
		nodes.uiw  = ui_nodes_ww;
		nodes.uih  = ui_nodes_wh;
	}
}

function ui_nodes_canvas_changed() {
	ui_nodes_recompile_mat       = true;
	ui_nodes_recompile_mat_final = true;
}

function ui_nodes_node_search(x: i32 = -1, y: i32 = -1, done: () => void = null) {
	_ui_nodes_node_search_first = true;
	_ui_nodes_node_search_done  = done;

	ui_menu_draw(function() {
		ui_menu_h                      = UI_ELEMENT_H() * 8;
		let search_handle: ui_handle_t = ui_handle(__ID__);
		let search: string             = to_lower_case(ui_text_input(search_handle, "", ui_align_t.LEFT, true, true));
		ui.changed                     = false;
		if (_ui_nodes_node_search_first) {
			_ui_nodes_node_search_first = false;
			search_handle.text          = "";
			ui_start_text_edit(search_handle); // Focus search bar
		}

		if (search_handle.changed) {
			ui_nodes_node_search_offset = 0;
		}

		if (ui.is_key_pressed) { // Move selection
			if (ui.key_code == key_code_t.DOWN && ui_nodes_node_search_offset < 6) {
				ui_nodes_node_search_offset++;
			}
			if (ui.key_code == key_code_t.UP && ui_nodes_node_search_offset > 0) {
				ui_nodes_node_search_offset--;
			}
		}
		let enter: bool             = keyboard_down("enter");
		let count: i32              = 0;
		let BUTTON_COL: i32         = ui.ops.theme.BUTTON_COL;
		let FILL_BUTTON_BG: bool    = ui.ops.theme.FILL_BUTTON_BG;
		ui.ops.theme.FILL_BUTTON_BG = true;

		let node_list: node_list_t[] = ui_nodes_canvas_type == canvas_type_t.MATERIAL ? nodes_material_list : nodes_brush_list;

		for (let i: i32 = 0; i < node_list.length; ++i) {
			let list: ui_node_t[] = node_list[i];
			for (let i: i32 = 0; i < list.length; ++i) {
				let n: ui_node_t = list[i];
				if (string_index_of(to_lower_case(tr(n.name)), search) >= 0) {
					ui.ops.theme.BUTTON_COL = count == ui_nodes_node_search_offset ? ui.ops.theme.HIGHLIGHT_COL : ui.ops.theme.SEPARATOR_COL;
					if (ui_button(tr(n.name), ui_align_t.LEFT) || (enter && count == ui_nodes_node_search_offset)) {
						ui_nodes_push_undo();
						let nodes: ui_nodes_t        = ui_nodes_get_nodes();
						let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
						ui_nodes_node_search_spawn   = ui_nodes_make_node(n, nodes, canvas); // Spawn selected node
						array_push(canvas.nodes, ui_nodes_node_search_spawn);
						nodes.nodes_selected_id = [ ui_nodes_node_search_spawn.id ];
						nodes.nodes_drag        = true;

						ui_nodes_hwnd.redraws = 2;
						if (enter) {
							ui.changed = true;
							count      = 6; // Trigger break
						}
						if (_ui_nodes_node_search_done != null) {
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
			ui.changed         = true;
			search_handle.text = "";
		}
		ui.ops.theme.BUTTON_COL     = BUTTON_COL;
		ui.ops.theme.FILL_BUTTON_BG = FILL_BUTTON_BG;
	}, x, y);
}

function ui_nodes_get_node_x(): i32 {
	return math_floor((mouse_x - ui_nodes_wx - UI_NODES_PAN_X()) / UI_NODES_SCALE());
}

function ui_nodes_get_node_y(): i32 {
	return math_floor((mouse_y - ui_nodes_wy - UI_NODES_PAN_Y()) / UI_NODES_SCALE());
}

function ui_nodes_draw_grid(zoom: f32): gpu_texture_t {
	let ww: i32 = config_raw.layout[layout_size_t.NODES_W];

	if (!ui_base_show) {
		ww += config_raw.layout[layout_size_t.SIDEBAR_W] + ui_toolbar_w(true);
	}
	if (!base_view3d_show) {
		ww += base_view3d_w();
	}

	let wh: i32   = sys_h();
	let step: f32 = ui_nodes_grid_cell_w * zoom;
	let mult: i32 = 5 * UI_SCALE();
	let w: i32    = math_floor(ww + step * mult);
	let h: i32    = math_floor(wh + step * mult);
	if (w < 1) {
		w = 1;
	}
	if (h < 1) {
		h = 1;
	}

	let grid: gpu_texture_t = gpu_create_render_target(w, h);
	draw_begin(grid, true, ui.ops.theme.SEPARATOR_COL);

	let sep_col: i32      = ui.ops.theme.SEPARATOR_COL;
	let line_primary: i32 = sep_col - 0x00050505;
	if (line_primary < 0xff000000) {
		line_primary = sep_col + 0x00050505;
	}

	let line_secondary: i32 = sep_col - 0x00090909;
	if (line_secondary < 0xff000000) {
		line_secondary = sep_col + 0x00090909;
	}

	draw_set_color(line_primary);
	step = ui_nodes_grid_small_cell_w * zoom;
	for (let i: i32 = 0; i < math_floor(h / step) + 1; ++i) {
		draw_line(0, i * step, w, i * step);
	}
	for (let i: i32 = 0; i < math_floor(w / step) + 1; ++i) {
		draw_line(i * step, 0, i * step, h);
	}

	draw_set_color(line_secondary);
	step = ui_nodes_grid_cell_w * zoom;
	for (let i: i32 = 0; i < math_floor(h / step) + 1; ++i) {
		draw_line(0, i * step, w, i * step);
	}
	for (let i: i32 = 0; i < math_floor(w / step) + 1; ++i) {
		draw_line(i * step, 0, i * step, h);
	}

	draw_end();
	return grid;
}

let _ui_nodes_render_tmp: (col: i32) => void;

function ui_nodes_recompile() {
	if (ui_nodes_recompile_mat) {
		if (ui_nodes_canvas_type == canvas_type_t.BRUSH) {
			make_material_parse_brush();
			util_render_make_brush_preview();
			ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		}
		else {
			let _material: slot_material_t = context_raw.material;
			if (ui_nodes_is_tab_selected()) {
				context_raw.material = ui_nodes_tabs[ui_nodes_tab_index()];
			}
			layers_is_fill_material() ? layers_update_fill_layers() : util_render_make_material_preview();
			context_raw.material = _material;

			if (ui_view2d_show && ui_view2d_type == view_2d_type_t.NODE) {
				ui_view2d_hwnd.redraws = 2;
			}
		}

		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		if (context_raw.split_view) {
			context_raw.ddirty = 2;
		}

		ui_nodes_recompile_mat = false;
	}
	else if (ui_nodes_recompile_mat_final) {
		make_material_parse_paint_material();

		if (ui_nodes_canvas_type == canvas_type_t.MATERIAL && layers_is_fill_material()) {
			layers_update_fill_layers();
			util_render_make_material_preview();
		}

		let decal: bool = context_is_decal();
		if (decal) {
			util_render_make_decal_preview();
		}

		ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		ui_nodes_recompile_mat_final               = false;
	}
}

function ui_nodes_get_linked_nodes(linked_nodes: ui_node_t[], n: ui_node_t, c: ui_node_canvas_t) {
	if (array_index_of(linked_nodes, n) == -1 && (n.flags & _ui_node_flag_t.PREVIEW)) {
		array_push(linked_nodes, n);
	}
	for (let i: i32 = 0; i < c.links.length; ++i) {
		let l: ui_node_link_t = c.links[i];
		if (l.from_id == n.id) {
			let nn: ui_node_t = ui_get_node(c.nodes, l.to_id);
			ui_nodes_get_linked_nodes(linked_nodes, nn, c);
		}
	}
}

function ui_nodes_render() {
	ui_nodes_recompile();

	let ui_nodes: ui_nodes_t = ui_nodes_get_nodes();

	// Remove dragged link when mouse is released out of the node viewport
	let c: ui_node_canvas_t = ui_nodes_get_canvas(true);
	if (ui_nodes_release_link && ui_nodes.link_drag_id != -1) {
		array_remove(c.links, ui_get_link(c.links, ui_nodes.link_drag_id));
		ui_nodes.link_drag_id = -1;
	}
	ui_nodes_release_link = ui.input_released;

	if (!ui_nodes_show) {
		return;
	}

	ui.input_enabled = base_ui_enabled;

	if (ui_nodes_last_zoom != ui_nodes.zoom) {
		ui_nodes_last_zoom   = ui_nodes.zoom;
		ui_nodes_grid_redraw = true;
	}

	if (ui_nodes_grid_redraw) {
		if (ui_nodes_grid != null) {
			gpu_delete_texture(ui_nodes_grid);
		}
		ui_nodes_grid        = ui_nodes_draw_grid(ui_nodes.zoom);
		ui_nodes_grid_redraw = false;
	}

	// Selected node preview
	if (ui_nodes.nodes_selected_id.length > 0 && ui_nodes.nodes_selected_id[0] != ui_nodes_last_node_selected_id) {
		ui_nodes_last_node_selected_id = ui_nodes.nodes_selected_id[0];
		let sel: ui_node_t             = ui_get_node(c.nodes, ui_nodes.nodes_selected_id[0]);
		ui_nodes_make_node_preview(sel);
	}
	else if (ui_nodes.nodes_selected_id.length == 0) {
		ui_nodes_last_node_selected_id = -1;
	}

	// Update node previews
	if (ui_nodes_node_changed != null && !ui.input_down) {
		let linked_nodes: ui_node_t[] = [];
		ui_nodes_get_linked_nodes(linked_nodes, ui_nodes_node_changed, c);

		let sel: ui_node_t = ui_nodes.nodes_selected_id.length > 0 ? ui_get_node(c.nodes, ui_nodes.nodes_selected_id[0]) : null;
		if (sel != null && array_index_of(linked_nodes, sel) == -1) {
			array_push(linked_nodes, sel);
		}

		for (let i: i32 = 0; i < linked_nodes.length; ++i) {
			ui_nodes_make_node_preview(linked_nodes[i]);
		}
		ui_nodes_node_changed = null;
	}

	// Start with UI
	ui_begin(ui);

	// Make window
	ui_nodes_ww = config_raw.layout[layout_size_t.NODES_W];
	ui_nodes_wx = math_floor(sys_w()) + ui_toolbar_w(true);
	ui_nodes_wy = 0;

	if (!ui_base_show) {
		ui_nodes_ww += config_raw.layout[layout_size_t.SIDEBAR_W] + ui_toolbar_w(true);
		ui_nodes_wx -= ui_toolbar_w(true);
	}
	if (!base_view3d_show) {
		ui_nodes_ww += base_view3d_w();
	}

	let ew: i32 = math_floor(UI_ELEMENT_W() * 0.7);
	ui_nodes_wh = sys_h();
	if (config_raw.layout[layout_size_t.HEADER] == 1) {
		ui_nodes_wh += ui_header_h * 2;
	}

	if (ui_view2d_show) {
		ui_nodes_wh = config_raw.layout[layout_size_t.NODES_H];
		ui_nodes_wy = sys_h() - config_raw.layout[layout_size_t.NODES_H] + ui_header_h;
		if (config_raw.layout[layout_size_t.HEADER] == 1) {
			ui_nodes_wy += ui_header_h;
		}
		else {
			ui_nodes_wy -= ui_header_h;
		}
		if (!ui_base_show) {
			ui_nodes_wy -= ui_header_h * 2;
		}
	}

	if (ui_window(ui_nodes_hwnd, ui_nodes_wx, ui_nodes_wy, ui_nodes_ww, ui_nodes_wh)) {
		ui_tab(ui_nodes_htab, tr("Nodes"), false, -1, !base_view3d_show);

		// Additional tabs
		if (ui_nodes_canvas_type == canvas_type_t.MATERIAL) {
			if (ui_nodes_tabs == null) {
				ui_nodes_tabs = [];
			}

			for (let i: i32 = 0; i < ui_nodes_tabs.length; ++i) {
				ui_tab(ui_nodes_htab, ui_nodes_tabs[i].canvas.name);
				if (ui_tab(ui_nodes_htab, tr("x"))) {
					array_splice(ui_nodes_tabs, i, 1);
					ui_nodes_htab.i = 0;
				}
			}

			if (ui_tab(ui_nodes_htab, tr("+"))) {
				array_push(ui_nodes_tabs, context_raw.material);
			}
		}

		// Grid
		draw_set_color(0xffffffff);
		let step: f32 = ui_nodes_grid_cell_w * ui_nodes.zoom;
		let x: f32    = math_fmod(UI_NODES_PAN_X(), step) - step;
		let y: f32    = math_fmod(UI_NODES_PAN_Y(), step) - step;
		draw_image(ui_nodes_grid, x, y);

		// Undo
		if (ui.input_started || ui.is_key_pressed) {
			ui_nodes_last_canvas = util_clone_canvas(ui_nodes_get_canvas(true));
		}

		// Nodes
		let _input_enabled: bool = ui.input_enabled;
		let header_h: i32        = UI_ELEMENT_H() * 2 + UI_ELEMENT_OFFSET() * 2;
		let header_hover: bool   = ui.input_y < ui._window_y + header_h;
		ui.input_enabled         = _input_enabled && !ui_nodes_show_menu && !header_hover;
		ui.window_border_right   = config_raw.layout[layout_size_t.SIDEBAR_W];
		ui.window_border_top     = ui_header_h * 2;
		ui.window_border_bottom  = config_raw.layout[layout_size_t.STATUS_H];

		ui_node_canvas(ui_nodes, c);
		ui.input_enabled = _input_enabled;

		if (ui_nodes.color_picker_callback != null) {
			context_raw.color_picker_previous_tool = context_raw.tool;
			context_select_tool(tool_type_t.PICKER);
			_ui_nodes_render_tmp = ui_nodes.color_picker_callback;

			context_raw.color_picker_callback = function(color: swatch_color_t) {
				_ui_nodes_render_tmp(color.base);
				ui_nodes_hwnd.redraws = 2;

				let material_live: bool = config_raw.material_live;
				if (material_live) {
					ui_nodes_canvas_changed();
				}
			};
			ui_nodes.color_picker_callback = null;
		}

		// Remove nodes with unknown id for this canvas type
		if (ui_is_paste) {
			let node_list: node_list_t[] = ui_nodes_canvas_type == canvas_type_t.MATERIAL ? nodes_material_list : nodes_brush_list;

			let i: i32 = 0;
			while (i++ < c.nodes.length) {
				let canvas_node: ui_node_t = c.nodes[i - 1];
				if (array_index_of(ui_nodes_exclude_remove, canvas_node.type) >= 0) {
					continue;
				}
				let found: bool = false;
				for (let i: i32 = 0; i < node_list.length; ++i) {
					let list: ui_node_t[] = node_list[i];
					for (let i: i32 = 0; i < list.length; ++i) {
						let list_node: ui_node_t = list[i];
						if (canvas_node.type == list_node.type) {
							found = true;
							break;
						}
					}
					if (found) {
						break;
					}
				}
				if (canvas_node.type == "GROUP" && !ui_nodes_can_place_group(canvas_node.name)) {
					found = false;
				}
				if (!found) {
					ui_remove_node(canvas_node, c);
					array_remove(ui_nodes.nodes_selected_id, canvas_node.id);
					i--;
				}
			}
		}

		if (ui_nodes_is_node_menu_op) {
			ui_is_copy        = false;
			ui_is_cut         = false;
			ui_is_paste       = false;
			ui.is_delete_down = false;
		}

		// Recompile material on change
		if (ui.changed) {
			ui_nodes_recompile_mat = (ui.input_dx != 0 || ui.input_dy != 0 || !ui_nodes_uichanged_last) && config_raw.material_live; // Instant preview
		}
		else if (ui_nodes_uichanged_last) {
			ui_nodes_canvas_changed();
			ui_nodes_push_undo(ui_nodes_last_canvas);
		}
		ui_nodes_uichanged_last = ui.changed;

		// Node previews
		if (context_raw.selected_node_preview && ui_nodes.nodes_selected_id.length > 0) {
			let sel: ui_node_t     = ui_get_node(c.nodes, ui_nodes.nodes_selected_id[0]);
			let img: gpu_texture_t = ui_nodes_get_node_preview_image(sel);
			if (img != null && !(sel.flags & _ui_node_flag_t.PREVIEW)) {
				let tw: f32 = 128 * UI_SCALE();
				let th: f32 = tw * (img.height / img.width);
				let tx: f32 = ui_nodes_ww - tw - 8 * UI_SCALE();
				let ty: f32 = ui_nodes_wh - th - 8 * UI_SCALE();
				if (img == any_imap_get(context_raw.node_preview_map, sel.id)) {
					ui_draw_shadow(tx, ty, tw, th);
				}
				let single_channel: bool = sel.type == "LAYER_MASK";
				if (single_channel) {
					draw_set_pipeline(ui_view2d_pipe);
					gpu_set_int(ui_view2d_channel_loc, 1);
				}
				draw_set_color(0xffffffff);
				draw_scaled_image(img, tx, ty, tw, th);
				if (single_channel) {
					draw_set_pipeline(null);
				}
			}
		}

		ui_nodes_draw_menubar();

		ui.window_border_right  = 0;
		ui.window_border_top    = 0;
		ui.window_border_bottom = 0;
	}

	ui_end();

	if (ui_nodes_show_menu) {
		let list: node_list_t[]     = ui_nodes_canvas_type == canvas_type_t.MATERIAL ? nodes_material_list : nodes_brush_list;
		let category: ui_node_t[]   = list[ui_nodes_menu_category];
		let num_nodes: i32          = category.length;
		let is_group_category: bool = ui_nodes_canvas_type == canvas_type_t.MATERIAL && nodes_material_categories[ui_nodes_menu_category] == "Group";

		if (is_group_category) {
			num_nodes += project_material_groups.length;
		}

		let py: i32    = ui_nodes_popup_y;
		let menuw: i32 = math_floor(ew * 2.3);
		draw_begin(null);
		ui_begin_region(ui, math_floor(ui_nodes_popup_x), math_floor(py), menuw);
		let _FILL_BUTTON_BG: i32    = ui.ops.theme.FILL_BUTTON_BG;
		ui.ops.theme.FILL_BUTTON_BG = false;
		let _ELEMENT_OFFSET: i32    = ui.ops.theme.ELEMENT_OFFSET;
		ui.ops.theme.ELEMENT_OFFSET = 0;
		let _ELEMENT_H: i32         = ui.ops.theme.ELEMENT_H;
		ui.ops.theme.ELEMENT_H      = config_raw.touch_ui ? (28 + 2) : 28;

		ui_menu_h = category.length * UI_ELEMENT_H();
		if (is_group_category) {
			ui_menu_h += project_material_groups.length * UI_ELEMENT_H();
		}

		ui_menu_start();

		for (let i: i32 = 0; i < category.length; ++i) {
			let n: ui_node_t = category[i];
			if (ui_menu_button(tr(n.name))) {
				ui_nodes_push_undo();
				let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
				let nodes: ui_nodes_t        = ui_nodes_get_nodes();
				let node: ui_node_t          = ui_nodes_make_node(n, nodes, canvas);
				array_push(canvas.nodes, node);
				nodes.nodes_selected_id = [ node.id ];
				nodes.nodes_drag        = true;
			}
			// Next column
			if (ui._y - ui_nodes_wy + UI_ELEMENT_H() / 2 > ui_nodes_wh) {
				ui._x += menuw;
				ui._y = py;
			}
		}
		if (is_group_category) {
			for (let i: i32 = 0; i < project_material_groups.length; ++i) {
				let g: node_group_t = project_material_groups[i];
				ui.enabled          = ui_nodes_can_place_group(g.canvas.name);
				let row: f32[]      = [ 5 / 6, 1 / 6 ];
				ui_row(row);
				if (ui_button(config_button_spacing + g.canvas.name, ui_align_t.LEFT)) {
					ui_nodes_push_undo();
					let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
					let nodes: ui_nodes_t        = ui_nodes_get_nodes();
					let node: ui_node_t          = ui_nodes_make_group_node(g.canvas, nodes, canvas);
					array_push(canvas.nodes, node);
					nodes.nodes_selected_id = [ node.id ];
					nodes.nodes_drag        = true;
				}

				ui.enabled = !project_is_material_group_in_use(g);
				if (ui_button("x", ui_align_t.CENTER)) {
					history_delete_material_group(g);
					array_remove(project_material_groups, g);
				}

				ui.enabled = true;
			}
		}

		ui_nodes_hide_menu =
		    ui.combo_selected_handle == null && !ui_nodes_show_menu_first && (ui.changed || ui.input_released || ui.input_released_r || ui.is_escape_down);
		ui_nodes_show_menu_first = false;

		ui.ops.theme.FILL_BUTTON_BG = _FILL_BUTTON_BG;
		ui.ops.theme.ELEMENT_OFFSET = _ELEMENT_OFFSET;
		ui.ops.theme.ELEMENT_H      = _ELEMENT_H;
		ui_menu_end();
		ui_end_region();
		draw_end();
	}

	if (ui_nodes_hide_menu) {
		ui_nodes_show_menu       = false;
		ui_nodes_show_menu_first = true;
	}

	ui.input_enabled = true;
}

function ui_nodes_get_node_preview_image(n: ui_node_t): gpu_texture_t {
	if (n == null) {
		return null;
	}
	let img: gpu_texture_t = null;
	if (n.type == "LAYER" || n.type == "LAYER_MASK") {
		let id: i32 = n.buttons[0].default_value[0];
		if (id < project_layers.length) {
			img = project_layers[id].texpaint_preview;
		}
	}
	else if (n.type == "MATERIAL") {
		let id: i32 = n.buttons[0].default_value[0];
		if (id < project_materials.length) {
			img = project_materials[id].image;
		}
	}
	else if (n.type == "OUTPUT_MATERIAL_PBR") {
		img = context_raw.material.image;
	}
	else if (n.type == "brush_output_node") {
		img = context_raw.brush.image;
	}
	else if (n.type == "TEX_IMAGE" && parser_material_get_input_link(n.inputs[0]) == null) {
		let i: i32           = n.buttons[0].default_value[0];
		let filepath: string = parser_material_enum_data(base_enum_texts(n.type)[i]);
		let asset_index: i32 = -1;
		for (let i: i32 = 0; i < project_assets.length; ++i) {
			if (project_assets[i].file == filepath) {
				asset_index = i;
				break;
			}
		}
		if (asset_index > -1) {
			img = project_get_image(project_assets[asset_index]);
		}
	}
	else if (n.type == "NEURAL_TEXT_TO_IMAGE") {
		img = text_to_image_node_result;
	}
	else if (n.type == "NEURAL_UPSCALE_IMAGE") {
		img = upscale_image_node_result;
	}
	else if (n.type == "NEURAL_EDIT_IMAGE") {
		img = edit_image_node_result;
	}
	else if (n.type == "NEURAL_TILE_IMAGE") {
		img = tile_image_node_result;
	}
	else if (n.type == "NEURAL_INPAINT_IMAGE") {
		img = inpaint_image_node_result;
	}
	else if (ui_nodes_canvas_type == canvas_type_t.MATERIAL) {
		img = any_imap_get(context_raw.node_preview_map, n.id);
	}
	return img;
}

function ui_nodes_draw_menubar() {
	let c: ui_node_canvas_t = ui_nodes_get_canvas(true);
	let ew: i32             = math_floor(UI_ELEMENT_W() * 0.7);

	draw_set_color(ui.ops.theme.WINDOW_BG_COL);
	draw_filled_rect(0, UI_ELEMENT_H(), ui_nodes_ww, UI_ELEMENT_H() + UI_ELEMENT_OFFSET() * 2);
	draw_set_color(0xffffffff);

	let start_y: i32 = UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
	ui._x            = 0;
	ui._y            = 2 + start_y;
	ui._w            = ew;

	// Editable canvas name
	let h: ui_handle_t   = ui_handle(__ID__);
	h.text               = c.name;
	ui._w                = math_floor(math_min(draw_string_width(ui.ops.font, ui.font_size, h.text) + 15 * UI_SCALE(), 100 * UI_SCALE()));
	let new_name: string = ui_text_input(h, "");
	ui._x += ui._w + 3;
	ui._y = 2 + start_y;
	ui._w = ew;

	if (h.changed) { // Check whether renaming is possible and update group links
		if (ui_nodes_group_stack.length > 0) {
			let can_rename: bool = true;
			for (let i: i32 = 0; i < project_material_groups.length; ++i) {
				let m: node_group_t = project_material_groups[i];
				if (m.canvas.name == new_name) {
					can_rename = false; // Name already used
				}
			}

			if (can_rename) {
				let old_name: string             = c.name;
				c.name                           = new_name;
				let canvases: ui_node_canvas_t[] = [];
				for (let i: i32 = 0; i < project_materials.length; ++i) {
					let m: slot_material_t = project_materials[i];
					array_push(canvases, m.canvas);
				}
				for (let i: i32 = 0; i < project_material_groups.length; ++i) {
					let m: node_group_t = project_material_groups[i];
					array_push(canvases, m.canvas);
				}
				for (let i: i32 = 0; i < canvases.length; ++i) {
					let canvas: ui_node_canvas_t = canvases[i];
					for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
						let n: ui_node_t = canvas.nodes[i];
						if (n.type == "GROUP" && n.name == old_name) {
							n.name = c.name;
						}
					}
				}
			}
		}
		else {
			c.name = new_name;
		}
	}

	let _BUTTON_COL: i32    = ui.ops.theme.BUTTON_COL;
	ui.ops.theme.BUTTON_COL = ui.ops.theme.WINDOW_BG_COL;

	let cats: string[] = ui_nodes_canvas_type == canvas_type_t.MATERIAL ? nodes_material_categories : nodes_brush_categories;

	for (let i: i32 = 0; i < cats.length; ++i) {
		if ((_ui_menu_button(tr(cats[i]))) || (ui.is_hovered && ui_nodes_show_menu)) {
			ui_nodes_show_menu     = true;
			ui_nodes_menu_category = i;
			ui_nodes_popup_x       = ui_nodes_wx + ui._x;
			ui_nodes_popup_y       = ui_nodes_wy + ui._y;
			if (config_raw.touch_ui) {
				ui_nodes_show_menu_first = true;
				let menuw: i32           = math_floor(ew * 2.3);
				ui_nodes_popup_x -= menuw / 2;
				ui_nodes_popup_x += ui._w / 2;
			}
		}
		ui._x += ui._w + 3;
		ui._y = 2 + start_y;
	}

	if (config_raw.touch_ui) {
		let _w: i32 = ui._w;
		ui._w       = math_floor(36 * UI_SCALE());
		ui._y       = 4 * UI_SCALE() + start_y;
		if (ui_menubar_icon_button(2, 3)) {
			ui_nodes_node_search(math_floor(ui._window_x + ui._x), math_floor(ui._window_y + ui._y));
		}
		ui._w = _w;
	}
	else {
		if (_ui_menu_button(tr("Search"))) {
			ui_nodes_node_search_x = ui._window_x + ui._x;
			ui_nodes_node_search_y = ui._window_y + ui._y;
			// Allow for node menu to be closed first
			sys_notify_on_next_frame(function() {
				ui_nodes_node_search(math_floor(ui_nodes_node_search_x), math_floor(ui_nodes_node_search_y));
			});
		}
	}
	if (ui.is_hovered) {
		ui_tooltip(tr("Search for nodes") + " (" + map_get(config_keymap, "node_search") + ")");
	}
	ui._x += ui._w + 3;
	ui._y = 2 + start_y;

	ui.ops.theme.BUTTON_COL = _BUTTON_COL;

	// Close node group
	if (ui_nodes_group_stack.length > 0 && _ui_menu_button(tr("Close"))) {
		array_pop(ui_nodes_group_stack);
	}
}

function ui_nodes_contains_node_group_recursive(group: node_group_t, group_name: string): bool {
	if (group.canvas.name == group_name) {
		return true;
	}
	for (let i: i32 = 0; i < group.canvas.nodes.length; ++i) {
		let n: ui_node_t = group.canvas.nodes[i];
		if (n.type == "GROUP") {
			let g: node_group_t = project_get_material_group_by_name(n.name);
			if (g != null && ui_nodes_contains_node_group_recursive(g, group_name)) {
				return true;
			}
		}
	}
	return false;
}

function ui_nodes_can_place_group(group_name: string): bool {
	// Prevent Recursive node groups
	// The group to place must not contain the current group or a group that contains the current group
	if (ui_nodes_group_stack.length > 0) {
		for (let i: i32 = 0; i < ui_nodes_group_stack.length; ++i) {
			let g: node_group_t = ui_nodes_group_stack[i];
			if (ui_nodes_contains_node_group_recursive(project_get_material_group_by_name(group_name), g.canvas.name))
				return false;
		}
	}
	// Group was deleted / renamed
	let group_exists: bool = false;
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let group: node_group_t = project_material_groups[i];
		if (group_name == group.canvas.name) {
			group_exists = true;
		}
	}
	if (!group_exists) {
		return false;
	}
	return true;
}

function ui_nodes_push_undo(last_canvas: ui_node_canvas_t = null) {
	if (last_canvas == null) {
		last_canvas = ui_nodes_get_canvas(true);
	}
	let canvas_group: i32 = -1;
	if (ui_nodes_group_stack.length > 0) {
		canvas_group = array_index_of(project_material_groups, ui_nodes_group_stack[ui_nodes_group_stack.length - 1]);
	}

	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	history_edit_nodes(last_canvas, ui_nodes_canvas_type, canvas_group);
}

function ui_nodes_accept_asset_drop(index: i32) {
	ui_nodes_push_undo();
	let g: node_group_t = ui_nodes_group_stack.length > 0 ? ui_nodes_group_stack[ui_nodes_group_stack.length - 1] : null;
	let n: ui_node_t    = ui_nodes_canvas_type == canvas_type_t.MATERIAL ? nodes_material_create_node("TEX_IMAGE", g) : nodes_brush_create_node("TEX_IMAGE");

	n.buttons[0].default_value[0]          = index;
	ui_nodes_get_nodes().nodes_selected_id = [ n.id ];
}

function ui_nodes_accept_layer_drop(index: i32) {
	ui_nodes_push_undo();
	if (slot_layer_is_group(project_layers[index])) {
		return;
	}
	let g: node_group_t                    = ui_nodes_group_stack.length > 0 ? ui_nodes_group_stack[ui_nodes_group_stack.length - 1] : null;
	let n: ui_node_t                       = nodes_material_create_node(slot_layer_is_mask(context_raw.layer) ? "LAYER_MASK" : "LAYER", g);
	n.buttons[0].default_value[0]          = index;
	ui_nodes_get_nodes().nodes_selected_id = [ n.id ];
}

function ui_nodes_accept_material_drop(index: i32) {
	ui_nodes_push_undo();
	let g: node_group_t                    = ui_nodes_group_stack.length > 0 ? ui_nodes_group_stack[ui_nodes_group_stack.length - 1] : null;
	let n: ui_node_t                       = nodes_material_create_node("MATERIAL", g);
	n.buttons[0].default_value[0]          = index;
	ui_nodes_get_nodes().nodes_selected_id = [ n.id ];
}

function ui_nodes_accept_swatch_drop(swatch: swatch_color_t) {
	ui_nodes_push_undo();
	let g: node_group_t        = ui_nodes_group_stack.length > 0 ? ui_nodes_group_stack[ui_nodes_group_stack.length - 1] : null;
	let n: ui_node_t           = nodes_material_create_node("RGB", g);
	n.outputs[0].default_value = f32_array_create_xyzw(color_get_rb(swatch.base) / 255, color_get_gb(swatch.base) / 255, color_get_bb(swatch.base) / 255,
	                                                   color_get_ab(swatch.base) / 255);
	ui_nodes_get_nodes().nodes_selected_id = [ n.id ];
}

function ui_nodes_make_node(n: ui_node_t, nodes: ui_nodes_t, canvas: ui_node_canvas_t): ui_node_t {
	let node: ui_node_t = {};
	node.id             = ui_next_node_id(canvas.nodes);
	node.name           = n.name;
	node.type           = n.type;
	node.x              = ui_nodes_get_node_x();
	node.y              = ui_nodes_get_node_y();
	node.color          = n.color;
	node.inputs         = [];
	node.outputs        = [];
	node.buttons        = [];
	node.width          = 0;
	node.flags          = config_raw.node_previews ? _ui_node_flag_t.PREVIEW : _ui_node_flag_t.NONE;

	let count: i32 = 0;
	for (let i: i32 = 0; i < n.inputs.length; ++i) {
		let soc: ui_node_socket_t = {};
		soc.id                    = ui_get_socket_id(canvas.nodes) + count;
		count++;
		soc.node_id       = node.id;
		soc.name          = n.inputs[i].name;
		soc.type          = n.inputs[i].type;
		soc.color         = n.inputs[i].color;
		soc.default_value = f32_array_create_from_array(n.inputs[i].default_value);
		soc.min           = n.inputs[i].min;
		soc.max           = n.inputs[i].max;
		soc.precision     = n.inputs[i].precision;
		soc.display       = n.inputs[i].display;
		array_push(node.inputs, soc);
	}

	for (let i: i32 = 0; i < n.outputs.length; ++i) {
		let soc: ui_node_socket_t = {};
		soc.id                    = ui_get_socket_id(canvas.nodes) + count;
		count++;
		soc.node_id       = node.id;
		soc.name          = n.outputs[i].name;
		soc.type          = n.outputs[i].type;
		soc.color         = n.outputs[i].color;
		soc.default_value = f32_array_create_from_array(n.outputs[i].default_value);
		soc.min           = n.outputs[i].min;
		soc.max           = n.outputs[i].max;
		soc.precision     = n.outputs[i].precision;
		soc.display       = n.outputs[i].display;
		array_push(node.outputs, soc);
	}

	for (let i: i32 = 0; i < n.buttons.length; ++i) {
		let but: ui_node_button_t = {};
		but.name                  = n.buttons[i].name;
		but.type                  = n.buttons[i].type;
		but.output                = n.buttons[i].output;
		but.default_value         = f32_array_create_from_array(n.buttons[i].default_value);
		if (n.buttons[i].data != null) {
			but.data = u8_array_create_from_array(n.buttons[i].data);
		}
		but.min       = n.buttons[i].min;
		but.max       = n.buttons[i].max;
		but.precision = n.buttons[i].precision;
		but.height    = n.buttons[i].height;
		array_push(node.buttons, but);
	}

	ui_nodes_node_changed = node;
	return node;
}

function ui_nodes_make_group_node(group_canvas: ui_node_canvas_t, nodes: ui_nodes_t, canvas: ui_node_canvas_t): ui_node_t {
	let category: ui_node_t[]   = nodes_material_list[5];
	let n: ui_node_t            = category[0];
	let node: ui_node_t         = util_clone_canvas_node(n);
	node.name                   = group_canvas.name;
	node.id                     = ui_next_node_id(canvas.nodes);
	node.x                      = ui_nodes_get_node_x();
	node.y                      = ui_nodes_get_node_y();
	let group_input: ui_node_t  = null;
	let group_output: ui_node_t = null;
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		let cname: string   = g.canvas.name;
		if (cname == node.name) {
			for (let i: i32 = 0; i < g.canvas.nodes.length; ++i) {
				let n: ui_node_t = g.canvas.nodes[i];
				if (n.type == "GROUP_INPUT") {
					group_input = n;
				}
				else if (n.type == "GROUP_OUTPUT") {
					group_output = n;
				}
			}
			break;
		}
	}
	if (group_input != null && group_output != null) {
		for (let i: i32 = 0; i < group_input.outputs.length; ++i) {
			let soc: ui_node_socket_t = group_input.outputs[i];
			array_push(node.inputs, nodes_material_create_socket(nodes, node, soc.name, soc.type, canvas, soc.min, soc.max, soc.default_value));
		}
		for (let i: i32 = 0; i < group_output.inputs.length; ++i) {
			let soc: ui_node_socket_t = group_output.inputs[i];
			array_push(node.outputs, nodes_material_create_socket(nodes, node, soc.name, soc.type, canvas, soc.min, soc.max, soc.default_value));
		}
	}
	return node;
}

function ui_nodes_make_node_preview(node: ui_node_t) {
	context_raw.node_preview_name = node.name;

	if (node.type == "LAYER" || node.type == "LAYER_MASK" || node.type == "MATERIAL" || node.type == "OUTPUT_MATERIAL_PBR") {
		return;
	}

	if (ui_nodes_canvas_type == canvas_type_t.BRUSH) {
		return;
	}

	let nodes: ui_node_t[] = ui_nodes_get_canvas().nodes;
	if (array_index_of(nodes, node) == -1) {
		return;
	}

	let img: gpu_texture_t = any_imap_get(context_raw.node_preview_map, node.id);
	if (img == null) {
		img = gpu_create_render_target(util_render_node_preview_size, util_render_node_preview_size);
		any_imap_set(context_raw.node_preview_map, node.id, img);
	}

	ui_nodes_hwnd.redraws = 2;
	util_render_make_node_preview(ui_nodes_get_canvas(), node, img);
}

function ui_nodes_has_group(c: ui_node_canvas_t): bool {
	for (let i: i32 = 0; i < c.nodes.length; ++i) {
		let n: ui_node_t = c.nodes[i];
		if (n.type == "GROUP") {
			return true;
		}
	}
	return false;
}

function ui_nodes_traverse_group(mgroups: ui_node_canvas_t[], c: ui_node_canvas_t) {
	for (let i: i32 = 0; i < c.nodes.length; ++i) {
		let n: ui_node_t = c.nodes[i];
		if (n.type == "GROUP") {
			if (ui_nodes_get_group(mgroups, n.name) == null) {
				let canvases: ui_node_canvas_t[] = [];
				for (let i: i32 = 0; i < project_material_groups.length; ++i) {
					let g: node_group_t = project_material_groups[i];
					array_push(canvases, g.canvas);
				}
				let group: ui_node_canvas_t = ui_nodes_get_group(canvases, n.name);
				array_push(mgroups, util_clone_canvas(group));
				ui_nodes_traverse_group(mgroups, group);
			}
		}
	}
}

function ui_nodes_get_group(canvases: ui_node_canvas_t[], name: string): ui_node_canvas_t {
	for (let i: i32 = 0; i < canvases.length; ++i) {
		let c: ui_node_canvas_t = canvases[i];
		if (c.name == name) {
			return c;
		}
	}
	return null;
}

function ui_nodes_capture_output() {
	let ui_nodes: ui_nodes_t = ui_nodes_get_nodes();
	let c: ui_node_canvas_t  = ui_nodes_get_canvas(true);
	let sel: ui_node_t       = ui_get_node(c.nodes, ui_nodes.nodes_selected_id[0]);
	let img: gpu_texture_t   = ui_nodes_get_node_preview_image(sel);
	if (img == null) {
		return;
	}

	if (project_raw.packed_assets == null) {
		project_raw.packed_assets = [];
	}

	let num: i32    = 0;
	let abs: string = "/packed/node_preview0.png";
	for (let i: i32 = 0; i < project_raw.packed_assets.length; ++i) {
		let pa: packed_asset_t = project_raw.packed_assets[i];
		if (pa.name == abs) {
			i = 0;
			num++;
			abs = "/packed/node_preview" + num + ".png";
		}
	}

	let pa: packed_asset_t = {name : abs, bytes : gpu_get_texture_pixels(img)};
	array_push(project_raw.packed_assets, pa);
	map_set(data_cached_images, abs, img);
	import_texture_run(abs);
}
