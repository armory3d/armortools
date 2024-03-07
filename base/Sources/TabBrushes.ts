
///if (is_paint || is_sculpt)

class TabBrushes {

	static draw = (htab: zui_handle_t) => {
		let ui: zui_t = UIBase.ui;
		if (zui_tab(htab, tr("Brushes"))) {
			zui_begin_sticky();
			zui_row([1 / 4, 1 / 4, 1 / 4]);
			if (zui_button(tr("New"))) {
				Context.raw.brush = SlotBrush.create();
				Project.brushes.push(Context.raw.brush);
				MakeMaterial.parse_brush();
				UINodes.hwnd.redraws = 2;
			}
			if (zui_button(tr("Import"))) {
				Project.import_brush();
			}
			if (zui_button(tr("Nodes"))) {
				UIBase.show_brush_nodes();
			}
			zui_end_sticky();
			zui_separator(3, false);

			let slotw: i32 = Math.floor(51 * zui_SCALE(ui));
			let num: i32 = Math.floor(Config.raw.layout[layout_size_t.SIDEBAR_W] / slotw);

			for (let row: i32 = 0; row < Math.floor(Math.ceil(Project.brushes.length / num)); ++row) {
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
					if (i >= Project.brushes.length) {
						zui_end_element(imgw);
						if (Config.raw.show_asset_names) zui_end_element(0);
						continue;
					}
					let img: image_t = zui_SCALE(ui) > 1 ? Project.brushes[i].image : Project.brushes[i].image_icon;
					let imgFull: image_t = Project.brushes[i].image;

					if (Context.raw.brush == Project.brushes[i]) {
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
					//let uiy: f32 = ui._y;
					let tile: i32 = zui_SCALE(ui) > 1 ? 100 : 50;
					let state: zui_state_t = Project.brushes[i].preview_ready ? zui_image(img) : zui_image(Res.get("icons.k"), -1, -1.0, tile * 5, tile, tile, tile);
					if (state == zui_state_t.STARTED) {
						if (Context.raw.brush != Project.brushes[i]) Context.select_brush(i);
						if (time_time() - Context.raw.select_time < 0.25) UIBase.show_brush_nodes();
						Context.raw.select_time = time_time();
						// app_drag_off_x = -(mouse_x - uix - ui._windowX - 3);
						// app_drag_off_y = -(mouse_y - uiy - ui._windowY + 1);
						// app_drag_brush = Context.raw.brush;
					}
					if (ui.is_hovered && ui.input_released_r) {
						Context.select_brush(i);
						let add: i32 = Project.brushes.length > 1 ? 1 : 0;
						UIMenu.draw((ui: zui_t) => {
							//let b: SlotBrushRaw = Project.brushes[i];

							if (UIMenu.menu_button(ui, tr("Export"))) {
								Context.select_brush(i);
								BoxExport.show_brush();
							}

							if (UIMenu.menu_button(ui, tr("Duplicate"))) {
								let _init = () => {
									Context.raw.brush = SlotBrush.create();
									Project.brushes.push(Context.raw.brush);
									let cloned: any = json_parse(json_stringify(Project.brushes[i].canvas));
									Context.raw.brush.canvas = cloned;
									Context.set_brush(Context.raw.brush);
									UtilRender.make_brush_preview();
								}
								app_notify_on_init(_init);
							}

							if (Project.brushes.length > 1 && UIMenu.menu_button(ui, tr("Delete"), "delete")) {
								TabBrushes.delete_brush(Project.brushes[i]);
							}
						}, 2 + add);
					}

					if (ui.is_hovered) {
						if (imgFull == null) {
							app_notify_on_init(() => {
								let _brush: SlotBrushRaw = Context.raw.brush;
								Context.raw.brush = Project.brushes[i];
								MakeMaterial.parse_brush();
								UtilRender.make_brush_preview();
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
						zui_text(Project.brushes[i].canvas.name, zui_align_t.CENTER);
						if (ui.is_hovered) zui_tooltip(Project.brushes[i].canvas.name);
						ui._y -= slotw * 0.9;
						if (i == Project.brushes.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui);
						}
					}
				}

				ui._y += 6;
			}

			let inFocus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  		ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (inFocus && ui.is_delete_down && Project.brushes.length > 1) {
				ui.is_delete_down = false;
				TabBrushes.delete_brush(Context.raw.brush);
			}
		}
	}

	static delete_brush = (b: SlotBrushRaw) => {
		let i: i32 = Project.brushes.indexOf(b);
		Context.select_brush(i == Project.brushes.length - 1 ? i - 1 : i + 1);
		Project.brushes.splice(i, 1);
		UIBase.hwnds[1].redraws = 2;
	}
}

///end
