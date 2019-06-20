package arm.ui;

import zui.Id;

class TabPlugins{

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, "Plugins")) {
			if (ui.panel(Id.handle({selected: false}), "Console", 1)) {
				arm.plugin.Console.render(ui);
			}
			ui.separator();

			// Draw plugins
			for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
		}
	}
}
