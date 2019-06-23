package arm.ui;

class TabHistory {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, "History")) {
			for (i in 0...History.stack.length) {
				var active = History.stack.length - 1 - History.redos;
				if (i == active) {
					ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}
				ui.text(History.stack[i]);
				if (ui.isReleased) { // Jump to undo step
					var diff = i - active;
					while (diff > 0) { diff--; History.doRedo(); }
					while (diff < 0) { diff++; History.doUndo(); }
				}
			}
		}
	}
}
