
let _tab_fonts_draw_i: i32;

function tab_fonts_draw(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
	if (ui_tab(htab, tr("Fonts")) && statush > ui_status_default_status_h * ui_SCALE(ui)) {

		ui_begin_sticky();
		if (config_raw.touch_ui) {
			let row: f32[] = [1 / 4, 1 / 4];
			ui_row(row);
		}
		else {
			let row: f32[] = [1 / 14, 1 / 14];
			ui_row(row);
		}

		if (ui_button(tr("Import"))) {
			project_import_asset("ttf,ttc,otf");
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Import font file"));
		}

		if (ui_button(tr("2D View"))) {
			ui_base_show_2d_view(view_2d_type_t.FONT);
		}
		ui_end_sticky();
		ui_separator(3, false);

		let statusw: i32 = sys_width() - ui_toolbar_w - config_raw.layout[layout_size_t.SIDEBAR_W];
		let slotw: i32 = math_floor(51 * ui_SCALE(ui));
		let num: i32 = math_floor(statusw / slotw);
		if (num == 0) {
			return;
		}

		for (let row: i32 = 0; row < math_floor(math_ceil(project_fonts.length / num)); ++row) {
			let mult: i32 = config_raw.show_asset_names ? 2 : 1;
			let ar: f32[] = [];
			for (let i: i32 = 0; i < num * mult; ++i) {
				array_push(ar, 1 / num);
			}
			ui_row(ar);

			ui._x += 2;
			let off: f32 = config_raw.show_asset_names ? ui_ELEMENT_OFFSET(ui) * 10.0 : 6;
			if (row > 0) {
				ui._y += off;
			}

			for (let j: i32 = 0; j < num; ++j) {
				let imgw: i32 = math_floor(50 * ui_SCALE(ui));
				let i: i32 = j + row * num;
				if (i >= project_fonts.length) {
					_ui_end_element(imgw);
					if (config_raw.show_asset_names) {
						_ui_end_element(0);
					}
					continue;
				}
				let img: image_t = project_fonts[i].image;

				if (context_raw.font == project_fonts[i]) {
					// ui_fill(1, -2, img.width + 3, img.height + 3, ui.ops.theme.HIGHLIGHT_COL); // TODO
					let off: i32 = row % 2 == 1 ? 1 : 0;
					let w: i32 = 50;
					if (config_raw.window_scale > 1) {
						w += math_floor(config_raw.window_scale * 2);
					}
					ui_fill(-1,         -2, w + 3,       2, ui.ops.theme.HIGHLIGHT_COL);
					ui_fill(-1,    w - off, w + 3, 2 + off, ui.ops.theme.HIGHLIGHT_COL);
					ui_fill(-1,         -2,     2,   w + 3, ui.ops.theme.HIGHLIGHT_COL);
					ui_fill(w + 1,      -2,     2,   w + 4, ui.ops.theme.HIGHLIGHT_COL);
				}

				let uix: f32 = ui._x;
				let tile: i32 = ui_SCALE(ui) > 1 ? 100 : 50;
				let state: ui_state_t = ui_state_t.IDLE;
				if (project_fonts[i].preview_ready) {
					// g2_set_pipeline(pipe); // L8
					// g4_set_int(channel_location, 1);
					state = _ui_image(img);
					// g2_set_pipeline(null);
				}
				else {
					state = _ui_image(resource_get("icons.k"), -1, -1.0, tile * 6, tile, tile, tile);
				}

				if (state == ui_state_t.STARTED) {
					if (context_raw.font != project_fonts[i]) {
						_tab_fonts_draw_i = i;

						app_notify_on_init(function () {
							let i: i32 = _tab_fonts_draw_i;

							context_select_font(i);
						});
					}
					if (time_time() - context_raw.select_time < 0.25) {
						ui_base_show_2d_view(view_2d_type_t.FONT);
					}
					context_raw.select_time = time_time();
				}
				if (ui.is_hovered && ui.input_released_r) {
					context_select_font(i);
					_tab_fonts_draw_i = i;

					ui_menu_draw(function (ui: ui_t) {
						let i: i32 = _tab_fonts_draw_i;

						if (project_fonts.length > 1 && ui_menu_button(ui, tr("Delete"), "delete") && project_fonts[i].file != "") {
							tab_fonts_delete_font(project_fonts[i]);
						}
					});
				}
				if (ui.is_hovered) {
					if (img == null) {
						_tab_fonts_draw_i = i;

						app_notify_on_init(function () {
							let i: i32 = _tab_fonts_draw_i;

							let _font: slot_font_t = context_raw.font;
							context_raw.font = project_fonts[i];
							util_render_make_font_preview();
							context_raw.font = _font;
						});
					}
					else {
						_ui_tooltip_image(img);
						ui_tooltip(project_fonts[i].name);
					}
				}

				if (config_raw.show_asset_names) {
					ui._x = uix;
					ui._y += slotw * 0.9;
					ui_text(project_fonts[i].name, ui_align_t.CENTER);
					if (ui.is_hovered) {
						ui_tooltip(project_fonts[i].name);
					}
					ui._y -= slotw * 0.9;
					if (i == project_fonts.length - 1) {
						ui._y += j == num - 1 ? imgw : imgw + ui_ELEMENT_H(ui) + ui_ELEMENT_OFFSET(ui);
					}
				}
			}

			ui._y += 6;
		}

		let in_focus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
							 ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
		if (in_focus && ui.is_delete_down && project_fonts.length > 1 && context_raw.font.file != "") {
			ui.is_delete_down = false;
			tab_fonts_delete_font(context_raw.font);
		}
	}
}

function tab_fonts_delete_font(font: slot_font_t) {
	app_notify_on_init(function (font: slot_font_t) {
		let i: i32 = array_index_of(project_fonts, font);
		context_select_font(i == project_fonts.length - 1 ? i - 1 : i + 1);
		data_delete_font(project_fonts[i].file);
		array_splice(project_fonts, i, 1);
	}, font);

	ui_base_hwnds[2].redraws = 2;
}
