
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
		let ui: zui_t = UIBase.ui;
		let statush: i32 = Config.raw.layout[layout_size_t.STATUS_H];
		if (zui_tab(htab, tr("Swatches")) && statush > UIStatus.default_status_h * zui_SCALE(ui)) {

			zui_begin_sticky();
			if (Config.raw.touch_ui) {
				zui_row([1 / 5, 1 / 5, 1 / 5, 1 / 5, 1 / 5]);
			}
			else {
				zui_row([1 / 14, 1 / 14, 1 / 14, 1 / 14, 1 / 14]);
			}

			if (zui_button(tr("New"))) {
				Context.set_swatch(Project.make_swatch());
				Project.raw.swatches.push(Context.raw.swatch);
			}
			if (ui.is_hovered) zui_tooltip(tr("Add new swatch"));

			if (zui_button(tr("Import"))) {
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menu_button(ui, tr("Replace Existing"))) {
						Project.import_swatches(true);
						Context.set_swatch(Project.raw.swatches[0]);
					}
					if (UIMenu.menu_button(ui, tr("Append"))) {
						Project.import_swatches(false);
					}
				}, 2);
			}
			if (ui.is_hovered) zui_tooltip(tr("Import swatches"));

			if (zui_button(tr("Export"))) Project.export_swatches();
			if (ui.is_hovered) zui_tooltip(tr("Export swatches"));

			if (zui_button(tr("Clear"))) {
				Context.set_swatch(Project.make_swatch());
				Project.raw.swatches = [Context.raw.swatch];
			}

			if (zui_button(tr("Restore"))) {
				Project.set_default_swatches();
				Context.set_swatch(Project.raw.swatches[0]);
			}
			if (ui.is_hovered) zui_tooltip(tr("Restore default swatches"));

			zui_end_sticky();
			zui_separator(3, false);

			let slotw: i32 = Math.floor(26 * zui_SCALE(ui));
			let num: i32 = Math.floor(ui._w / (slotw + 3));
			let dragPositionSet: bool = false;

			let uix: f32 = 0.0;
			let uiy: f32 = 0.0;
			for (let row: i32 = 0; row < Math.floor(Math.ceil(Project.raw.swatches.length / num)); ++row) {
				let ar: f32[] = [];
				for (let i: i32 = 0; i < num; ++i) ar.push(1 / num);
				zui_row(ar);

				ui._x += 2;
				if (row > 0) ui._y += 6;

				for (let j: i32 = 0; j < num; ++j) {
					let i: i32 = j + row * num;
					if (i >= Project.raw.swatches.length) {
						zui_end_element(slotw);
						continue;
					}

					if (Context.raw.swatch == Project.raw.swatches[i]) {
						let w: i32 = 32;
						zui_fill(-2, -2, w, w, ui.t.HIGHLIGHT_COL);
					}

					uix = ui._x;
					uiy = ui._y;

					// Draw the drag position indicator
					if (Base.drag_swatch != null && TabSwatches.drag_pos == i) {
						zui_fill(-1, -2 , 2, 32, ui.t.HIGHLIGHT_COL);
					}

					let state: zui_state_t = zui_image(TabSwatches.empty, Project.raw.swatches[i].base, slotw);

					if (state == zui_state_t.STARTED) {
						Context.set_swatch(Project.raw.swatches[i]);

						Base.drag_off_x = -(mouse_x - uix - ui._window_x - 2 * slotw);
						Base.drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
						Base.drag_swatch = Context.raw.swatch;
					}
					else if (state == zui_state_t.HOVERED) {
						TabSwatches.drag_pos = (mouse_x > uix + ui._window_x + slotw / 2) ? i + 1 : i; // Switch to the next position if the mouse crosses the swatch rectangle center
						dragPositionSet = true;
					}
					else if (state == zui_state_t.RELEASED) {
						if (time_time() - Context.raw.select_time < 0.25) {
							UIMenu.draw((ui: zui_t) => {
								ui.changed = false;
								let h: zui_handle_t = zui_handle("tabswatches_0");
								h.color = Context.raw.swatch.base;

								Context.raw.swatch.base = zui_color_wheel(h, false, null, 11 * ui.t.ELEMENT_H * zui_SCALE(ui), true, () => {
									Context.raw.color_picker_previous_tool = Context.raw.tool;
									Context.select_tool(workspace_tool_t.PICKER);
									Context.raw.color_picker_callback = (color: swatch_color_t) => {
										Project.raw.swatches[i] = Project.clone_swatch(color);
									};
								});
								let hopacity: zui_handle_t = zui_handle("tabswatches_1");
								hopacity.value = Context.raw.swatch.opacity;
								Context.raw.swatch.opacity = zui_slider(hopacity, "Opacity", 0, 1, true);
								let hocclusion: zui_handle_t = zui_handle("tabswatches_2");
								hocclusion.value = Context.raw.swatch.occlusion;
								Context.raw.swatch.occlusion = zui_slider(hocclusion, "Occlusion", 0, 1, true);
								let hroughness: zui_handle_t = zui_handle("tabswatches_3");
								hroughness.value = Context.raw.swatch.roughness;
								Context.raw.swatch.roughness = zui_slider(hroughness, "Roughness", 0, 1, true);
								let hmetallic: zui_handle_t = zui_handle("tabswatches_4");
								hmetallic.value = Context.raw.swatch.metallic;
								Context.raw.swatch.metallic = zui_slider(hmetallic, "Metallic", 0, 1, true);
								let hheight: zui_handle_t = zui_handle("tabswatches_5");
								hheight.value = Context.raw.swatch.height;
								Context.raw.swatch.height = zui_slider(hheight, "Height", 0, 1, true);

								if (ui.changed || ui.is_typing) UIMenu.keep_open = true;
								if (ui.input_released) Context.set_swatch(Context.raw.swatch); // Trigger material preview update
							}, 16, Math.floor(mouse_x - 200 * zui_SCALE(ui)), Math.floor(mouse_y - 250 * zui_SCALE(ui)));
						}

						Context.raw.select_time = time_time();
					}
					if (ui.is_hovered && ui.input_released_r) {
						Context.set_swatch(Project.raw.swatches[i]);
						let add: i32 = Project.raw.swatches.length > 1 ? 1 : 0;
						///if (krom_windows || krom_linux || krom_darwin)
						add += 1; // Copy
						///end

						///if (is_paint || is_sculpt)
						add += 3;
						///end
						///if is_lav
						add += 1;
						///end

						UIMenu.draw((ui: zui_t) => {
							if (UIMenu.menu_button(ui, tr("Duplicate"))) {
								Context.set_swatch(Project.clone_swatch(Context.raw.swatch));
								Project.raw.swatches.push(Context.raw.swatch);
							}
							///if (krom_windows || krom_linux || krom_darwin)
							else if (UIMenu.menu_button(ui, tr("Copy Hex Code"))) {
								let color: i32 = Context.raw.swatch.base;
								color = color_set_ab(color, Context.raw.swatch.opacity * 255);
								let val: i32 = color;
								if (val < 0) val += 4294967296;
								krom_copy_to_clipboard(val.toString(16));
							}
							///end
							else if (Project.raw.swatches.length > 1 && UIMenu.menu_button(ui, tr("Delete"), "delete")) {
								TabSwatches.delete_swatch(Project.raw.swatches[i]);
							}
							///if (is_paint || is_sculpt)
							else if (UIMenu.menu_button(ui, tr("Create Material"))) {
								TabMaterials.accept_swatch_drag(Project.raw.swatches[i]);
							}
							else if (UIMenu.menu_button(ui, tr("Create Color Layer"))) {
								let color: i32 = Project.raw.swatches[i].base;
								color = color_set_ab(color, Project.raw.swatches[i].opacity * 255);
								Base.create_color_layer(color, Project.raw.swatches[i].occlusion, Project.raw.swatches[i].roughness, Project.raw.swatches[i].metallic);
							}
							///end
						}, add);
					}
					if (ui.is_hovered) {
						let color: i32 = Project.raw.swatches[i].base;
						color = color_set_ab(color, Project.raw.swatches[i].opacity * 255);
						let val: i32 = color;
						if (val < 0) val += 4294967296;
						zui_tooltip("#" + val.toString(16));
					}
				}
			}

			// Draw the rightmost line next to the last swatch
			if (Base.drag_swatch != null && TabSwatches.drag_pos == Project.raw.swatches.length) {
				ui._x = uix; // Reset the position because otherwise it would start in the row below
				ui._y = uiy;
				zui_fill(28, -2, 2, 32, ui.t.HIGHLIGHT_COL);
			}

			// Currently there is no valid dragPosition so reset it
			if (!dragPositionSet) {
				TabSwatches.drag_pos = -1;
			}

			let inFocus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  		ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (inFocus && ui.is_delete_down && Project.raw.swatches.length > 1) {
				ui.is_delete_down = false;
				TabSwatches.delete_swatch(Context.raw.swatch);
			}
		}
	}

	static accept_swatch_drag = (swatch: swatch_color_t) => {
		// No valid position available
		if (TabSwatches.drag_pos == -1) return;

		let swatchPosition: i32 = Project.raw.swatches.indexOf(swatch);
		// A new swatch from color picker
		if (swatchPosition == -1) {
			Project.raw.swatches.splice(TabSwatches.drag_pos, 0, swatch);
		}
		else if (Math.abs(swatchPosition - TabSwatches.drag_pos) > 0) { // Existing swatch is reordered
			array_remove(Project.raw.swatches, swatch);
			// If the new position is after the old one, decrease by one because the swatch has been deleted
			let newPosition: i32 = TabSwatches.drag_pos - swatchPosition > 0 ? TabSwatches.drag_pos -1 : TabSwatches.drag_pos;
			Project.raw.swatches.splice(newPosition, 0, swatch);
		}
	}

	static delete_swatch = (swatch: swatch_color_t) => {
		let i: i32 = Project.raw.swatches.indexOf(swatch);
		Context.set_swatch(Project.raw.swatches[i == Project.raw.swatches.length - 1 ? i - 1 : i + 1]);
		Project.raw.swatches.splice(i, 1);
		UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
	}
}
