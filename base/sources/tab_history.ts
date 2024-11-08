
function tab_history_draw(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	if (ui_tab(htab, tr("History"))) {
		for (let i: i32 = 0; i < history_steps.length; ++i) {
			let active: i32 = history_steps.length - 1 - history_redos;
			if (i == active) {
				ui_fill(0, 0, ui._window_w, ui.ops.theme.ELEMENT_H, ui.ops.theme.HIGHLIGHT_COL);
			}
			ui_text(history_steps[i].name);
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
			ui_fill(0, 0, (ui._window_w / ui_SCALE(ui) - 2), 1 * ui_SCALE(ui), ui.ops.theme.SEPARATOR_COL);
		}
	}
}
