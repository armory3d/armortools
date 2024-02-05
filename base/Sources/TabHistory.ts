
///if (is_paint || is_sculpt)

class TabHistory {

	static draw = (htab: HandleRaw) => {
		let ui = UIBase.ui;
		if (Zui.tab(htab, tr("History"))) {
			for (let i = 0; i < History.steps.length; ++i) {
				let active = History.steps.length - 1 - History.redos;
				if (i == active) {
					Zui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}
				Zui.text(History.steps[i].name);
				if (ui.isReleased) { // Jump to undo step
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
				Zui.fill(0, 0, (ui._windowW / Zui.SCALE(ui) - 2), 1 * Zui.SCALE(ui), ui.t.SEPARATOR_COL);
			}
		}
	}
}

///end
