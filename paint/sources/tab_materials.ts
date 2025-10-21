
let _tab_materials_draw_slots: i32;

function tab_materials_draw(htab: ui_handle_t) {
	let mini: bool = ui._window_w <= ui_sidebar_w_mini;
	mini ? tab_materials_draw_mini(htab) : tab_materials_draw_full(htab);
}

function tab_materials_draw_mini(htab: ui_handle_t) {
	ui_set_hovered_tab_name(tr("Materials"));

	ui_begin_sticky();
	ui_separator(5);

	tab_materials_button_nodes();
	tab_materials_button_new("+");

	ui_end_sticky();
	ui_separator(3, false);
	tab_materials_draw_slots(true);
}

function tab_materials_draw_full(htab: ui_handle_t) {
	if (ui_tab(htab, tr("Materials"))) {
		ui_begin_sticky();
		let row: f32[] = [ -70, -70, -70 ];
		ui_row(row);

		tab_materials_button_new(tr("New"));
		if (ui_button(tr("Import"))) {
			project_import_material();
		}
		tab_materials_button_nodes();

		ui_end_sticky();
		ui_separator(3, false);
		tab_materials_draw_slots(false);
	}
}

function tab_materials_button_nodes() {
	if (ui_button(tr("Nodes"))) {
		ui_base_show_material_nodes();
	}
	else if (ui.is_hovered) {
		ui_tooltip(tr("Show Node Editor") + " (" + map_get(config_keymap, "toggle_node_editor") + ")");
	}
}

function tab_materials_draw_slots(mini: bool) {
	let slotw: i32 = math_floor((51 + math_floor(config_raw.window_scale * 2)) * UI_SCALE());
	let num: i32   = math_floor(ui._window_w / slotw);
	if (num == 0) {
		return;
	}

	for (let row: i32 = 0; row < math_floor(math_ceil(project_materials.length / num)); ++row) {
		let mult: i32 = config_raw.show_asset_names ? 2 : 1;
		let ar: f32[] = [];
		for (let i: i32 = 0; i < num * mult; ++i) {
			array_push(ar, 1 / num);
		}
		ui_row(ar);

		ui._x += 2;
		let off: f32 = config_raw.show_asset_names ? UI_ELEMENT_OFFSET() * 10.0 : 6;
		if (row > 0) {
			ui._y += off;
		}

		for (let j: i32 = 0; j < num; ++j) {
			let imgw: i32 = math_floor(50 * UI_SCALE());
			let i: i32    = j + row * num;
			if (i >= project_materials.length) {
				ui_end_element_of_size(imgw);
				if (config_raw.show_asset_names) {
					ui_end_element_of_size(0);
				}
				continue;
			}
			let img: gpu_texture_t      = UI_SCALE() > 1 ? project_materials[i].image : project_materials[i].image_icon;
			let img_full: gpu_texture_t = project_materials[i].image;

			// Highligh selected
			if (context_raw.material == project_materials[i]) {
				if (mini) {
					let w: f32 = ui._w / UI_SCALE();
					ui_rect(0, -2, w - 2, w - 4, ui.ops.theme.HIGHLIGHT_COL, 3);
				}
				else {
					let off: i32 = row % 2 == 1 ? 1 : 0;
					let w: i32   = 50 + math_floor(config_raw.window_scale * 2);
					ui_fill(-1, -2, w + 3, 2, ui.ops.theme.HIGHLIGHT_COL);
					ui_fill(-1, w - off, w + 3, 2 + off, ui.ops.theme.HIGHLIGHT_COL);
					ui_fill(-1, -2, 2, w + 3, ui.ops.theme.HIGHLIGHT_COL);
					ui_fill(w + 1, -2, 2, w + 4, ui.ops.theme.HIGHLIGHT_COL);
				}
			}

			// Draw material icon
			let uix: f32          = ui._x;
			let uiy: f32          = ui._y;
			let tile: i32         = UI_SCALE() > 1 ? 100 : 50;
			let imgh: f32         = mini ? ui_sidebar_default_w_mini * 0.85 * UI_SCALE() : 50 * UI_SCALE();
			let state: ui_state_t = project_materials[i].preview_ready ? ui_image(img, 0xffffffff, imgh)
			                                                           : ui_sub_image(resource_get("icons.k"), 0xffffffff, -1.0, tile, tile, tile, tile);

			// Draw material numbers when selecting a material via keyboard shortcut
			let is_typing: bool = ui.is_typing;
			if (!is_typing) {
				if (i < 9 && operator_shortcut(map_get(config_keymap, "select_material"), shortcut_type_t.DOWN)) {
					let number: string = i32_to_string(i + 1);
					let width: i32     = draw_string_width(ui.ops.font, ui.font_size, number) + 10;
					let height: i32    = draw_font_height(ui.ops.font, ui.font_size);
					draw_set_color(ui.ops.theme.TEXT_COL);
					draw_filled_rect(uix, uiy, width, height);
					draw_set_color(ui.ops.theme.BUTTON_COL);
					draw_string(number, uix + 5, uiy);
				}
			}

			// Select material
			if (state == ui_state_t.STARTED && ui.input_y > ui._window_y) {
				if (context_raw.material != project_materials[i]) {
					context_select_material(i);
					if (context_raw.tool == tool_type_t.MATERIAL) {
						sys_notify_on_next_frame(layers_update_fill_layers);
					}
				}
				base_drag_off_x    = -(mouse_x - uix - ui._window_x - 3);
				base_drag_off_y    = -(mouse_y - uiy - ui._window_y + 1);
				base_drag_material = context_raw.material;
				// Double click to show nodes
				if (sys_time() - context_raw.select_time < 0.2) {
					ui_base_show_material_nodes();
					base_drag_material = null;
					base_is_dragging   = false;
				}
				context_raw.select_time = sys_time();
			}

			// Context menu
			if (ui.is_hovered && ui.input_released_r) {
				context_select_material(i);
				_tab_materials_draw_slots = i;

				ui_menu_draw(function() {
					let i: i32             = _tab_materials_draw_slots;
					let m: slot_material_t = project_materials[i];

					if (ui_menu_button(tr("To Fill Layer"))) {
						context_select_material(i);
						layers_create_fill_layer();
					}

					if (ui_menu_button(tr("Export"))) {
						context_select_material(i);
						box_export_show_material();
					}

					if (ui_menu_button(tr("Bake"))) {
						context_select_material(i);
						box_export_show_bake_material();
					}

					if (ui_menu_button(tr("Duplicate"))) {
						sys_notify_on_next_frame(function() {
							let i: i32 = _tab_materials_draw_slots;

							context_raw.material = slot_material_create(project_materials[0].data);
							array_push(project_materials, context_raw.material);
							let cloned: ui_node_canvas_t = util_clone_canvas(project_materials[i].canvas);
							context_raw.material.canvas  = cloned;
							tab_materials_update_material();
							history_duplicate_material();
						});
					}

					if (project_materials.length > 1 && ui_menu_button(tr("Delete"), "delete")) {
						tab_materials_delete_material(m);
					}

					let base_handle: ui_handle_t = ui_nest(ui_handle(__ID__), m.id);
					if (base_handle.init) {
						base_handle.b = m.paint_base;
					}

					let opac_handle: ui_handle_t = ui_nest(ui_handle(__ID__), m.id);
					if (opac_handle.init) {
						opac_handle.b = m.paint_opac;
					}

					let nor_handle: ui_handle_t = ui_nest(ui_handle(__ID__), m.id);
					if (nor_handle.init) {
						nor_handle.b = m.paint_nor;
					}

					let occ_handle: ui_handle_t = ui_nest(ui_handle(__ID__), m.id);
					if (occ_handle.init) {
						occ_handle.b = m.paint_occ;
					}

					let rough_handle: ui_handle_t = ui_nest(ui_handle(__ID__), m.id);
					if (rough_handle.init) {
						rough_handle.b = m.paint_rough;
					}

					let met_handle: ui_handle_t = ui_nest(ui_handle(__ID__), m.id);
					if (met_handle.init) {
						met_handle.b = m.paint_met;
					}

					let height_handle: ui_handle_t = ui_nest(ui_handle(__ID__), m.id);
					if (height_handle.init) {
						height_handle.b = m.paint_height;
					}

					let emis_handle: ui_handle_t = ui_nest(ui_handle(__ID__), m.id);
					if (emis_handle.init) {
						emis_handle.b = m.paint_emis;
					}

					let subs_handle: ui_handle_t = ui_nest(ui_handle(__ID__), m.id);
					if (subs_handle.init) {
						subs_handle.b = m.paint_subs;
					}

					m.paint_base   = ui_check(base_handle, tr("Base Color"));
					m.paint_opac   = ui_check(opac_handle, tr("Opacity"));
					m.paint_nor    = ui_check(nor_handle, tr("Normal"));
					m.paint_occ    = ui_check(occ_handle, tr("Occlusion"));
					m.paint_rough  = ui_check(rough_handle, tr("Roughness"));
					m.paint_met    = ui_check(met_handle, tr("Metallic"));
					m.paint_height = ui_check(height_handle, tr("Height"));
					m.paint_emis   = ui_check(emis_handle, tr("Emission"));
					m.paint_subs   = ui_check(subs_handle, tr("Subsurface"));
					if (base_handle.changed || opac_handle.changed || nor_handle.changed || occ_handle.changed || rough_handle.changed || met_handle.changed ||
					    height_handle.changed || emis_handle.changed || subs_handle.changed) {
						make_material_parse_paint_material();
						ui_menu_keep_open = true;
					}
				});
			}
			if (ui.is_hovered) {
				ui_tooltip_image(img_full);
				if (i < 9) {
					let i1: i32 = i + 1;
					ui_tooltip(project_materials[i].canvas.name + " - (" + map_get(config_keymap, "select_material") + " " + i1 + ")");
				}
				else {
					ui_tooltip(project_materials[i].canvas.name);
				}
			}

			if (config_raw.show_asset_names) {
				ui._x = uix;
				ui._y += slotw * 0.9;
				ui_text(project_materials[i].canvas.name, ui_align_t.CENTER);
				if (ui.is_hovered) {
					if (i < 9) {
						let i1: i32 = i + 1;
						ui_tooltip(project_materials[i].canvas.name + " - (" + map_get(config_keymap, "select_material") + " " + i1 + ")");
					}
					else {
						ui_tooltip(project_materials[i].canvas.name);
					}
				}
				ui._y -= slotw * 0.9;
				if (i == project_materials.length - 1) {
					ui._y += j == num - 1 ? imgw : imgw + UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
				}
			}
		}

		ui._y += mini ? 0 : 6;
	}

	let in_focus: bool =
	    ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w && ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
	if (in_focus && ui.is_delete_down && project_materials.length > 1) {
		ui.is_delete_down = false;
		tab_materials_delete_material(context_raw.material);
	}
}

function tab_materials_button_new(text: string) {
	if (ui_button(text)) {
		sys_notify_on_next_frame(function() {
			context_raw.material = slot_material_create(project_materials[0].data);
			array_push(project_materials, context_raw.material);
			tab_materials_update_material();
			history_new_material();
		});
	}
}

function tab_materials_update_material() {
	ui_header_handle.redraws = 2;
	ui_nodes_hwnd.redraws    = 2;
	ui_nodes_group_stack     = [];
	make_material_parse_paint_material();
	util_render_make_material_preview();
	let decal: bool = context_is_decal();
	if (decal) {
		util_render_make_decal_preview();
	}
}

function tab_materials_update_material_pointers(nodes: ui_node_t[], i: i32) {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let n: ui_node_t = nodes[i];
		if (n.type == "MATERIAL") {
			if (n.buttons[0].default_value[0] == i) {
				n.buttons[0].default_value[0] = 9999; // Material deleted
			}
			else if (n.buttons[0].default_value[0] > i) {
				n.buttons[0].default_value[0]--; // Offset by deleted material
			}
		}
	}
}

function tab_materials_accept_swatch_drop(swatch: swatch_color_t) {
	context_raw.material = slot_material_create(project_materials[0].data);
	for (let i: i32 = 0; i < context_raw.material.canvas.nodes.length; ++i) {
		let node: ui_node_t = context_raw.material.canvas.nodes[i];
		if (node.type == "RGB") {
			node.outputs[0].default_value = f32_array_create_xyzw(color_get_rb(swatch.base) / 255, color_get_gb(swatch.base) / 255,
			                                                      color_get_bb(swatch.base) / 255, color_get_ab(swatch.base) / 255);
		}
		else if (node.type == "OUTPUT_MATERIAL_PBR") {
			node.inputs[1].default_value[0] = swatch.opacity;
			node.inputs[2].default_value[0] = swatch.occlusion;
			node.inputs[3].default_value[0] = swatch.roughness;
			node.inputs[4].default_value[0] = swatch.metallic;
			node.inputs[7].default_value[0] = swatch.height;
		}
	}
	array_push(project_materials, context_raw.material);
	tab_materials_update_material();
	history_new_material();
}

function tab_materials_delete_material(m: slot_material_t) {
	let i: i32 = array_index_of(project_materials, m);
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		if (l.fill_layer == m) {
			l.fill_layer = null;
		}
	}
	history_delete_material();
	context_select_material(i == project_materials.length - 1 ? i - 1 : i + 1);
	array_splice(project_materials, i, 1);
	ui_base_hwnds[1].redraws = 2;
	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		tab_materials_update_material_pointers(m.canvas.nodes, i);
	}
	for (let i: i32 = 0; i < m.canvas.nodes.length; ++i) {
		let n: ui_node_t = m.canvas.nodes[i];
		ui_viewnodes_on_node_remove(n);
	}
}
