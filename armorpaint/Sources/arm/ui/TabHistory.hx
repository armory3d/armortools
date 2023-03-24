package arm.ui;

import zui.Zui;

class TabHistory {

	@:access(zui.Zui)
	public static function draw(htab: Handle) {
		var ui = UIBase.inst.ui;
		if (ui.tab(htab, tr("History"))) {
			for (i in 0...History.steps.length) {
				var active = History.steps.length - 1 - History.redos;
				if (i == active) {
					ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}
				ui.text(History.steps[i].name);
				if (ui.isReleased) { // Jump to undo step
					var diff = i - active;
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
