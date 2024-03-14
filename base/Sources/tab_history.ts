
///if (is_paint || is_sculpt)

function tab_history_draw(htab: zui_handle_t) {
	let ui: zui_t = ui_base_ui;
	if (zui_tab(htab, tr("History"))) {
		for (let i: i32 = 0; i < history_steps.length; ++i) {
			let active: i32 = history_steps.length - 1 - history_redos;
			if (i == active) {
				zui_fill(0, 0, ui._window_w, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
			}
			zui_text(history_steps[i].name);
			if (ui.is_released) { // Jump to undo step
				let diff: i32 = i - active;
				while (diff > 0) {
					diff--;
					history_redo();
				}
				while (diff < 0) {
					diff++;
					history_undo();
				}
			}
			zui_fill(0, 0, (ui._window_w / zui_SCALE(ui) - 2), 1 * zui_SCALE(ui), ui.t.SEPARATOR_COL);
		}
	}
}

///end
