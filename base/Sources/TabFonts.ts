
///if (is_paint || is_sculpt)

class TabFonts {

	static draw = (htab: Handle) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (ui.tab(htab, tr("Fonts")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			ui.beginSticky();
			if (Config.raw.touch_ui) {
				ui.row([1 / 4, 1 / 4]);
			}
			else {
				ui.row([1 / 14, 1 / 14]);
			}

			if (ui.button(tr("Import"))) Project.importAsset("ttf,ttc,otf");
			if (ui.isHovered) ui.tooltip(tr("Import font file"));

			if (ui.button(tr("2D View"))) {
				UIBase.show2DView(View2DType.View2DFont);
			}
			ui.endSticky();
			ui.separator(3, false);

			let statusw = System.width - UIToolbar.toolbarw - Config.raw.layout[LayoutSize.LayoutSidebarW];
			let slotw = Math.floor(51 * ui.SCALE());
			let num = Math.floor(statusw / slotw);

			for (let row = 0; row < Math.floor(Math.ceil(Project.fonts.length / num)); ++row) {
				let mult = Config.raw.show_asset_names ? 2 : 1;
				let ar = [];
				for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
				ui.row(ar);

				ui._x += 2;
				let off = Config.raw.show_asset_names ? ui.ELEMENT_OFFSET() * 10.0 : 6;
				if (row > 0) ui._y += off;

				for (let j = 0; j < num; ++j) {
					let imgw = Math.floor(50 * ui.SCALE());
					let i = j + row * num;
					if (i >= Project.fonts.length) {
						ui.endElement(imgw);
						if (Config.raw.show_asset_names) ui.endElement(0);
						continue;
					}
					let img = Project.fonts[i].image;

					if (Context.raw.font == Project.fonts[i]) {
						// ui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
						let off = row % 2 == 1 ? 1 : 0;
						let w = 50;
						if (Config.raw.window_scale > 1) w += Math.floor(Config.raw.window_scale * 2);
						ui.fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						ui.fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						ui.fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						ui.fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					let uix = ui._x;
					let tile = ui.SCALE() > 1 ? 100 : 50;
					let state = State.Idle;
					if (Project.fonts[i].previewReady) {
						// ui.g.pipeline = UIView2D.pipe; // L8
						// ///if krom_opengl
						// ui.currentWindow.texture.g4.setPipeline(UIView2D.pipe);
						// ///end
						// ui.currentWindow.texture.g4.setInt(UIView2D.channelLocation, 1);
						state = ui.image(img);
						// ui.g.pipeline = null;
					}
					else {
						state = ui.image(Res.get("icons.k"), -1, null, tile * 6, tile, tile, tile);
					}

					if (state == State.Started) {
						if (Context.raw.font != Project.fonts[i]) {
							let _init = () => {
								Context.selectFont(i);
							}
							App.notifyOnInit(_init);
						}
						if (Time.time() - Context.raw.selectTime < 0.25) UIBase.show2DView(View2DType.View2DFont);
						Context.raw.selectTime = Time.time();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						Context.selectFont(i);
						let add = Project.fonts.length > 1 ? 1 : 0;
						let fontName = Project.fonts[i].name;
						UIMenu.draw((ui: Zui) => {
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
							ui.tooltipImage(img);
							ui.tooltip(Project.fonts[i].name);
						}
					}

					if (Config.raw.show_asset_names) {
						ui._x = uix;
						ui._y += slotw * 0.9;
						ui.text(Project.fonts[i].name, Align.Center);
						if (ui.isHovered) ui.tooltip(Project.fonts[i].name);
						ui._y -= slotw * 0.9;
						if (i == Project.fonts.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + ui.ELEMENT_H() + ui.ELEMENT_OFFSET();
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
