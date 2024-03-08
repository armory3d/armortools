
///if (is_paint || is_sculpt)

class TabFonts {

	static draw = (htab: zui_handle_t) => {
		let ui: zui_t = UIBase.ui;
		let statush: i32 = Config.raw.layout[layout_size_t.STATUS_H];
		if (zui_tab(htab, tr("Fonts")) && statush > UIStatus.default_status_h * zui_SCALE(ui)) {

			zui_begin_sticky();
			if (Config.raw.touch_ui) {
				zui_row([1 / 4, 1 / 4]);
			}
			else {
				zui_row([1 / 14, 1 / 14]);
			}

			if (zui_button(tr("Import"))) Project.import_asset("ttf,ttc,otf");
			if (ui.is_hovered) zui_tooltip(tr("Import font file"));

			if (zui_button(tr("2D View"))) {
				UIBase.show_2d_view(view_2d_type_t.FONT);
			}
			zui_end_sticky();
			zui_separator(3, false);

			let statusw: i32 = sys_width() - UIToolbar.toolbar_w - Config.raw.layout[layout_size_t.SIDEBAR_W];
			let slotw: i32 = Math.floor(51 * zui_SCALE(ui));
			let num: i32 = Math.floor(statusw / slotw);

			for (let row: i32 = 0; row < Math.floor(Math.ceil(Project.fonts.length / num)); ++row) {
				let mult: i32 = Config.raw.show_asset_names ? 2 : 1;
				let ar: f32[] = [];
				for (let i: i32 = 0; i < num * mult; ++i) ar.push(1 / num);
				zui_row(ar);

				ui._x += 2;
				let off: f32 = Config.raw.show_asset_names ? zui_ELEMENT_OFFSET(ui) * 10.0 : 6;
				if (row > 0) ui._y += off;

				for (let j: i32 = 0; j < num; ++j) {
					let imgw: i32 = Math.floor(50 * zui_SCALE(ui));
					let i: i32 = j + row * num;
					if (i >= Project.fonts.length) {
						zui_end_element(imgw);
						if (Config.raw.show_asset_names) zui_end_element(0);
						continue;
					}
					let img: image_t = Project.fonts[i].image;

					if (Context.raw.font == Project.fonts[i]) {
						// Zui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
						let off: i32 = row % 2 == 1 ? 1 : 0;
						let w: i32 = 50;
						if (Config.raw.window_scale > 1) w += Math.floor(Config.raw.window_scale * 2);
						zui_fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						zui_fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					let uix: f32 = ui._x;
					let tile: i32 = zui_SCALE(ui) > 1 ? 100 : 50;
					let state: zui_state_t = zui_state_t.IDLE;
					if (Project.fonts[i].preview_ready) {
						// g2_set_pipeline(UIView2D.pipe); // L8
						// ///if krom_opengl
						// g4_set_pipeline(UIView2D.pipe);
						// ///end
						// g4_set_int(UIView2D.channelLocation, 1);
						state = zui_image(img);
						// g2_set_pipeline(null);
					}
					else {
						state = zui_image(Res.get("icons.k"), -1, -1.0, tile * 6, tile, tile, tile);
					}

					if (state == zui_state_t.STARTED) {
						if (Context.raw.font != Project.fonts[i]) {
							let _init = () => {
								Context.select_font(i);
							}
							app_notify_on_init(_init);
						}
						if (time_time() - Context.raw.select_time < 0.25) UIBase.show_2d_view(view_2d_type_t.FONT);
						Context.raw.select_time = time_time();
					}
					if (ui.is_hovered && ui.input_released_r) {
						Context.select_font(i);
						let add: i32 = Project.fonts.length > 1 ? 1 : 0;
						UIMenu.draw((ui: zui_t) => {
							if (Project.fonts.length > 1 && UIMenu.menu_button(ui, tr("Delete"), "delete") && Project.fonts[i].file != "") {
								TabFonts.delete_font(Project.fonts[i]);
							}
						}, 0 + add);
					}
					if (ui.is_hovered) {
						if (img == null) {
							app_notify_on_init(() => {
								let _font: SlotFontRaw = Context.raw.font;
								Context.raw.font = Project.fonts[i];
								UtilRender.make_font_preview();
								Context.raw.font = _font;
							});
						}
						else {
							zui_tooltip_image(img);
							zui_tooltip(Project.fonts[i].name);
						}
					}

					if (Config.raw.show_asset_names) {
						ui._x = uix;
						ui._y += slotw * 0.9;
						zui_text(Project.fonts[i].name, zui_align_t.CENTER);
						if (ui.is_hovered) zui_tooltip(Project.fonts[i].name);
						ui._y -= slotw * 0.9;
						if (i == Project.fonts.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui);
						}
					}
				}

				ui._y += 6;
			}

			let in_focus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						    	 ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (in_focus && ui.is_delete_down && Project.fonts.length > 1 && Context.raw.font.file != "") {
				ui.is_delete_down = false;
				TabFonts.delete_font(Context.raw.font);
			}
		}
	}

	static delete_font = (font: SlotFontRaw) => {
		let i: i32 = Project.fonts.indexOf(font);
		let _init = () => {
			Context.select_font(i == Project.fonts.length - 1 ? i - 1 : i + 1);
			data_delete_font(Project.fonts[i].file);
			Project.fonts.splice(i, 1);
		}
		app_notify_on_init(_init);
		UIBase.hwnds[2].redraws = 2;
	}
}

///end
