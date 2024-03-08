
class UINodes {

	///if (is_paint || is_sculpt)
	static show: bool = false;
	///end
	///if is_lab
	static show: bool = true;
	///end

	static wx: i32;
	static wy: i32;
	static ww: i32;
	static wh: i32;

	static ui: zui_t;
	static canvas_type = canvas_type_t.MATERIAL;
	static show_menu: bool = false;
	static show_menu_first: bool = true;
	static hide_menu: bool = false;
	static menu_category: i32 = 0;
	static popup_x: f32 = 0.0;
	static popup_y: f32 = 0.0;

	static uichanged_last: bool = false;
	static recompile_mat: bool = false; // Mat preview
	static recompile_mat_final: bool = false;
	static node_search_spawn: zui_node_t = null;
	static node_search_offset: i32 = 0;
	static last_canvas: zui_node_canvas_t = null;
	static last_node_selected_id: i32 = -1;
	static release_link: bool = false;
	static is_node_menu_op: bool = false;

	static grid: image_t = null;
	static hwnd: zui_handle_t = zui_handle_create();
	static group_stack: node_group_t[] = [];
	static controls_down: bool = false;

	constructor() {
		zui_set_on_link_drag(UINodes.on_link_drag);
		zui_set_on_socket_released(UINodes.on_socket_released);
		zui_set_on_canvas_released(UINodes.on_canvas_released);
		zui_set_on_canvas_control(UINodes.on_canvas_control);

		let scale: f32 = Config.raw.window_scale;
		UINodes.ui = zui_create({ theme: base_theme, font: base_font, color_wheel: base_color_wheel, black_white_gradient: base_color_wheel_gradient, scale_factor: scale });
		UINodes.ui.scroll_enabled = false;
	}

	static on_link_drag = (linkDragId: i32, isNewLink: bool) => {
		if (isNewLink) {
			let nodes: zui_nodes_t = UINodes.get_nodes();
			let link_drag: zui_node_link_t = zui_get_link(UINodes.get_canvas(true).links, linkDragId);
			let node: zui_node_t = zui_get_node(UINodes.get_canvas(true).nodes, link_drag.from_id > -1 ? link_drag.from_id : link_drag.to_id);
			let link_x: i32 = UINodes.ui._window_x + zui_nodes_NODE_X(node);
			let link_y: i32 = UINodes.ui._window_y + zui_nodes_NODE_Y(node);
			if (link_drag.from_id > -1) {
				link_x += zui_nodes_NODE_W(node);
				link_y += zui_nodes_OUTPUT_Y(node.outputs, link_drag.from_socket);
			}
			else {
				link_y += zui_nodes_INPUT_Y(UINodes.get_canvas(true), node.inputs, link_drag.to_socket) + zui_nodes_OUTPUTS_H(node.outputs) + zui_nodes_BUTTONS_H(node);
			}
			if (Math.abs(mouse_x - link_x) > 5 || Math.abs(mouse_y - link_y) > 5) { // Link length
				UINodes.node_search(-1, -1, () => {
					let n: zui_node_t = zui_get_node(UINodes.get_canvas(true).nodes, nodes.nodes_selected_id[0]);
					if (link_drag.to_id == -1 && n.inputs.length > 0) {
						link_drag.to_id = n.id;
						let from_type: string = node.outputs[link_drag.from_socket].type;
						// Connect to the first socket
						link_drag.to_socket = 0;
						// Try to find the first type-matching socket and use it if present
						for (let socket of n.inputs) {
							if (socket.type == from_type) {
								link_drag.to_socket = n.inputs.indexOf(socket);
								break;
							}
						}
						UINodes.get_canvas(true).links.push(link_drag);
					}
					else if (link_drag.from_id == -1 && n.outputs.length > 0) {
						link_drag.from_id = n.id;
						link_drag.from_socket = 0;
						UINodes.get_canvas(true).links.push(link_drag);
					}
					///if is_lab
					ParserLogic.parse(UINodes.get_canvas(true));
					Context.raw.rdirty = 5;
					///end
				});
			}
			// Selecting which node socket to preview
			else if (node.id == nodes.nodes_selected_id[0]) {
				Context.raw.node_preview_socket = link_drag.from_id > -1 ? link_drag.from_socket : 0;
				///if (is_paint || is_sculpt)
				Context.raw.node_preview_dirty = true;
				///end
			}
		}
	}

	static on_socket_released = (socket_id: i32) => {
		let nodes: zui_nodes_t = UINodes.get_nodes();
		let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
		let socket: zui_node_socket_t = zui_get_socket(canvas.nodes, socket_id);
		let node: zui_node_t = zui_get_node(canvas.nodes, socket.node_id);
		if (UINodes.ui.input_released_r) {
			if (node.type == "GROUP_INPUT" || node.type == "GROUP_OUTPUT") {
				base_notify_on_next_frame(() => {
					UIMenu.draw((ui: zui_t) => {
						if (UIMenu.menu_button(ui, tr("Edit"))) {
							let htype: zui_handle_t = zui_handle("uinodes_0");
							let hname: zui_handle_t = zui_handle("uinodes_1");
							let hmin: zui_handle_t = zui_handle("uinodes_2");
							let hmax: zui_handle_t = zui_handle("uinodes_3");
							let hval0: zui_handle_t = zui_handle("uinodes_4");
							let hval1: zui_handle_t = zui_handle("uinodes_5");
							let hval2: zui_handle_t = zui_handle("uinodes_6");
							let hval3: zui_handle_t = zui_handle("uinodes_7");
							htype.position = socket.type == "RGBA" ? 0 : socket.type == "VECTOR" ? 1 : 2;
							hname.text = socket.name;
							hmin.value = socket.min;
							hmax.value = socket.max;
							if (socket.type == "RGBA" || socket.type == "VECTOR") {
								hval0.value = socket.default_value[0];
								hval1.value = socket.default_value[1];
								hval2.value = socket.default_value[2];
								if (socket.type == "RGBA") {
									hval3.value = socket.default_value[3];
								}
							}
							else hval0.value = socket.default_value;
							base_notify_on_next_frame(() => {
								zui_end_input();
								UIBox.show_custom((ui: zui_t) => {
									if (zui_tab(zui_handle("uinodes_8"), tr("Socket"))) {
										let type: i32 = zui_combo(htype, [tr("Color"), tr("Vector"), tr("Value")], tr("Type"), true);
										if (htype.changed) hname.text = type == 0 ? tr("Color") : type == 1 ? tr("Vector") : tr("Value");
										let name: string = zui_text_input(hname, tr("Name"));
										let min: f32 = zui_float_input(hmin, tr("Min"));
										let max: f32 = zui_float_input(hmax, tr("Max"));
										let default_value: any = null;
										if (type == 0) {
											zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
											zui_float_input(hval0, tr("R"));
											zui_float_input(hval1, tr("G"));
											zui_float_input(hval2, tr("B"));
											zui_float_input(hval3, tr("A"));
											default_value = new Float32Array([hval0.value, hval1.value, hval2.value, hval3.value]);
										}
										else if (type == 1) {
											zui_row([1 / 3, 1 / 3, 1 / 3]);
											hval0.value = zui_float_input(hval0, tr("X"));
											hval1.value = zui_float_input(hval1, tr("Y"));
											hval2.value = zui_float_input(hval2, tr("Z"));
											default_value = new Float32Array([hval0.value, hval1.value, hval2.value]);
										}
										else {
											default_value = zui_float_input(hval0, tr("default_value"));
										}
										if (zui_button(tr("OK"))) { // || ui.isReturnDown
											socket.name = name;
											socket.type = type == 0 ? "RGBA" : type == 1 ? "VECTOR" : "VALUE";
											socket.color = NodesMaterial.get_socket_color(socket.type);
											socket.min = min;
											socket.max = max;
											socket.default_value = default_value;
											UIBox.hide();
											NodesMaterial.sync_sockets(node);
											UINodes.hwnd.redraws = 2;
										}
									}
								}, 400, 250);
							});
						}
						if (UIMenu.menu_button(ui, tr("Delete"))) {
							let i: i32 = 0;
							// Remove links connected to the socket
							while (i < canvas.links.length) {
								let l: zui_node_link_t = canvas.links[i];
								if ((l.from_id == node.id && l.from_socket == node.outputs.indexOf(socket)) ||
									(l.to_id == node.id && l.to_socket == node.inputs.indexOf(socket))) {
									canvas.links.splice(i, 1);
								}
								else i++;
							}
							// Remove socket
							array_remove(node.inputs, socket);
							array_remove(node.outputs, socket);
							NodesMaterial.sync_sockets(node);
						}
					}, 2);
				});
			}
			else UINodes.on_canvas_released();
		}
		// Selecting which node socket to preview
		else if (node.id == nodes.nodes_selected_id[0]) {
			let i: i32 = node.outputs.indexOf(socket);
			if (i > -1) {
				Context.raw.node_preview_socket = i;
				///if (is_paint || is_sculpt)
				Context.raw.node_preview_dirty = true;
				///end
			}
		}
	}

	static on_canvas_released = () => {
		if (UINodes.ui.input_released_r && Math.abs(UINodes.ui.input_x - UINodes.ui.input_started_x) < 2 && Math.abs(UINodes.ui.input_y - UINodes.ui.input_started_y) < 2) {
			// Node selection
			let nodes: zui_nodes_t = UINodes.get_nodes();
			let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
			let selected: zui_node_t = null;
			for (let node of canvas.nodes) {
				if (zui_get_input_in_rect(UINodes.ui._window_x + zui_nodes_NODE_X(node), UINodes.ui._window_y + zui_nodes_NODE_Y(node), zui_nodes_NODE_W(node), zui_nodes_NODE_H(canvas, node))) {
					selected = node;
					break;
				}
			}
			if (selected == null) nodes.nodes_selected_id = [];
			else if (nodes.nodes_selected_id.indexOf(selected.id) == -1) nodes.nodes_selected_id = [selected.id];

			// Node context menu
			if (!zui_socket_released()) {
				let number_of_entries: i32 = 5;
				if (UINodes.canvas_type == canvas_type_t.MATERIAL) ++number_of_entries;
				if (selected != null && selected.type == "RGB") ++number_of_entries;

				UIMenu.draw((uiMenu: zui_t) => {
					uiMenu._y += 1;
					let is_protected: bool = selected == null ||
									///if (is_paint || is_sculpt)
									selected.type == "OUTPUT_MATERIAL_PBR" ||
									///end
									selected.type == "GROUP_INPUT" ||
									selected.type == "GROUP_OUTPUT" ||
									selected.type == "BrushOutputNode";
					uiMenu.enabled = !is_protected;
					if (UIMenu.menu_button(uiMenu, tr("Cut"), "ctrl+x")) {
						base_notify_on_next_frame(() => {
							UINodes.hwnd.redraws = 2;
							zui_set_is_copy(true);
							zui_set_is_cut(true);
							UINodes.is_node_menu_op = true;
						});
					}
					if (UIMenu.menu_button(uiMenu, tr("Copy"), "ctrl+c")) {
						base_notify_on_next_frame(() => {
							zui_set_is_copy(true);
							UINodes.is_node_menu_op = true;
						});
					}
					uiMenu.enabled = zui_clipboard != "";
					if (UIMenu.menu_button(uiMenu, tr("Paste"), "ctrl+v")) {
						base_notify_on_next_frame(() => {
							UINodes.hwnd.redraws = 2;
							zui_set_is_paste(true);
							UINodes.is_node_menu_op = true;
						});
					}
					uiMenu.enabled = !is_protected;
					if (UIMenu.menu_button(uiMenu, tr("Delete"), "delete")) {
						base_notify_on_next_frame(() => {
							UINodes.hwnd.redraws = 2;
							UINodes.ui.is_delete_down = true;
							UINodes.is_node_menu_op = true;
						});
					}
					if (UIMenu.menu_button(uiMenu, tr("Duplicate"))) {
						base_notify_on_next_frame(() => {
							UINodes.hwnd.redraws = 2;
							zui_set_is_copy(true);
							zui_set_is_paste(true);
							UINodes.is_node_menu_op = true;
						});
					}
					if (selected != null && selected.type == "RGB") {
						if (UIMenu.menu_button(uiMenu, tr("Add Swatch"))) {
							let color: any = selected.outputs[0].default_value;
							let new_swatch: swatch_color_t = Project.make_swatch(color_from_floats(color[0], color[1], color[2], color[3]));
							Context.set_swatch(new_swatch);
							Project.raw.swatches.push(new_swatch);
							UIBase.hwnds[tab_area_t.STATUS].redraws = 1;
						}
					}

					if (UINodes.canvas_type == canvas_type_t.MATERIAL) {
						UIMenu.menu_separator(uiMenu);
						if (UIMenu.menu_button(uiMenu, tr("2D View"))) {
							UIBase.show_2d_view(view_2d_type_t.NODE);
						}
					}

					uiMenu.enabled = true;
				}, number_of_entries);
			}
		}

		if (UINodes.ui.input_released) {
			let nodes: zui_nodes_t = UINodes.get_nodes();
			let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
			for (let node of canvas.nodes) {
				if (zui_get_input_in_rect(UINodes.ui._window_x + zui_nodes_NODE_X(node), UINodes.ui._window_y + zui_nodes_NODE_Y(node), zui_nodes_NODE_W(node), zui_nodes_NODE_H(canvas, node))) {
					if (node.id == nodes.nodes_selected_id[0]) {
						UIView2D.hwnd.redraws = 2;
						if (time_time() - Context.raw.select_time < 0.25) UIBase.show_2d_view(view_2d_type_t.NODE);
						Context.raw.select_time = time_time();
					}
					break;
				}
			}
		}
	}

	static on_canvas_control = (): zui_canvas_control_t => {
		return UINodes.get_canvas_control(UINodes.ui, UINodes);
	}

	static get_canvas_control = (ui: zui_t, parent: any): zui_canvas_control_t => {
		if (Config.raw.wrap_mouse && parent.controlsDown) {
			if (ui.input_x < ui._window_x) {
				ui.input_x = ui._window_x + ui._window_w;
				krom_set_mouse_position(Math.floor(ui.input_x), Math.floor(ui.input_y));
			}
			else if (ui.input_x > ui._window_x + ui._window_w) {
				ui.input_x = ui._window_x;
				krom_set_mouse_position(Math.floor(ui.input_x), Math.floor(ui.input_y));
			}
			else if (ui.input_y < ui._window_y) {
				ui.input_y = ui._window_y + ui._window_h;
				krom_set_mouse_position(Math.floor(ui.input_x), Math.floor(ui.input_y));
			}
			else if (ui.input_y > ui._window_y + ui._window_h) {
				ui.input_y = ui._window_y;
				krom_set_mouse_position(Math.floor(ui.input_x), Math.floor(ui.input_y));
			}
		}

		if (Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutStarted) ||
			Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutStarted) ||
			ui.input_started_r ||
			ui.input_wheel_delta != 0.0) {
			parent.controlsDown = true;
		}
		else if (!Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown) &&
			!Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown) &&
			!ui.input_down_r &&
			ui.input_wheel_delta == 0.0) {
			parent.controlsDown = false;
		}
		if (!parent.controlsDown) {
			return {
				pan_x: 0,
				pan_y: 0,
				zoom: 0
			}
		}

		let pan: bool = ui.input_down_r || Operator.shortcut(Config.keymap.action_pan, ShortcutType.ShortcutDown);
		let zoom_delta: f32 = Operator.shortcut(Config.keymap.action_zoom, ShortcutType.ShortcutDown) ? UINodes.get_zoom_delta(ui) / 100.0 : 0.0;
		let control: any = {
			pan_x: pan ? ui.input_dx : 0.0,
			pan_y: pan ? ui.input_dy : 0.0,
			zoom: ui.input_wheel_delta != 0.0 ? -ui.input_wheel_delta / 10 : zoom_delta
		};
		if (base_is_combo_selected()) control.zoom = 0.0;
		return control;
	}

	static get_zoom_delta = (ui: zui_t): f32 => {
		return Config.raw.zoom_direction == zoom_direction_t.VERTICAL ? -ui.input_dy :
			   Config.raw.zoom_direction == zoom_direction_t.VERTICAL_INVERTED ? -ui.input_dy :
			   Config.raw.zoom_direction == zoom_direction_t.HORIZONTAL ? ui.input_dx :
			   Config.raw.zoom_direction == zoom_direction_t.HORIZONTAL_INVERTED ? ui.input_dx :
			   -(ui.input_dy - ui.input_dx);
	}

	static get_canvas = (groups: bool = false): zui_node_canvas_t => {
		///if (is_paint || is_sculpt)
		if (UINodes.canvas_type == canvas_type_t.MATERIAL) {
			if (groups && UINodes.group_stack.length > 0) return UINodes.group_stack[UINodes.group_stack.length - 1].canvas;
			else return UINodes.get_canvas_material();
		}
		else return Context.raw.brush.canvas;
		///end

		///if is_lab
		return Project.canvas;
		///end
	}

	///if (is_paint || is_sculpt)
	static get_canvas_material = (): zui_node_canvas_t => {
		return Context.raw.material.canvas;
	}
	///end

	static get_nodes = (): zui_nodes_t => {
		///if (is_paint || is_sculpt)
		if (UINodes.canvas_type == canvas_type_t.MATERIAL) {
			if (UINodes.group_stack.length > 0) return UINodes.group_stack[UINodes.group_stack.length - 1].nodes;
			else return Context.raw.material.nodes;
		}
		else return Context.raw.brush.nodes;
		///end

		///if is_lab
		if (UINodes.group_stack.length > 0) return UINodes.group_stack[UINodes.group_stack.length - 1].nodes;
		else return Project.nodes;
		///end
	}

	static update = () => {
		if (!UINodes.show || !base_ui_enabled) return;

		///if (is_paint || is_sculpt)
		UINodes.wx = Math.floor(app_w()) + UIToolbar.toolbar_w;
		///end
		///if is_lab
		UINodes.wx = Math.floor(app_w());
		///end
		UINodes.wy = UIHeader.headerh * 2;

		if (UIView2D.show) {
			UINodes.wy += app_h() - Config.raw.layout[layout_size_t.NODES_H];
		}

		let ww: i32 = Config.raw.layout[layout_size_t.NODES_W];
		if (!UIBase.show) {
			///if (is_paint || is_sculpt)
			ww += Config.raw.layout[layout_size_t.SIDEBAR_W] + UIToolbar.toolbar_w;
			UINodes.wx -= UIToolbar.toolbar_w;
			///end
			UINodes.wy = 0;
		}

		let mx: i32 = mouse_x;
		let my: i32 = mouse_y;
		if (mx < UINodes.wx || mx > UINodes.wx + ww || my < UINodes.wy) return;
		if (UINodes.ui.is_typing || !UINodes.ui.input_enabled) return;

		let nodes: zui_nodes_t = UINodes.get_nodes();
		if (nodes.nodes_selected_id.length > 0 && UINodes.ui.is_key_pressed) {
			if (UINodes.ui.key == key_code_t.LEFT) for (let n of nodes.nodes_selected_id) zui_get_node(UINodes.get_canvas(true).nodes, n).x -= 1;
			else if (UINodes.ui.key == key_code_t.RIGHT) for (let n of nodes.nodes_selected_id) zui_get_node(UINodes.get_canvas(true).nodes, n).x += 1;
			if (UINodes.ui.key == key_code_t.UP) for (let n of nodes.nodes_selected_id) zui_get_node(UINodes.get_canvas(true).nodes, n).y -= 1;
			else if (UINodes.ui.key == key_code_t.DOWN) for (let n of nodes.nodes_selected_id) zui_get_node(UINodes.get_canvas(true).nodes, n).y += 1;
		}

		// Node search popup
		if (Operator.shortcut(Config.keymap.node_search)) UINodes.node_search();
		if (UINodes.node_search_spawn != null) {
			UINodes.ui.input_x = mouse_x; // Fix inputDX after popup removal
			UINodes.ui.input_y = mouse_y;
			UINodes.node_search_spawn = null;
		}

		if (Operator.shortcut(Config.keymap.view_reset)) {
			nodes.panX = 0.0;
			nodes.panY = 0.0;
			nodes.zoom = 1.0;
		}
	}

	static canvas_changed = () => {
		UINodes.recompile_mat = true;
		UINodes.recompile_mat_final = true;
	}

	static node_search = (x: i32 = -1, y: i32 = -1, done: ()=>void = null) => {
		let search_handle: zui_handle_t = zui_handle("uinodes_9");
		let first: bool = true;
		UIMenu.draw((ui: zui_t) => {
			g2_set_color(ui.t.SEPARATOR_COL);
			zui_draw_rect(true, ui._x, ui._y, ui._w, zui_ELEMENT_H(ui) * 8);
			g2_set_color(0xffffffff);

			let search: string = zui_text_input(search_handle, "", zui_align_t.LEFT, true, true).toLowerCase();
			ui.changed = false;
			if (first) {
				first = false;
				search_handle.text = "";
				zui_start_text_edit(search_handle); // Focus search bar
			}

			if (search_handle.changed) UINodes.node_search_offset = 0;

			if (ui.is_key_pressed) { // Move selection
				if (ui.key == key_code_t.DOWN && UINodes.node_search_offset < 6) UINodes.node_search_offset++;
				if (ui.key == key_code_t.UP && UINodes.node_search_offset > 0) UINodes.node_search_offset--;
			}
			let enter: bool = keyboard_down("enter");
			let count: i32 = 0;
			let BUTTON_COL: i32 = ui.t.BUTTON_COL;

			///if (is_paint || is_sculpt)
			let node_list: zui_node_t[][] = UINodes.canvas_type == canvas_type_t.MATERIAL ? NodesMaterial.list : NodesBrush.list;
			///end
			///if is_lab
			let node_list: zui_node_t[][] = NodesBrush.list;
			///end

			for (let list of node_list) {
				for (let n of list) {
					if (tr(n.name).toLowerCase().indexOf(search) >= 0) {
						ui.t.BUTTON_COL = count == UINodes.node_search_offset ? ui.t.HIGHLIGHT_COL : ui.t.SEPARATOR_COL;
						if (zui_button(tr(n.name), zui_align_t.LEFT) || (enter && count == UINodes.node_search_offset)) {
							UINodes.push_undo();
							let nodes: zui_nodes_t = UINodes.get_nodes();
							let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
							UINodes.node_search_spawn = UINodes.make_node(n, nodes, canvas); // Spawn selected node
							canvas.nodes.push(UINodes.node_search_spawn);
							nodes.nodes_selected_id = [UINodes.node_search_spawn.id];
							nodes.nodesDrag = true;

							///if is_lab
							ParserLogic.parse(canvas);
							///end

							UINodes.hwnd.redraws = 2;
							if (enter) {
								ui.changed = true;
								count = 6; // Trigger break
							}
							if (done != null) done();
						}
						if (++count > 6) break;
					}
				}
				if (count > 6) break;
			}
			if (enter && count == 0) { // Hide popup on enter when node is not found
				ui.changed = true;
				search_handle.text = "";
			}
			ui.t.BUTTON_COL = BUTTON_COL;
		}, 8, x, y);
	}

	static get_node_x = (): i32 => {
		return Math.floor((mouse_x - UINodes.wx - zui_nodes_PAN_X()) / zui_nodes_SCALE());
	}

	static get_node_y = (): i32 => {
		return Math.floor((mouse_y - UINodes.wy - zui_nodes_PAN_Y()) / zui_nodes_SCALE());
	}

	static draw_grid = () => {
		let ww: i32 = Config.raw.layout[layout_size_t.NODES_W];

		///if (is_paint || is_sculpt)
		if (!UIBase.show) {
			ww += Config.raw.layout[layout_size_t.SIDEBAR_W] + UIToolbar.toolbar_w;
		}
		///end

		let wh: i32 = app_h();
		let step: f32 = 100 * zui_SCALE(UINodes.ui);
		let w: i32 = Math.floor(ww + step * 3);
		let h: i32 = Math.floor(wh + step * 3);
		if (w < 1) w = 1;
		if (h < 1) h = 1;
		UINodes.grid = image_create_render_target(w, h);
		g2_begin(UINodes.grid);
		g2_clear(UINodes.ui.t.SEPARATOR_COL);

		g2_set_color(UINodes.ui.t.SEPARATOR_COL - 0x00050505);
		step = 20 * zui_SCALE(UINodes.ui);
		for (let i: i32 = 0; i < Math.floor(h / step) + 1; ++i) {
			g2_draw_line(0, i * step, w, i * step);
		}
		for (let i: i32 = 0; i < Math.floor(w / step) + 1; ++i) {
			g2_draw_line(i * step, 0, i * step, h);
		}

		g2_set_color(UINodes.ui.t.SEPARATOR_COL - 0x00090909);
		step = 100 * zui_SCALE(UINodes.ui);
		for (let i: i32 = 0; i < Math.floor(h / step) + 1; ++i) {
			g2_draw_line(0, i * step, w, i * step);
		}
		for (let i: i32 = 0; i < Math.floor(w / step) + 1; ++i) {
			g2_draw_line(i * step, 0, i * step, h);
		}

		g2_end();
	}

	static render = () => {
		if (UINodes.recompile_mat) {
			///if (is_paint || is_sculpt)
			if (UINodes.canvas_type == canvas_type_t.BRUSH) {
				MakeMaterial.parse_brush();
				UtilRender.make_brush_preview();
				UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
			}
			else {
				base_is_fill_material() ? base_update_fill_layers() : UtilRender.make_material_preview();
				if (UIView2D.show && UIView2D.type == view_2d_type_t.NODE) {
					UIView2D.hwnd.redraws = 2;
				}
			}

			UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
			if (Context.raw.split_view) Context.raw.ddirty = 2;
			///end

			///if is_lab
			ParserLogic.parse(Project.canvas);
			///end

			UINodes.recompile_mat = false;
		}
		else if (UINodes.recompile_mat_final) {
			///if (is_paint || is_sculpt)
			MakeMaterial.parse_paint_material();

			if (UINodes.canvas_type == canvas_type_t.MATERIAL && base_is_fill_material()) {
				base_update_fill_layers();
				UtilRender.make_material_preview();
			}

			let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
			if (decal) UtilRender.make_decal_preview();

			UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
			Context.raw.node_preview_dirty = true;
			///end

			UINodes.recompile_mat_final = false;
		}

		let nodes: zui_nodes_t = UINodes.get_nodes();
		if (nodes.nodes_selected_id.length > 0 && nodes.nodes_selected_id[0] != UINodes.last_node_selected_id) {
			UINodes.last_node_selected_id = nodes.nodes_selected_id[0];
			///if (is_paint || is_sculpt)
			Context.raw.node_preview_dirty = true;
			///end

			///if is_lab
			Context.raw.ddirty = 2; // Show selected node texture in viewport
			UIHeader.header_handle.redraws = 2;
			///end

			Context.raw.node_preview_socket = 0;
		}

		// Remove dragged link when mouse is released out of the node viewport
		let c: zui_node_canvas_t = UINodes.get_canvas(true);
		if (UINodes.release_link && nodes.linkDragId != -1) {
			array_remove(c.links, zui_get_link(c.links, nodes.linkDragId));
			nodes.linkDragId = -1;
		}
		UINodes.release_link = UINodes.ui.input_released;

		if (!UINodes.show || sys_width() == 0 || sys_height() == 0) return;

		UINodes.ui.input_enabled = base_ui_enabled;

		g2_end();

		if (UINodes.grid == null) UINodes.draw_grid();

		///if (is_paint || is_sculpt)
		if (Config.raw.node_preview && Context.raw.node_preview_dirty) {
			UINodes.make_node_preview();
		}
		///end

		// Start with UI
		zui_begin(UINodes.ui);

		// Make window
		UINodes.ww = Config.raw.layout[layout_size_t.NODES_W];

		///if (is_paint || is_sculpt)
		UINodes.wx = Math.floor(app_w()) + UIToolbar.toolbar_w;
		///end
		///if is_lab
		UINodes.wx = Math.floor(app_w());
		///end

		UINodes.wy = 0;

		///if (is_paint || is_sculpt)
		if (!UIBase.show) {
			UINodes.ww += Config.raw.layout[layout_size_t.SIDEBAR_W] + UIToolbar.toolbar_w;
			UINodes.wx -= UIToolbar.toolbar_w;
		}
		///end

		let ew: i32 = Math.floor(zui_ELEMENT_W(UINodes.ui) * 0.7);
		UINodes.wh = app_h() + UIHeader.headerh;
		if (Config.raw.layout[layout_size_t.HEADER] == 1) UINodes.wh += UIHeader.headerh;

		if (UIView2D.show) {
			UINodes.wh = Config.raw.layout[layout_size_t.NODES_H];
			UINodes.wy = app_h() - Config.raw.layout[layout_size_t.NODES_H] + UIHeader.headerh;
			if (Config.raw.layout[layout_size_t.HEADER] == 1) UINodes.wy += UIHeader.headerh;
			if (!UIBase.show) {
				UINodes.wy -= UIHeader.headerh * 2;
			}
		}

		if (zui_window(UINodes.hwnd, UINodes.wx, UINodes.wy, UINodes.ww, UINodes.wh)) {

			zui_tab(zui_handle("uinodes_10"), tr("Nodes"));

			// Grid
			g2_set_color(0xffffffff);
			let step: f32 = 100 * zui_SCALE(UINodes.ui);
			g2_draw_image(UINodes.grid, (nodes.panX * zui_nodes_SCALE()) % step - step, (nodes.panY * zui_nodes_SCALE()) % step - step);

			// Undo
			if (UINodes.ui.input_started || UINodes.ui.is_key_pressed) {
				UINodes.last_canvas = JSON.parse(JSON.stringify(UINodes.get_canvas(true)));
			}

			// Nodes
			let _input_enabled: bool = UINodes.ui.input_enabled;
			UINodes.ui.input_enabled = _input_enabled && !UINodes.show_menu;
			///if (is_paint || is_sculpt)
			UINodes.ui.window_border_right = Config.raw.layout[layout_size_t.SIDEBAR_W];
			///end
			UINodes.ui.window_border_top = UIHeader.headerh * 2;
			UINodes.ui.window_border_bottom = Config.raw.layout[layout_size_t.STATUS_H];
			zui_node_canvas(nodes, UINodes.ui, c);
			UINodes.ui.input_enabled = _input_enabled;

			if (nodes.colorPickerCallback != null) {
				Context.raw.color_picker_previous_tool = Context.raw.tool;
				Context.select_tool(workspace_tool_t.PICKER);
				let tmp: (col: i32)=>void = nodes.colorPickerCallback;
				Context.raw.color_picker_callback = (color: swatch_color_t) => {
					tmp(color.base);
					UINodes.hwnd.redraws = 2;

					///if (is_paint || is_sculpt)
					let material_live: bool = Config.raw.material_live;
					///end
					///if is_lab
					let material_live: bool = true;
					///end

					if (material_live) {
						UINodes.canvas_changed();
					}
				};
				nodes.colorPickerCallback = null;
			}

			// Remove nodes with unknown id for this canvas type
			if (zui_is_paste) {
				///if (is_paint || is_sculpt)
				let node_list: zui_node_t[][] = UINodes.canvas_type == canvas_type_t.MATERIAL ? NodesMaterial.list : NodesBrush.list;
				///end
				///if is_lab
				let node_list: zui_node_t[][] = NodesBrush.list;
				///end

				let i: i32 = 0;
				while (i++ < c.nodes.length) {
					let canvas_node: zui_node_t = c.nodes[i - 1];
					if (zui_exclude_remove.indexOf(canvas_node.type) >= 0) {
						continue;
					}
					let found: bool = false;
					for (let list of node_list) {
						for (let list_node of list) {
							if (canvas_node.type == list_node.type) {
								found = true;
								break;
							}
						}
						if (found) break;
					}
					if (canvas_node.type == "GROUP" && !UINodes.can_place_group(canvas_node.name)) {
						found = false;
					}
					if (!found) {
						zui_remove_node(canvas_node, c);
						array_remove(nodes.nodes_selected_id, canvas_node.id);
						i--;
					}
				}
			}

			if (UINodes.is_node_menu_op) {
				zui_set_is_copy(false);
				zui_set_is_cut(false);
				zui_set_is_paste(false);
				UINodes.ui.is_delete_down = false;
			}

			// Recompile material on change
			if (UINodes.ui.changed) {
				///if (is_paint || is_sculpt)
				UINodes.recompile_mat = (UINodes.ui.input_dx != 0 || UINodes.ui.input_dy != 0 || !UINodes.uichanged_last) && Config.raw.material_live; // Instant preview
				///end
				///if is_lab
				UINodes.recompile_mat = (UINodes.ui.input_dx != 0 || UINodes.ui.input_dy != 0 || !UINodes.uichanged_last); // Instant preview
				///end
			}
			else if (UINodes.uichanged_last) {
				UINodes.canvas_changed();
				UINodes.push_undo(UINodes.last_canvas);
			}
			UINodes.uichanged_last = UINodes.ui.changed;

			// Node previews
			if (Config.raw.node_preview && nodes.nodes_selected_id.length > 0) {
				let img: image_t = null;
				let sel: zui_node_t = zui_get_node(c.nodes, nodes.nodes_selected_id[0]);

				///if (is_paint || is_sculpt)

				let single_channel: bool = sel.type == "LAYER_MASK";
				if (sel.type == "LAYER" || sel.type == "LAYER_MASK") {
					let id: any = sel.buttons[0].default_value;
					if (id < Project.layers.length) {
						///if is_paint
						img = Project.layers[id].texpaint_preview;
						///end
					}
				}
				else if (sel.type == "MATERIAL") {
					let id: any = sel.buttons[0].default_value;
					if (id < Project.materials.length) {
						img = Project.materials[id].image;
					}
				}
				else if (sel.type == "OUTPUT_MATERIAL_PBR") {
					img = Context.raw.material.image;
				}
				else if (sel.type == "BrushOutputNode") {
					img = Context.raw.brush.image;
				}
				else if (UINodes.canvas_type == canvas_type_t.MATERIAL) {
					img = Context.raw.node_preview;
				}

				///else

				let brush_node: LogicNode = ParserLogic.get_logic_node(sel);
				if (brush_node != null) {
					img = brush_node.get_cached_image();
				}

				///end

				if (img != null) {
					let tw: f32 = 128 * zui_SCALE(UINodes.ui);
					let th: f32 = tw * (img.height / img.width);
					let tx: f32 = UINodes.ww - tw - 8 * zui_SCALE(UINodes.ui);
					let ty: f32 = UINodes.wh - th - 8 * zui_SCALE(UINodes.ui);

					///if krom_opengl
					let invert_y: bool = sel.type == "MATERIAL";
					///else
					let invert_y: bool = false;
					///end

					///if (is_paint || is_sculpt)
					if (single_channel) {
						g2_set_pipeline(UIView2D.pipe);
						///if krom_opengl
						krom_g4_set_pipeline(UIView2D.pipe.pipeline_);
						///end
						krom_g4_set_int(UIView2D.channel_location, 1);
					}
					///end

					g2_set_color(0xffffffff);
					invert_y ?
						g2_draw_scaled_image(img, tx, ty + th, tw, -th) :
						g2_draw_scaled_image(img, tx, ty, tw, th);

					///if (is_paint || is_sculpt)
					if (single_channel) {
						g2_set_pipeline(null);
					}
					///end
				}
			}

			// Menu
			g2_set_color(UINodes.ui.t.SEPARATOR_COL);
			g2_fill_rect(0, zui_ELEMENT_H(UINodes.ui), UINodes.ww, zui_ELEMENT_H(UINodes.ui) + zui_ELEMENT_OFFSET(UINodes.ui) * 2);
			g2_set_color(0xffffffff);

			let start_y: i32 = zui_ELEMENT_H(UINodes.ui) + zui_ELEMENT_OFFSET(UINodes.ui);
			UINodes.ui._x = 0;
			UINodes.ui._y = 2 + start_y;
			UINodes.ui._w = ew;

			///if (is_paint || is_sculpt)
			// Editable canvas name
			let h: zui_handle_t = zui_handle("uinodes_11");
			h.text = c.name;
			UINodes.ui._w = Math.floor(Math.min(g2_font_width(UINodes.ui.font, UINodes.ui.font_size, h.text) + 15 * zui_SCALE(UINodes.ui), 100 * zui_SCALE(UINodes.ui)));
			let new_name: string = zui_text_input(h, "");
			UINodes.ui._x += UINodes.ui._w + 3;
			UINodes.ui._y = 2 + start_y;
			UINodes.ui._w = ew;

			if (h.changed) { // Check whether renaming is possible and update group links
				if (UINodes.group_stack.length > 0) {
					let can_rename: bool = true;
					for (let m of Project.material_groups) {
						if (m.canvas.name == new_name) can_rename = false; // Name already used
					}

					if (can_rename) {
						let old_name: string = c.name;
						c.name = new_name;
						let canvases: zui_node_canvas_t[] = [];
						for (let m of Project.materials) canvases.push(m.canvas);
						for (let m of Project.material_groups) canvases.push(m.canvas);
						for (let canvas of canvases) {
							for (let n of canvas.nodes) {
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
			///end

			///if is_lab
			UINodes.ui.window_border_top = 0;
			UINodesExt.drawButtons(ew, start_y);
			///end

			let _BUTTON_COL: i32 = UINodes.ui.t.BUTTON_COL;
			UINodes.ui.t.BUTTON_COL = UINodes.ui.t.SEPARATOR_COL;

			///if (is_paint || is_sculpt)
			let cats: string[] = UINodes.canvas_type == canvas_type_t.MATERIAL ? NodesMaterial.categories : NodesBrush.categories;
			///end
			///if is_lab
			let cats: string[] = NodesBrush.categories;
			///end

			for (let i: i32 = 0; i < cats.length; ++i) {
				if ((zui_menu_button(tr(cats[i]))) || (UINodes.ui.is_hovered && UINodes.show_menu)) {
					UINodes.show_menu = true;
					UINodes.menu_category = i;
					UINodes.popup_x = UINodes.wx + UINodes.ui._x;
					UINodes.popup_y = UINodes.wy + UINodes.ui._y;
					if (Config.raw.touch_ui) {
						UINodes.show_menu_first = true;
						let menuw: i32 = Math.floor(ew * 2.3);
						UINodes.popup_x -= menuw / 2;
						UINodes.popup_x += UINodes.ui._w / 2;
					}
					UIMenu.menu_category_w = UINodes.ui._w;
					UIMenu.menu_category_h = Math.floor(zui_MENUBAR_H(UINodes.ui));
				}
				UINodes.ui._x += UINodes.ui._w + 3;
				UINodes.ui._y = 2 + start_y;
			}

			if (Config.raw.touch_ui) {
				let _w: i32 = UINodes.ui._w;
				UINodes.ui._w = Math.floor(36 * zui_SCALE(UINodes.ui));
				UINodes.ui._y = 4 * zui_SCALE(UINodes.ui) + start_y;
				if (UIMenubar.icon_button(UINodes.ui, 2, 3)) {
					UINodes.node_search(Math.floor(UINodes.ui._window_x + UINodes.ui._x), Math.floor(UINodes.ui._window_y + UINodes.ui._y));
				}
				UINodes.ui._w = _w;
			}
			else {
				if (zui_menu_button(tr("Search"))) {
					UINodes.node_search(Math.floor(UINodes.ui._window_x + UINodes.ui._x), Math.floor(UINodes.ui._window_y + UINodes.ui._y));
				}
			}
			if (UINodes.ui.is_hovered) {
				zui_tooltip(tr("Search for nodes") + ` (${Config.keymap.node_search})`);
			}
			UINodes.ui._x += UINodes.ui._w + 3;
			UINodes.ui._y = 2 + start_y;

			UINodes.ui.t.BUTTON_COL = _BUTTON_COL;

			// Close node group
			if (UINodes.group_stack.length > 0 && zui_menu_button(tr("Close"))) {
				UINodes.group_stack.pop();
			}
		}

		zui_end(!UINodes.show_menu);

		g2_begin(null);

		if (UINodes.show_menu) {
			///if (is_paint || is_sculpt)
			let list:zui_node_t[][] = UINodes.canvas_type == canvas_type_t.MATERIAL ? NodesMaterial.list : NodesBrush.list;
			///end
			///if is_lab
			let list:zui_node_t[][] = NodesBrush.list;
			///end

			let num_nodes: i32 = list[UINodes.menu_category].length;

			///if (is_paint || is_sculpt)
			let is_group_category: bool = UINodes.canvas_type == canvas_type_t.MATERIAL && NodesMaterial.categories[UINodes.menu_category] == "Group";
			///end
			///if is_lab
			let is_group_category: bool = NodesMaterial.categories[UINodes.menu_category] == "Group";
			///end

			if (is_group_category) num_nodes += Project.material_groups.length;

			let py: i32 = UINodes.popup_y;
			let menuw: i32 = Math.floor(ew * 2.3);
			zui_begin_region(UINodes.ui, Math.floor(UINodes.popup_x), Math.floor(py), menuw);
			let _BUTTON_COL: i32 = UINodes.ui.t.BUTTON_COL;
			UINodes.ui.t.BUTTON_COL = UINodes.ui.t.SEPARATOR_COL;
			let _ELEMENT_OFFSET: i32 = UINodes.ui.t.ELEMENT_OFFSET;
			UINodes.ui.t.ELEMENT_OFFSET = 0;
			let _ELEMENT_H: i32 = UINodes.ui.t.ELEMENT_H;
			UINodes.ui.t.ELEMENT_H = Config.raw.touch_ui ? (28 + 2) : 28;

			UIMenu.menu_start(UINodes.ui);

			for (let n of list[UINodes.menu_category]) {
				if (UIMenu.menu_button(UINodes.ui, tr(n.name))) {
					UINodes.push_undo();
					let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
					let nodes: zui_nodes_t = UINodes.get_nodes();
					let node: zui_node_t = UINodes.make_node(n, nodes, canvas);
					canvas.nodes.push(node);
					nodes.nodes_selected_id = [node.id];
					nodes.nodesDrag = true;
					///if is_lab
					ParserLogic.parse(canvas);
					///end
				}
				// Next column
				if (UINodes.ui._y - UINodes.wy + zui_ELEMENT_H(UINodes.ui) / 2 > UINodes.wh) {
					UINodes.ui._x += menuw;
					UINodes.ui._y = py;
				}
			}
			if (is_group_category) {
				for (let g of Project.material_groups) {
					zui_fill(0, 1, UINodes.ui._w / zui_SCALE(UINodes.ui), UINodes.ui.t.BUTTON_H + 2, UINodes.ui.t.ACCENT_SELECT_COL);
					zui_fill(1, 1, UINodes.ui._w / zui_SCALE(UINodes.ui) - 2, UINodes.ui.t.BUTTON_H + 1, UINodes.ui.t.SEPARATOR_COL);
					UINodes.ui.enabled = UINodes.can_place_group(g.canvas.name);
					UIMenu.menu_fill(UINodes.ui);
					zui_row([5 / 6, 1 / 6]);
					if (zui_button(Config.button_spacing + g.canvas.name, zui_align_t.LEFT)) {
						UINodes.push_undo();
						let canvas: zui_node_canvas_t = UINodes.get_canvas(true);
						let nodes: zui_nodes_t = UINodes.get_nodes();
						let node: zui_node_t = UINodes.make_group_node(g.canvas, nodes, canvas);
						canvas.nodes.push(node);
						nodes.nodes_selected_id = [node.id];
						nodes.nodesDrag = true;
					}

					///if (is_paint || is_sculpt)
					UINodes.ui.enabled = !Project.is_material_group_in_use(g);
					if (zui_button("x", zui_align_t.CENTER)) {
						History.delete_material_group(g);
						array_remove(Project.material_groups, g);
					}
					///end

					UINodes.ui.enabled = true;
				}
			}

			UINodes.hide_menu = UINodes.ui.combo_selected_handle_ptr == 0 && !UINodes.show_menu_first && (UINodes.ui.changed || UINodes.ui.input_released || UINodes.ui.input_released_r || UINodes.ui.is_escape_down);
			UINodes.show_menu_first = false;

			UINodes.ui.t.BUTTON_COL = _BUTTON_COL;
			UINodes.ui.t.ELEMENT_OFFSET = _ELEMENT_OFFSET;
			UINodes.ui.t.ELEMENT_H = _ELEMENT_H;
			zui_end_region();
		}

		if (UINodes.hide_menu) {
			UINodes.show_menu = false;
			UINodes.show_menu_first = true;
		}
	}

	static contains_node_group_recursive = (group: node_group_t, groupName: string): bool => {
		if (group.canvas.name == groupName) {
			return true;
		}
		for (let n of group.canvas.nodes) {
			if (n.type == "GROUP") {
				let g: node_group_t = Project.get_material_group_by_name(n.name);
				if (g != null && UINodes.contains_node_group_recursive(g, groupName)) {
					return true;
				}
			}
		}
		return false;
	}

	static can_place_group = (groupName: string): bool => {
		// Prevent Recursive node groups
		// The group to place must not contain the current group or a group that contains the current group
		if (UINodes.group_stack.length > 0) {
			for (let g of UINodes.group_stack) {
				if (UINodes.contains_node_group_recursive(Project.get_material_group_by_name(groupName), g.canvas.name)) return false;
			}
		}
		// Group was deleted / renamed
		let group_exists: bool = false;
		for (let group of Project.material_groups) {
			if (groupName == group.canvas.name) {
				group_exists = true;
			}
		}
		if (!group_exists) return false;
		return true;
	}

	static push_undo = (last_canvas: zui_node_canvas_t = null) => {
		if (last_canvas == null) last_canvas = UINodes.get_canvas(true);
		let canvas_group: i32 = UINodes.group_stack.length > 0 ? Project.material_groups.indexOf(UINodes.group_stack[UINodes.group_stack.length - 1]) : null;

		///if (is_paint || is_sculpt)
		UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		History.edit_nodes(last_canvas, UINodes.canvas_type, canvas_group);
		///end
		///if is_lab
		History.edit_nodes(last_canvas, canvas_group);
		///end
	}

	static accept_asset_drag = (index: i32) => {
		UINodes.push_undo();
		let g: node_group_t = UINodes.group_stack.length > 0 ? UINodes.group_stack[UINodes.group_stack.length - 1] : null;
		///if (is_paint || is_sculpt)
		let n: zui_node_t = UINodes.canvas_type == canvas_type_t.MATERIAL ? NodesMaterial.create_node("TEX_IMAGE", g) : NodesBrush.create_node("TEX_IMAGE");
		///end
		///if is_lab
		let n: zui_node_t = NodesBrush.create_node("ImageTextureNode");
		///end

		n.buttons[0].default_value = index;
		UINodes.get_nodes().nodes_selected_id = [n.id];

		///if is_lab
		ParserLogic.parse(Project.canvas);
		///end
	}

	///if (is_paint || is_sculpt)
	static accept_layer_drag = (index: i32) => {
		UINodes.push_undo();
		if (SlotLayer.is_group(Project.layers[index])) return;
		let g: node_group_t = UINodes.group_stack.length > 0 ? UINodes.group_stack[UINodes.group_stack.length - 1] : null;
		let n: zui_node_t = NodesMaterial.create_node(SlotLayer.is_mask(Context.raw.layer) ? "LAYER_MASK" : "LAYER", g);
		n.buttons[0].default_value = index;
		UINodes.get_nodes().nodes_selected_id = [n.id];
	}

	static accept_material_drag = (index: i32) => {
		UINodes.push_undo();
		let g: node_group_t = UINodes.group_stack.length > 0 ? UINodes.group_stack[UINodes.group_stack.length - 1] : null;
		let n: zui_node_t = NodesMaterial.create_node("MATERIAL", g);
		n.buttons[0].default_value = index;
		UINodes.get_nodes().nodes_selected_id = [n.id];
	}
	///end

	static accept_swatch_drag = (swatch: swatch_color_t) => {
		///if (is_paint || is_sculpt)
		UINodes.push_undo();
		let g: node_group_t = UINodes.group_stack.length > 0 ? UINodes.group_stack[UINodes.group_stack.length - 1] : null;
		let n: zui_node_t = NodesMaterial.create_node("RGB", g);
		n.outputs[0].default_value = [
			color_get_rb(swatch.base) / 255,
			color_get_gb(swatch.base) / 255,
			color_get_bb(swatch.base) / 255,
			color_get_ab(swatch.base) / 255
		];
		UINodes.get_nodes().nodes_selected_id = [n.id];
		///end
	}

	static make_node = (n: zui_node_t, nodes: zui_nodes_t, canvas: zui_node_canvas_t): zui_node_t => {
		let node: zui_node_t = JSON.parse(JSON.stringify(n));
		node.id = zui_get_node_id(canvas.nodes);
		node.x = UINodes.get_node_x();
		node.y = UINodes.get_node_y();
		let count: i32 = 0;
		for (let soc of node.inputs) {
			soc.id = zui_get_socket_id(canvas.nodes) + count;
			soc.node_id = node.id;
			count++;
		}
		for (let soc of node.outputs) {
			soc.id = zui_get_socket_id(canvas.nodes) + count;
			soc.node_id = node.id;
			count++;
		}
		return node;
	}

	static make_group_node = (groupCanvas: zui_node_canvas_t, nodes: zui_nodes_t, canvas: zui_node_canvas_t): zui_node_t => {
		let n: zui_node_t = NodesMaterial.list[5][0];
		let node: zui_node_t = JSON.parse(JSON.stringify(n));
		node.name = groupCanvas.name;
		node.id = zui_get_node_id(canvas.nodes);
		node.x = UINodes.get_node_x();
		node.y = UINodes.get_node_y();
		let group_input: zui_node_t = null;
		let group_output: zui_node_t = null;
		for (let g of Project.material_groups) {
			if (g.canvas.name == node.name) {
				for (let n of g.canvas.nodes) {
					if (n.type == "GROUP_INPUT") group_input = n;
					else if (n.type == "GROUP_OUTPUT") group_output = n;
				}
				break;
			}
		}
		if (group_input != null && group_output != null) {
			for (let soc of group_input.outputs) {
				node.inputs.push(NodesMaterial.create_socket(nodes, node, soc.name, soc.type, canvas, soc.min, soc.max, soc.default_value));
			}
			for (let soc of group_output.inputs) {
				node.outputs.push(NodesMaterial.create_socket(nodes, node, soc.name, soc.type, canvas, soc.min, soc.max, soc.default_value));
			}
		}
		return node;
	}

	///if (is_paint || is_sculpt)
	static make_node_preview = () => {
		let nodes: zui_nodes_t = Context.raw.material.nodes;
		if (nodes.nodes_selected_id.length == 0) return;

		let node: zui_node_t = zui_get_node(Context.raw.material.canvas.nodes, nodes.nodes_selected_id[0]);
		// if (node == null) return;
		Context.raw.node_preview_name = node.name;

		if (node.type == "LAYER" ||
			node.type == "LAYER_MASK" ||
			node.type == "MATERIAL" ||
			node.type == "OUTPUT_MATERIAL_PBR") return;

		if (Context.raw.material.canvas.nodes.indexOf(node) == -1) return;

		if (Context.raw.node_preview == null) {
			Context.raw.node_preview = image_create_render_target(UtilRender.material_preview_size, UtilRender.material_preview_size);
		}

		Context.raw.node_preview_dirty = false;
		UINodes.hwnd.redraws = 2;
		UtilRender.make_node_preview(Context.raw.material.canvas, node, Context.raw.node_preview);
	}
	///end

	static has_group = (c: zui_node_canvas_t): bool => {
		for (let n of c.nodes) if (n.type == "GROUP") return true;
		return false;
	}

	static traverse_group = (mgroups: zui_node_canvas_t[], c: zui_node_canvas_t) => {
		for (let n of c.nodes) {
			if (n.type == "GROUP") {
				if (UINodes.get_group(mgroups, n.name) == null) {
					let canvases: zui_node_canvas_t[] = [];
					for (let g of Project.material_groups) canvases.push(g.canvas);
					let group: zui_node_canvas_t = UINodes.get_group(canvases, n.name);
					mgroups.push(JSON.parse(JSON.stringify(group)));
					UINodes.traverse_group(mgroups, group);
				}
			}
		}
	}

	static get_group = (canvases: zui_node_canvas_t[], name: string): zui_node_canvas_t => {
		for (let c of canvases) if (c.name == name) return c;
		return null;
	}
}
