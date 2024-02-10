
class TabSwatches {

	static _empty: image_t;

	static set empty(image: image_t) {
		TabSwatches._empty = image;
	}

	static get empty(): image_t {
		if (TabSwatches._empty == null) {
			let b = new Uint8Array(4);
			b[0] = 255;
			b[1] = 255;
			b[2] = 255;
			b[3] = 255;
			TabSwatches._empty = image_from_bytes(b.buffer, 1, 1);
		}
		return TabSwatches._empty;
	}

	static dragPosition: i32 = -1;

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (zui_tab(htab, tr("Swatches")) && statush > UIStatus.defaultStatusH * zui_SCALE(ui)) {

			zui_begin_sticky();
			if (Config.raw.touch_ui) {
				zui_row([1 / 5, 1 / 5, 1 / 5, 1 / 5, 1 / 5]);
			}
			else {
				zui_row([1 / 14, 1 / 14, 1 / 14, 1 / 14, 1 / 14]);
			}

			if (zui_button(tr("New"))) {
				Context.setSwatch(Project.makeSwatch());
				Project.raw.swatches.push(Context.raw.swatch);
			}
			if (ui.is_hovered) zui_tooltip(tr("Add new swatch"));

			if (zui_button(tr("Import"))) {
				UIMenu.draw((ui: zui_t) => {
					if (UIMenu.menuButton(ui, tr("Replace Existing"))) {
						Project.importSwatches(true);
						Context.setSwatch(Project.raw.swatches[0]);
					}
					if (UIMenu.menuButton(ui, tr("Append"))) {
						Project.importSwatches(false);
					}
				}, 2);
			}
			if (ui.is_hovered) zui_tooltip(tr("Import swatches"));

			if (zui_button(tr("Export"))) Project.exportSwatches();
			if (ui.is_hovered) zui_tooltip(tr("Export swatches"));

			if (zui_button(tr("Clear"))) {
				Context.setSwatch(Project.makeSwatch());
				Project.raw.swatches = [Context.raw.swatch];
			}

			if (zui_button(tr("Restore"))) {
				Project.setDefaultSwatches();
				Context.setSwatch(Project.raw.swatches[0]);
			}
			if (ui.is_hovered) zui_tooltip(tr("Restore default swatches"));

			zui_end_sticky();
			zui_separator(3, false);

			let slotw = Math.floor(26 * zui_SCALE(ui));
			let num = Math.floor(ui._w / (slotw + 3));
			let dragPositionSet = false;

			let uix = 0.0;
			let uiy = 0.0;
			for (let row = 0; row < Math.floor(Math.ceil(Project.raw.swatches.length / num)); ++row) {
				let ar = [];
				for (let i = 0; i < num; ++i) ar.push(1 / num);
				zui_row(ar);

				ui._x += 2;
				if (row > 0) ui._y += 6;

				for (let j = 0; j < num; ++j) {
					let i = j + row * num;
					if (i >= Project.raw.swatches.length) {
						zui_end_element(slotw);
						continue;
					}

					if (Context.raw.swatch == Project.raw.swatches[i]) {
						let off = row % 2 == 1 ? 1 : 0;
						let w = 32;
						zui_fill(-2, -2, w, w, ui.t.HIGHLIGHT_COL);
					}

					uix = ui._x;
					uiy = ui._y;

					// Draw the drag position indicator
					if (Base.dragSwatch != null && TabSwatches.dragPosition == i) {
						zui_fill(-1, -2 , 2, 32, ui.t.HIGHLIGHT_COL);
					}

					let state = zui_image(TabSwatches.empty, Project.raw.swatches[i].base, slotw);

					if (state == State.Started) {
						Context.setSwatch(Project.raw.swatches[i]);

						Base.dragOffX = -(mouse_x - uix - ui._window_x - 2 * slotw);
						Base.dragOffY = -(mouse_y - uiy - ui._window_y + 1);
						Base.dragSwatch = Context.raw.swatch;
					}
					else if (state == State.Hovered) {
						TabSwatches.dragPosition = (mouse_x > uix + ui._window_x + slotw / 2) ? i + 1 : i; // Switch to the next position if the mouse crosses the swatch rectangle center
						dragPositionSet = true;
					}
					else if (state == State.Released) {
						if (time_time() - Context.raw.selectTime < 0.25) {
							UIMenu.draw((ui: zui_t) => {
								ui.changed = false;
								let h = zui_handle("tabswatches_0");
								h.color = Context.raw.swatch.base;

								Context.raw.swatch.base = zui_color_wheel(h, false, null, 11 * ui.t.ELEMENT_H * zui_SCALE(ui), true, () => {
									Context.raw.colorPickerPreviousTool = Context.raw.tool;
									Context.selectTool(WorkspaceTool.ToolPicker);
									Context.raw.colorPickerCallback = (color: TSwatchColor) => {
										Project.raw.swatches[i] = Project.cloneSwatch(color);
									};
								});
								let hopacity = zui_handle("tabswatches_1");
								hopacity.value = Context.raw.swatch.opacity;
								Context.raw.swatch.opacity = zui_slider(hopacity, "Opacity", 0, 1, true);
								let hocclusion = zui_handle("tabswatches_2");
								hocclusion.value = Context.raw.swatch.occlusion;
								Context.raw.swatch.occlusion = zui_slider(hocclusion, "Occlusion", 0, 1, true);
								let hroughness = zui_handle("tabswatches_3");
								hroughness.value = Context.raw.swatch.roughness;
								Context.raw.swatch.roughness = zui_slider(hroughness, "Roughness", 0, 1, true);
								let hmetallic = zui_handle("tabswatches_4");
								hmetallic.value = Context.raw.swatch.metallic;
								Context.raw.swatch.metallic = zui_slider(hmetallic, "Metallic", 0, 1, true);
								let hheight = zui_handle("tabswatches_5");
								hheight.value = Context.raw.swatch.height;
								Context.raw.swatch.height = zui_slider(hheight, "Height", 0, 1, true);

								if (ui.changed || ui.is_typing) UIMenu.keepOpen = true;
								if (ui.input_released) Context.setSwatch(Context.raw.swatch); // Trigger material preview update
							}, 16, Math.floor(mouse_x - 200 * zui_SCALE(ui)), Math.floor(mouse_y - 250 * zui_SCALE(ui)));
						}

						Context.raw.selectTime = time_time();
					}
					if (ui.is_hovered && ui.input_released_r) {
						Context.setSwatch(Project.raw.swatches[i]);
						let add = Project.raw.swatches.length > 1 ? 1 : 0;
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
							if (UIMenu.menuButton(ui, tr("Duplicate"))) {
								Context.setSwatch(Project.cloneSwatch(Context.raw.swatch));
								Project.raw.swatches.push(Context.raw.swatch);
							}
							///if (krom_windows || krom_linux || krom_darwin)
							else if (UIMenu.menuButton(ui, tr("Copy Hex Code"))) {
								let color = Context.raw.swatch.base;
								color = color_set_ab(color, Context.raw.swatch.opacity * 255);
								let val = color;
								if (val < 0) val += 4294967296;
								krom_copy_to_clipboard(val.toString(16));
							}
							///end
							else if (Project.raw.swatches.length > 1 && UIMenu.menuButton(ui, tr("Delete"), "delete")) {
								TabSwatches.deleteSwatch(Project.raw.swatches[i]);
							}
							///if (is_paint || is_sculpt)
							else if (UIMenu.menuButton(ui, tr("Create Material"))) {
								TabMaterials.acceptSwatchDrag(Project.raw.swatches[i]);
							}
							else if (UIMenu.menuButton(ui, tr("Create Color Layer"))) {
								let color = Project.raw.swatches[i].base;
								color = color_set_ab(color, Project.raw.swatches[i].opacity * 255);
								Base.createColorLayer(color, Project.raw.swatches[i].occlusion, Project.raw.swatches[i].roughness, Project.raw.swatches[i].metallic);
							}
							///end
						}, add);
					}
					if (ui.is_hovered) {
						let color = Project.raw.swatches[i].base;
						color = color_set_ab(color, Project.raw.swatches[i].opacity * 255);
						let val = color;
						if (val < 0) val += 4294967296;
						zui_tooltip("#" + val.toString(16));
					}
				}
			}

			// Draw the rightmost line next to the last swatch
			if (Base.dragSwatch != null && TabSwatches.dragPosition == Project.raw.swatches.length) {
				ui._x = uix; // Reset the position because otherwise it would start in the row below
				ui._y = uiy;
				zui_fill(28, -2, 2, 32, ui.t.HIGHLIGHT_COL);
			}

			// Currently there is no valid dragPosition so reset it
			if (!dragPositionSet) {
				TabSwatches.dragPosition = -1;
			}

			let inFocus = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (inFocus && ui.is_delete_down && Project.raw.swatches.length > 1) {
				ui.is_delete_down = false;
				TabSwatches.deleteSwatch(Context.raw.swatch);
			}
		}
	}

	static acceptSwatchDrag = (swatch: TSwatchColor) => {
		// No valid position available
		if (TabSwatches.dragPosition == -1) return;

		let swatchPosition = Project.raw.swatches.indexOf(swatch);
		// A new swatch from color picker
		if (swatchPosition == -1) {
			Project.raw.swatches.splice(TabSwatches.dragPosition, 0, swatch);
		}
		else if (Math.abs(swatchPosition - TabSwatches.dragPosition) > 0) { // Existing swatch is reordered
			array_remove(Project.raw.swatches, swatch);
			// If the new position is after the old one, decrease by one because the swatch has been deleted
			let newPosition = TabSwatches.dragPosition - swatchPosition > 0 ? TabSwatches.dragPosition -1 : TabSwatches.dragPosition;
			Project.raw.swatches.splice(newPosition, 0, swatch);
		}
	}

	static deleteSwatch = (swatch: TSwatchColor) => {
		let i = Project.raw.swatches.indexOf(swatch);
		Context.setSwatch(Project.raw.swatches[i == Project.raw.swatches.length - 1 ? i - 1 : i + 1]);
		Project.raw.swatches.splice(i, 1);
		UIBase.hwnds[TabArea.TabStatus].redraws = 2;
	}
}
