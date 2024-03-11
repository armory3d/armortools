
class TabSwatches {

	static _empty: image_t;
	static drag_pos: i32 = -1;

	static set empty(image: image_t) {
		TabSwatches._empty = image;
	}

	static get empty(): image_t {
		if (TabSwatches._empty == null) {
			let b: Uint8Array = new Uint8Array(4);
			b[0] = 255;
			b[1] = 255;
			b[2] = 255;
			b[3] = 255;
			TabSwatches._empty = image_from_bytes(b.buffer, 1, 1);
		}
		return TabSwatches._empty;
	}

	static draw = (htab: zui_handle_t) => {
		let ui: zui_t = ui_base_ui;
		let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
		if (zui_tab(htab, tr("Swatches")) && statush > ui_status_default_status_h * zui_SCALE(ui)) {

			zui_begin_sticky();
			if (config_raw.touch_ui) {
				zui_row([1 / 5, 1 / 5, 1 / 5, 1 / 5, 1 / 5]);
			}
			else {
				zui_row([1 / 14, 1 / 14, 1 / 14, 1 / 14, 1 / 14]);
			}

			if (zui_button(tr("New"))) {
				context_set_swatch(make_swatch());
				project_raw.swatches.push(context_raw.swatch);
			}
			if (ui.is_hovered) zui_tooltip(tr("Add new swatch"));

			if (zui_button(tr("Import"))) {
				ui_menu_draw((ui: zui_t) => {
					if (ui_menu_button(ui, tr("Replace Existing"))) {
						project_import_swatches(true);
						context_set_swatch(project_raw.swatches[0]);
					}
					if (ui_menu_button(ui, tr("Append"))) {
						project_import_swatches(false);
					}
				}, 2);
			}
			if (ui.is_hovered) zui_tooltip(tr("Import swatches"));

			if (zui_button(tr("Export"))) project_export_swatches();
			if (ui.is_hovered) zui_tooltip(tr("Export swatches"));

			if (zui_button(tr("Clear"))) {
				context_set_swatch(make_swatch());
				project_raw.swatches = [context_raw.swatch];
			}

			if (zui_button(tr("Restore"))) {
				project_set_default_swatches();
				context_set_swatch(project_raw.swatches[0]);
			}
			if (ui.is_hovered) zui_tooltip(tr("Restore default swatches"));

			zui_end_sticky();
			zui_separator(3, false);

			let slotw: i32 = math_floor(26 * zui_SCALE(ui));
			let num: i32 = math_floor(ui._w / (slotw + 3));
			let drag_pos_set: bool = false;

			let uix: f32 = 0.0;
			let uiy: f32 = 0.0;
			for (let row: i32 = 0; row < math_floor(math_ceil(project_raw.swatches.length / num)); ++row) {
				let ar: f32[] = [];
				for (let i: i32 = 0; i < num; ++i) ar.push(1 / num);
				zui_row(ar);

				ui._x += 2;
				if (row > 0) ui._y += 6;

				for (let j: i32 = 0; j < num; ++j) {
					let i: i32 = j + row * num;
					if (i >= project_raw.swatches.length) {
						zui_end_element(slotw);
						continue;
					}

					if (context_raw.swatch == project_raw.swatches[i]) {
						let w: i32 = 32;
						zui_fill(-2, -2, w, w, ui.t.HIGHLIGHT_COL);
					}

					uix = ui._x;
					uiy = ui._y;

					// Draw the drag position indicator
					if (base_drag_swatch != null && TabSwatches.drag_pos == i) {
						zui_fill(-1, -2 , 2, 32, ui.t.HIGHLIGHT_COL);
					}

					let state: zui_state_t = zui_image(TabSwatches.empty, project_raw.swatches[i].base, slotw);

					if (state == zui_state_t.STARTED) {
						context_set_swatch(project_raw.swatches[i]);

						base_drag_off_x = -(mouse_x - uix - ui._window_x - 2 * slotw);
						base_drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
						base_drag_swatch = context_raw.swatch;
					}
					else if (state == zui_state_t.HOVERED) {
						TabSwatches.drag_pos = (mouse_x > uix + ui._window_x + slotw / 2) ? i + 1 : i; // Switch to the next position if the mouse crosses the swatch rectangle center
						drag_pos_set = true;
					}
					else if (state == zui_state_t.RELEASED) {
						if (time_time() - context_raw.select_time < 0.25) {
							ui_menu_draw((ui: zui_t) => {
								ui.changed = false;
								let h: zui_handle_t = zui_handle("tabswatches_0");
								h.color = context_raw.swatch.base;

								context_raw.swatch.base = zui_color_wheel(h, false, null, 11 * ui.t.ELEMENT_H * zui_SCALE(ui), true, () => {
									context_raw.color_picker_previous_tool = context_raw.tool;
									context_select_tool(workspace_tool_t.PICKER);
									context_raw.color_picker_callback = (color: swatch_color_t) => {
										project_raw.swatches[i] = project_clone_swatch(color);
									};
								});
								let hopacity: zui_handle_t = zui_handle("tabswatches_1");
								hopacity.value = context_raw.swatch.opacity;
								context_raw.swatch.opacity = zui_slider(hopacity, "Opacity", 0, 1, true);
								let hocclusion: zui_handle_t = zui_handle("tabswatches_2");
								hocclusion.value = context_raw.swatch.occlusion;
								context_raw.swatch.occlusion = zui_slider(hocclusion, "Occlusion", 0, 1, true);
								let hroughness: zui_handle_t = zui_handle("tabswatches_3");
								hroughness.value = context_raw.swatch.roughness;
								context_raw.swatch.roughness = zui_slider(hroughness, "Roughness", 0, 1, true);
								let hmetallic: zui_handle_t = zui_handle("tabswatches_4");
								hmetallic.value = context_raw.swatch.metallic;
								context_raw.swatch.metallic = zui_slider(hmetallic, "Metallic", 0, 1, true);
								let hheight: zui_handle_t = zui_handle("tabswatches_5");
								hheight.value = context_raw.swatch.height;
								context_raw.swatch.height = zui_slider(hheight, "Height", 0, 1, true);

								if (ui.changed || ui.is_typing) ui_menu_keep_open = true;
								if (ui.input_released) context_set_swatch(context_raw.swatch); // Trigger material preview update
							}, 16, math_floor(mouse_x - 200 * zui_SCALE(ui)), math_floor(mouse_y - 250 * zui_SCALE(ui)));
						}

						context_raw.select_time = time_time();
					}
					if (ui.is_hovered && ui.input_released_r) {
						context_set_swatch(project_raw.swatches[i]);
						let add: i32 = project_raw.swatches.length > 1 ? 1 : 0;
						///if (krom_windows || krom_linux || krom_darwin)
						add += 1; // Copy
						///end

						///if (is_paint || is_sculpt)
						add += 3;
						///end
						///if is_lav
						add += 1;
						///end

						ui_menu_draw((ui: zui_t) => {
							if (ui_menu_button(ui, tr("Duplicate"))) {
								context_set_swatch(project_clone_swatch(context_raw.swatch));
								project_raw.swatches.push(context_raw.swatch);
							}
							///if (krom_windows || krom_linux || krom_darwin)
							else if (ui_menu_button(ui, tr("Copy Hex Code"))) {
								let color: i32 = context_raw.swatch.base;
								color = color_set_ab(color, context_raw.swatch.opacity * 255);
								let val: i32 = color;
								if (val < 0) val += 4294967296;
								krom_copy_to_clipboard(val.toString(16));
							}
							///end
							else if (project_raw.swatches.length > 1 && ui_menu_button(ui, tr("Delete"), "delete")) {
								TabSwatches.delete_swatch(project_raw.swatches[i]);
							}
							///if (is_paint || is_sculpt)
							else if (ui_menu_button(ui, tr("Create Material"))) {
								TabMaterials.accept_swatch_drag(project_raw.swatches[i]);
							}
							else if (ui_menu_button(ui, tr("Create Color Layer"))) {
								let color: i32 = project_raw.swatches[i].base;
								color = color_set_ab(color, project_raw.swatches[i].opacity * 255);
								base_create_color_layer(color, project_raw.swatches[i].occlusion, project_raw.swatches[i].roughness, project_raw.swatches[i].metallic);
							}
							///end
						}, add);
					}
					if (ui.is_hovered) {
						let color: i32 = project_raw.swatches[i].base;
						color = color_set_ab(color, project_raw.swatches[i].opacity * 255);
						let val: i32 = color;
						if (val < 0) val += 4294967296;
						zui_tooltip("#" + val.toString(16));
					}
				}
			}

			// Draw the rightmost line next to the last swatch
			if (base_drag_swatch != null && TabSwatches.drag_pos == project_raw.swatches.length) {
				ui._x = uix; // Reset the position because otherwise it would start in the row below
				ui._y = uiy;
				zui_fill(28, -2, 2, 32, ui.t.HIGHLIGHT_COL);
			}

			// Currently there is no valid dragPosition so reset it
			if (!drag_pos_set) {
				TabSwatches.drag_pos = -1;
			}

			let in_focus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
								 ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (in_focus && ui.is_delete_down && project_raw.swatches.length > 1) {
				ui.is_delete_down = false;
				TabSwatches.delete_swatch(context_raw.swatch);
			}
		}
	}

	static accept_swatch_drag = (swatch: swatch_color_t) => {
		// No valid position available
		if (TabSwatches.drag_pos == -1) return;

		let swatch_pos: i32 = project_raw.swatches.indexOf(swatch);
		// A new swatch from color picker
		if (swatch_pos == -1) {
			project_raw.swatches.splice(TabSwatches.drag_pos, 0, swatch);
		}
		else if (math_abs(swatch_pos - TabSwatches.drag_pos) > 0) { // Existing swatch is reordered
			array_remove(project_raw.swatches, swatch);
			// If the new position is after the old one, decrease by one because the swatch has been deleted
			let new_pos: i32 = TabSwatches.drag_pos - swatch_pos > 0 ? TabSwatches.drag_pos -1 : TabSwatches.drag_pos;
			project_raw.swatches.splice(new_pos, 0, swatch);
		}
	}

	static delete_swatch = (swatch: swatch_color_t) => {
		let i: i32 = project_raw.swatches.indexOf(swatch);
		context_set_swatch(project_raw.swatches[i == project_raw.swatches.length - 1 ? i - 1 : i + 1]);
		project_raw.swatches.splice(i, 1);
		ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
	}
}
