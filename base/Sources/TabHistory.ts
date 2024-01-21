
///if (is_paint || is_sculpt)

class TabHistory {

	static draw = (htab: Handle) => {
		let ui = UIBase.ui;
		if (ui.tab(htab, tr("History"))) {
			for (let i = 0; i < History.steps.length; ++i) {
				let active = History.steps.length - 1 - History.redos;
				if (i == active) {
					ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}
				ui.text(History.steps[i].name);
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
				ui.fill(0, 0, (ui._windowW / ui.SCALE() - 2), 1 * ui.SCALE(), ui.t.SEPARATOR_COL);
			}
		}
	}
}

///end
