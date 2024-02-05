
///if (is_paint || is_sculpt)

class TabFonts {

	static draw = (htab: HandleRaw) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (Zui.tab(htab, tr("Fonts")) && statush > UIStatus.defaultStatusH * Zui.SCALE(ui)) {

			Zui.beginSticky();
			if (Config.raw.touch_ui) {
				Zui.row([1 / 4, 1 / 4]);
			}
			else {
				Zui.row([1 / 14, 1 / 14]);
			}

			if (Zui.button(tr("Import"))) Project.importAsset("ttf,ttc,otf");
			if (ui.isHovered) Zui.tooltip(tr("Import font file"));

			if (Zui.button(tr("2D View"))) {
				UIBase.show2DView(View2DType.View2DFont);
			}
			Zui.endSticky();
			Zui.separator(3, false);

			let statusw = sys_width() - UIToolbar.toolbarw - Config.raw.layout[LayoutSize.LayoutSidebarW];
			let slotw = Math.floor(51 * Zui.SCALE(ui));
			let num = Math.floor(statusw / slotw);

			for (let row = 0; row < Math.floor(Math.ceil(Project.fonts.length / num)); ++row) {
				let mult = Config.raw.show_asset_names ? 2 : 1;
				let ar = [];
				for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
				Zui.row(ar);

				ui._x += 2;
				let off = Config.raw.show_asset_names ? Zui.ELEMENT_OFFSET(ui) * 10.0 : 6;
				if (row > 0) ui._y += off;

				for (let j = 0; j < num; ++j) {
					let imgw = Math.floor(50 * Zui.SCALE(ui));
					let i = j + row * num;
					if (i >= Project.fonts.length) {
						Zui.endElement(imgw);
						if (Config.raw.show_asset_names) Zui.endElement(0);
						continue;
					}
					let img = Project.fonts[i].image;

					if (Context.raw.font == Project.fonts[i]) {
						// Zui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
						let off = row % 2 == 1 ? 1 : 0;
						let w = 50;
						if (Config.raw.window_scale > 1) w += Math.floor(Config.raw.window_scale * 2);
						Zui.fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						Zui.fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						Zui.fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						Zui.fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					let uix = ui._x;
					let tile = Zui.SCALE(ui) > 1 ? 100 : 50;
					let state = State.Idle;
					if (Project.fonts[i].previewReady) {
						// ui.g.pipeline = UIView2D.pipe; // L8
						// ///if krom_opengl
						// ui.currentWindow.texture.g4.setPipeline(UIView2D.pipe);
						// ///end
						// ui.currentWindow.texture.g4.setInt(UIView2D.channelLocation, 1);
						state = Zui.image(img);
						// ui.g.pipeline = null;
					}
					else {
						state = Zui.image(Res.get("icons.k"), -1, null, tile * 6, tile, tile, tile);
					}

					if (state == State.Started) {
						if (Context.raw.font != Project.fonts[i]) {
							let _init = () => {
								Context.selectFont(i);
							}
							App.notifyOnInit(_init);
						}
						if (time_time() - Context.raw.selectTime < 0.25) UIBase.show2DView(View2DType.View2DFont);
						Context.raw.selectTime = time_time();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						Context.selectFont(i);
						let add = Project.fonts.length > 1 ? 1 : 0;
						let fontName = Project.fonts[i].name;
						UIMenu.draw((ui: ZuiRaw) => {
							if (Project.fonts.length > 1 && UIMenu.menuButton(ui, tr("Delete"), "delete") && Project.fonts[i].file != "") {
								TabFonts.deleteFont(Project.fonts[i]);
							}
						}, 0 + add);
					}
					if (ui.isHovered) {
						if (img == null) {
							App.notifyOnInit(() => {
								let _font = Context.raw.font;
								Context.raw.font = Project.fonts[i];
								UtilRender.makeFontPreview();
								Context.raw.font = _font;
							});
						}
						else {
							Zui.tooltipImage(img);
							Zui.tooltip(Project.fonts[i].name);
						}
					}

					if (Config.raw.show_asset_names) {
						ui._x = uix;
						ui._y += slotw * 0.9;
						Zui.text(Project.fonts[i].name, Align.Center);
						if (ui.isHovered) Zui.tooltip(Project.fonts[i].name);
						ui._y -= slotw * 0.9;
						if (i == Project.fonts.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + Zui.ELEMENT_H(ui) + Zui.ELEMENT_OFFSET(ui);
						}
					}
				}

				ui._y += 6;
			}

			let inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (inFocus && ui.isDeleteDown && Project.fonts.length > 1 && Context.raw.font.file != "") {
				ui.isDeleteDown = false;
				TabFonts.deleteFont(Context.raw.font);
			}
		}
	}

	static deleteFont = (font: SlotFontRaw) => {
		let i = Project.fonts.indexOf(font);
		let _init = () => {
			Context.selectFont(i == Project.fonts.length - 1 ? i - 1 : i + 1);
			Data.deleteFont(Project.fonts[i].file);
			Project.fonts.splice(i, 1);
		}
		App.notifyOnInit(_init);
		UIBase.hwnds[2].redraws = 2;
	}
}

///end
