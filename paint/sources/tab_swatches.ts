
let _tab_swatches_empty: gpu_texture_t;
let tab_swatches_drag_pos: i32 = -1;
let _tab_swatches_draw_i: i32;

function tab_swatches_empty_set(image: gpu_texture_t) {
	_tab_swatches_empty = image;
}

function tab_swatches_empty_get(): gpu_texture_t {
	if (_tab_swatches_empty == null) {
		let b: u8_array_t   = u8_array_create(4);
		b[0]                = 255;
		b[1]                = 255;
		b[2]                = 255;
		b[3]                = 255;
		_tab_swatches_empty = gpu_create_texture_from_bytes(b, 1, 1);
	}
	return _tab_swatches_empty;
}

function tab_swatches_draw(htab: ui_handle_t) {
	if (ui_tab(htab, tr("Swatches")) && ui._window_h > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();
		let row: f32[] = [ -100, -100, -100, -100, -100 ];
		ui_row(row);

		if (ui_button(tr("New"))) {
			context_set_swatch(make_swatch());
			array_push(project_raw.swatches, context_raw.swatch);
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Add new swatch"));
		}

		if (ui_button(tr("Import"))) {
			ui_menu_draw(function() {
				if (ui_menu_button(tr("Replace Existing"))) {
					project_import_swatches(true);
					context_set_swatch(project_raw.swatches[0]);
				}
				if (ui_menu_button(tr("Append"))) {
					project_import_swatches(false);
				}
			});
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Import swatches"));
		}

		if (ui_button(tr("Export"))) {
			project_export_swatches();
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Export swatches"));
		}

		if (ui_button(tr("Clear"))) {
			context_set_swatch(make_swatch());
			project_raw.swatches = [ context_raw.swatch ];
		}

		if (ui_button(tr("Restore"))) {
			project_set_default_swatches();
			context_set_swatch(project_raw.swatches[0]);
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Restore default swatches"));
		}

		ui_end_sticky();
		ui_separator(3, false);

		let slotw: i32 = math_floor(26 * UI_SCALE());
		let num: i32   = math_floor(ui._w / (slotw + 3));
		if (num == 0) {
			return;
		}
		let drag_pos_set: bool = false;

		let uix: f32 = 0.0;
		let uiy: f32 = 0.0;
		for (let row: i32 = 0; row < math_floor(math_ceil(project_raw.swatches.length / num)); ++row) {
			let ar: f32[] = [];
			for (let i: i32 = 0; i < num; ++i) {
				array_push(ar, 1 / num);
			}
			ui_row(ar);

			ui._x += 2;
			if (row > 0) {
				ui._y += 6;
			}

			for (let j: i32 = 0; j < num; ++j) {
				let i: i32 = j + row * num;
				if (i >= project_raw.swatches.length) {
					ui_end_element_of_size(slotw);
					continue;
				}

				if (context_raw.swatch == project_raw.swatches[i]) {
					let w: i32 = 32;
					ui_fill(-2, -2, w, w, ui.ops.theme.HIGHLIGHT_COL);
				}

				uix = ui._x;
				uiy = ui._y;

				// Draw the drag position indicator
				if (base_drag_swatch != null && tab_swatches_drag_pos == i) {
					ui_fill(-1, -2, 2, 32, ui.ops.theme.HIGHLIGHT_COL);
				}

				let state: ui_state_t = ui_image(tab_swatches_empty_get(), project_raw.swatches[i].base, slotw);

				if (state == ui_state_t.STARTED) {
					context_set_swatch(project_raw.swatches[i]);
					base_drag_off_x          = -(mouse_x - uix - ui._window_x - 2 * slotw);
					base_drag_off_y          = -(mouse_y - uiy - ui._window_y + 1);
					base_drag_swatch         = context_raw.swatch;
					context_raw.picked_color = util_clone_swatch_color(context_raw.swatch);
					ui_header_handle.redraws = 2;
				}
				else if (state == ui_state_t.HOVERED) {
					tab_swatches_drag_pos =
					    (mouse_x > uix + ui._window_x + slotw / 2) ? i + 1 : i; // Switch to the next position if the mouse crosses the swatch rectangle center
					drag_pos_set = true;
				}
				else if (state == ui_state_t.RELEASED) {
					if (sys_time() - context_raw.select_time < 0.2) {

						_tab_swatches_draw_i = i;

						ui_menu_draw(function() {
							ui.changed         = false;
							let h: ui_handle_t = ui_handle(__ID__);
							h.color            = context_raw.swatch.base;

							context_raw.swatch.base = ui_color_wheel(h, false, -1, 11 * ui.ops.theme.ELEMENT_H * UI_SCALE(), true, function() {
								context_raw.color_picker_previous_tool = context_raw.tool;
								context_select_tool(tool_type_t.PICKER);
								context_raw.color_picker_callback = function(color: swatch_color_t) {
									let i: i32              = _tab_swatches_draw_i;
									project_raw.swatches[i] = project_clone_swatch(color);
								};
							});

							let hopacity: ui_handle_t    = ui_handle(__ID__);
							hopacity.f                   = context_raw.swatch.opacity;
							context_raw.swatch.opacity   = ui_slider(hopacity, "Opacity", 0, 1, true);
							let hocclusion: ui_handle_t  = ui_handle(__ID__);
							hocclusion.f                 = context_raw.swatch.occlusion;
							context_raw.swatch.occlusion = ui_slider(hocclusion, "Occlusion", 0, 1, true);
							let hroughness: ui_handle_t  = ui_handle(__ID__);
							hroughness.f                 = context_raw.swatch.roughness;
							context_raw.swatch.roughness = ui_slider(hroughness, "Roughness", 0, 1, true);
							let hmetallic: ui_handle_t   = ui_handle(__ID__);
							hmetallic.f                  = context_raw.swatch.metallic;
							context_raw.swatch.metallic  = ui_slider(hmetallic, "Metallic", 0, 1, true);
							let hheight: ui_handle_t     = ui_handle(__ID__);
							hheight.f                    = context_raw.swatch.height;
							context_raw.swatch.height    = ui_slider(hheight, "Height", 0, 1, true);

							if (ui.changed || ui.is_typing) {
								ui_menu_keep_open = true;
							}
							if (ui.input_released) {
								context_set_swatch(context_raw.swatch); // Trigger material preview update
								context_raw.picked_color = util_clone_swatch_color(context_raw.swatch);
								ui_header_handle.redraws = 2;
							}
						});
					}

					context_raw.select_time = sys_time();
				}
				if (ui.is_hovered && ui.input_released_r) {
					context_set_swatch(project_raw.swatches[i]);

					_tab_swatches_draw_i = i;

					ui_menu_draw(function() {
						let i: i32 = _tab_swatches_draw_i;

						if (ui_menu_button(tr("Duplicate"))) {
							context_set_swatch(project_clone_swatch(context_raw.swatch));
							array_push(project_raw.swatches, context_raw.swatch);
						}
						/// if (arm_windows || arm_linux || arm_macos)
						else if (ui_menu_button(tr("Copy Hex Code"))) {
							let color: i32 = context_raw.swatch.base;
							color          = color_set_ab(color, context_raw.swatch.opacity * 255);
							let val: u32   = color;
							iron_copy_to_clipboard(i32_to_string(val));
						}
						/// end
						else if (project_raw.swatches.length > 1 && ui_menu_button(tr("Delete"), "delete")) {
							tab_swatches_delete_swatch(project_raw.swatches[i]);
						}
						else if (ui_menu_button(tr("Create Material"))) {
							tab_materials_accept_swatch_drop(project_raw.swatches[i]);
						}
						else if (ui_menu_button(tr("Create Color Layer"))) {
							let color: i32 = project_raw.swatches[i].base;
							color          = color_set_ab(color, project_raw.swatches[i].opacity * 255);
							layers_create_color_layer(color, project_raw.swatches[i].occlusion, project_raw.swatches[i].roughness,
							                          project_raw.swatches[i].metallic);
						}
					});
				}
				if (ui.is_hovered) {
					let color: i32 = project_raw.swatches[i].base;
					color          = color_set_ab(color, project_raw.swatches[i].opacity * 255);
					let val: u32   = color;
					ui_tooltip("#" + i32_to_string_hex(val));
				}
			}
		}

		// Draw the rightmost line next to the last swatch
		if (base_drag_swatch != null && tab_swatches_drag_pos == project_raw.swatches.length) {
			ui._x = uix; // Reset the position because otherwise it would start in the row below
			ui._y = uiy;
			ui_fill(28, -2, 2, 32, ui.ops.theme.HIGHLIGHT_COL);
		}

		// Currently there is no valid drag_position so reset it
		if (!drag_pos_set) {
			tab_swatches_drag_pos = -1;
		}

		let in_focus: bool =
		    ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w && ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
		if (in_focus && ui.is_delete_down && project_raw.swatches.length > 1) {
			ui.is_delete_down = false;
			tab_swatches_delete_swatch(context_raw.swatch);
		}
	}
}

function tab_swatches_accept_swatch_drop(swatch: swatch_color_t) {
	// No valid position available
	if (tab_swatches_drag_pos == -1) {
		return;
	}

	let swatch_pos: i32 = array_index_of(project_raw.swatches, swatch);
	// A new swatch from color picker
	if (swatch_pos == -1) {
		array_insert(project_raw.swatches, tab_swatches_drag_pos, swatch);
	}
	else if (math_abs(swatch_pos - tab_swatches_drag_pos) > 0) { // Existing swatch is reordered
		array_remove(project_raw.swatches, swatch);
		// If the new position is after the old one, decrease by one because the swatch has been deleted
		let new_pos: i32 = tab_swatches_drag_pos - swatch_pos > 0 ? tab_swatches_drag_pos - 1 : tab_swatches_drag_pos;
		array_insert(project_raw.swatches, new_pos, swatch);
	}
}

function tab_swatches_delete_swatch(swatch: swatch_color_t) {
	let i: i32 = array_index_of(project_raw.swatches, swatch);
	context_set_swatch(project_raw.swatches[i == project_raw.swatches.length - 1 ? i - 1 : i + 1]);
	array_splice(project_raw.swatches, i, 1);
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
}
