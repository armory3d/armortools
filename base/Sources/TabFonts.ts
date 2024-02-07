
///if (is_paint || is_sculpt)

class TabFonts {

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (zui_tab(htab, tr("Fonts")) && statush > UIStatus.defaultStatusH * zui_SCALE(ui)) {

			zui_begin_sticky();
			if (Config.raw.touch_ui) {
				zui_row([1 / 4, 1 / 4]);
			}
			else {
				zui_row([1 / 14, 1 / 14]);
			}

			if (zui_button(tr("Import"))) Project.importAsset("ttf,ttc,otf");
			if (ui.is_hovered) zui_tooltip(tr("Import font file"));

			if (zui_button(tr("2D View"))) {
				UIBase.show2DView(View2DType.View2DFont);
			}
			zui_end_sticky();
			zui_separator(3, false);

			let statusw = sys_width() - UIToolbar.toolbarw - Config.raw.layout[LayoutSize.LayoutSidebarW];
			let slotw = Math.floor(51 * zui_SCALE(ui));
			let num = Math.floor(statusw / slotw);

			for (let row = 0; row < Math.floor(Math.ceil(Project.fonts.length / num)); ++row) {
				let mult = Config.raw.show_asset_names ? 2 : 1;
				let ar = [];
				for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
				zui_row(ar);

				ui._x += 2;
				let off = Config.raw.show_asset_names ? zui_ELEMENT_OFFSET(ui) * 10.0 : 6;
				if (row > 0) ui._y += off;

				for (let j = 0; j < num; ++j) {
					let imgw = Math.floor(50 * zui_SCALE(ui));
					let i = j + row * num;
					if (i >= Project.fonts.length) {
						zui_end_element(imgw);
						if (Config.raw.show_asset_names) zui_end_element(0);
						continue;
					}
					let img = Project.fonts[i].image;

					if (Context.raw.font == Project.fonts[i]) {
						// Zui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
						let off = row % 2 == 1 ? 1 : 0;
						let w = 50;
						if (Config.raw.window_scale > 1) w += Math.floor(Config.raw.window_scale * 2);
						zui_fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						zui_fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					let uix = ui._x;
					let tile = zui_SCALE(ui) > 1 ? 100 : 50;
					let state = State.Idle;
					if (Project.fonts[i].previewReady) {
						// g2_set_pipeline(UIView2D.pipe); // L8
						// ///if krom_opengl
						// g4_set_pipeline(UIView2D.pipe);
						// ///end
						// g4_set_int(UIView2D.channelLocation, 1);
						state = zui_image(img);
						// g2_set_pipeline(null);
					}
					else {
						state = zui_image(Res.get("icons.k"), -1, null, tile * 6, tile, tile, tile);
					}

					if (state == State.Started) {
						if (Context.raw.font != Project.fonts[i]) {
							let _init = () => {
								Context.selectFont(i);
							}
							app_notify_on_init(_init);
						}
						if (time_time() - Context.raw.selectTime < 0.25) UIBase.show2DView(View2DType.View2DFont);
						Context.raw.selectTime = time_time();
					}
					if (ui.is_hovered && ui.input_released_r) {
						Context.selectFont(i);
						let add = Project.fonts.length > 1 ? 1 : 0;
						let fontName = Project.fonts[i].name;
						UIMenu.draw((ui: zui_t) => {
							if (Project.fonts.length > 1 && UIMenu.menuButton(ui, tr("Delete"), "delete") && Project.fonts[i].file != "") {
								TabFonts.deleteFont(Project.fonts[i]);
							}
						}, 0 + add);
					}
					if (ui.is_hovered) {
						if (img == null) {
							app_notify_on_init(() => {
								let _font = Context.raw.font;
								Context.raw.font = Project.fonts[i];
								UtilRender.makeFontPreview();
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
						zui_text(Project.fonts[i].name, Align.Center);
						if (ui.is_hovered) zui_tooltip(Project.fonts[i].name);
						ui._y -= slotw * 0.9;
						if (i == Project.fonts.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui);
						}
					}
				}

				ui._y += 6;
			}

			let inFocus = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (inFocus && ui.is_delete_down && Project.fonts.length > 1 && Context.raw.font.file != "") {
				ui.is_delete_down = false;
				TabFonts.deleteFont(Context.raw.font);
			}
		}
	}

	static deleteFont = (font: SlotFontRaw) => {
		let i = Project.fonts.indexOf(font);
		let _init = () => {
			Context.selectFont(i == Project.fonts.length - 1 ? i - 1 : i + 1);
			data_delete_font(Project.fonts[i].file);
			Project.fonts.splice(i, 1);
		}
		app_notify_on_init(_init);
		UIBase.hwnds[2].redraws = 2;
	}
}

///end
