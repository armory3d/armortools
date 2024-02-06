
///if (is_paint || is_sculpt)

class TabBrushes {

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		if (zui_tab(htab, tr("Brushes"))) {
			zui_begin_sticky();
			zui_row([1 / 4, 1 / 4, 1 / 4]);
			if (zui_button(tr("New"))) {
				Context.raw.brush = SlotBrush.create();
				Project.brushes.push(Context.raw.brush);
				MakeMaterial.parseBrush();
				UINodes.hwnd.redraws = 2;
			}
			if (zui_button(tr("Import"))) {
				Project.importBrush();
			}
			if (zui_button(tr("Nodes"))) {
				UIBase.showBrushNodes();
			}
			zui_end_sticky();
			zui_separator(3, false);

			let slotw = Math.floor(51 * zui_SCALE(ui));
			let num = Math.floor(Config.raw.layout[LayoutSize.LayoutSidebarW] / slotw);

			for (let row = 0; row < Math.floor(Math.ceil(Project.brushes.length / num)); ++row) {
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
					if (i >= Project.brushes.length) {
						zui_end_element(imgw);
						if (Config.raw.show_asset_names) zui_end_element(0);
						continue;
					}
					let img = zui_SCALE(ui) > 1 ? Project.brushes[i].image : Project.brushes[i].imageIcon;
					let imgFull = Project.brushes[i].image;

					if (Context.raw.brush == Project.brushes[i]) {
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
					//let uiy = ui._y;
					let tile = zui_SCALE(ui) > 1 ? 100 : 50;
					let state = Project.brushes[i].previewReady ? zui_image(img) : zui_image(Res.get("icons.k"), -1, null, tile * 5, tile, tile, tile);
					if (state == State.Started) {
						if (Context.raw.brush != Project.brushes[i]) Context.selectBrush(i);
						if (time_time() - Context.raw.selectTime < 0.25) UIBase.showBrushNodes();
						Context.raw.selectTime = time_time();
						// app_drag_off_x = -(mouse_x - uix - ui._windowX - 3);
						// app_drag_off_y = -(mouse_y - uiy - ui._windowY + 1);
						// app_drag_brush = Context.raw.brush;
					}
					if (ui.is_hovered && ui.input_released_r) {
						Context.selectBrush(i);
						let add = Project.brushes.length > 1 ? 1 : 0;
						UIMenu.draw((ui: zui_t) => {
							//let b = Project.brushes[i];

							if (UIMenu.menuButton(ui, tr("Export"))) {
								Context.selectBrush(i);
								BoxExport.showBrush();
							}

							if (UIMenu.menuButton(ui, tr("Duplicate"))) {
								let _init = () => {
									Context.raw.brush = SlotBrush.create();
									Project.brushes.push(Context.raw.brush);
									let cloned = JSON.parse(JSON.stringify(Project.brushes[i].canvas));
									Context.raw.brush.canvas = cloned;
									Context.setBrush(Context.raw.brush);
									UtilRender.makeBrushPreview();
								}
								app_notify_on_init(_init);
							}

							if (Project.brushes.length > 1 && UIMenu.menuButton(ui, tr("Delete"), "delete")) {
								TabBrushes.deleteBrush(Project.brushes[i]);
							}
						}, 2 + add);
					}

					if (ui.is_hovered) {
						if (imgFull == null) {
							app_notify_on_init(() => {
								let _brush = Context.raw.brush;
								Context.raw.brush = Project.brushes[i];
								MakeMaterial.parseBrush();
								UtilRender.makeBrushPreview();
								Context.raw.brush = _brush;
							});
						}
						else {
							zui_tooltip_image(imgFull);
							zui_tooltip(Project.brushes[i].canvas.name);
						}
					}

					if (Config.raw.show_asset_names) {
						ui._x = uix;
						ui._y += slotw * 0.9;
						zui_text(Project.brushes[i].canvas.name, Align.Center);
						if (ui.is_hovered) zui_tooltip(Project.brushes[i].canvas.name);
						ui._y -= slotw * 0.9;
						if (i == Project.brushes.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui);
						}
					}
				}

				ui._y += 6;
			}

			let inFocus = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (inFocus && ui.is_delete_down && Project.brushes.length > 1) {
				ui.is_delete_down = false;
				TabBrushes.deleteBrush(Context.raw.brush);
			}
		}
	}

	static deleteBrush = (b: SlotBrushRaw) => {
		let i = Project.brushes.indexOf(b);
		Context.selectBrush(i == Project.brushes.length - 1 ? i - 1 : i + 1);
		Project.brushes.splice(i, 1);
		UIBase.hwnds[1].redraws = 2;
	}
}

///end
