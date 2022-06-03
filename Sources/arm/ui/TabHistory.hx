package arm.ui;

class TabHistory {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab0, tr("History"))) {
			for (i in 0...History.steps.length) {
				var step = History.steps[i];
				
				#if !arm_debug
				if (step.internal) {
					continue;
				}
				#end
				
				var active = History.steps.length - 1 - History.redos;
				if (i == active) {
					ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}

				#if arm_debug
				var formatted_name = '${step.name} (${step.num_children})';
				if (step.internal)
					formatted_name = '<internal>' + formatted_name;
				ui.text(formatted_name);
				#else
				ui.text(step.name);
				#end
				
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
