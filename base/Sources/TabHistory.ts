
///if (is_paint || is_sculpt)

class TabHistory {

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		if (zui_tab(htab, tr("History"))) {
			for (let i = 0; i < History.steps.length; ++i) {
				let active = History.steps.length - 1 - History.redos;
				if (i == active) {
					zui_fill(0, 0, ui._window_w, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}
				zui_text(History.steps[i].name);
				if (ui.is_released) { // Jump to undo step
					let diff = i - active;
					while (diff > 0) {
						diff--;
						History.redo();
					}
					while (diff < 0) {
						diff++;
						History.undo();
					}
				}
				zui_fill(0, 0, (ui._window_w / zui_SCALE(ui) - 2), 1 * zui_SCALE(ui), ui.t.SEPARATOR_COL);
			}
		}
	}
}

///end
