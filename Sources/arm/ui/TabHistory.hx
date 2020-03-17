package arm.ui;

import arm.App.tr;

class TabHistory {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, tr("History"))) {
			for (i in 0...History.steps.length) {
				var active = History.steps.length - 1 - History.redos;
				if (i == active) {
					ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}
				ui.text(History.steps[i].name);
				if (ui.isReleased) { // Jump to undo step
					var diff = i - active;
					while (diff > 0) { diff--; History.redo(); }
					while (diff < 0) { diff++; History.undo(); }
				}
			}
		}
	}
}
