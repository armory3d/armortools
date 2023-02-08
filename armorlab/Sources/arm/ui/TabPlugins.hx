package arm.ui;

import arm.Enums;

class TabPlugins {

	public static function draw() {
		var ui = UISidebar.inst.ui;
		var statush = Config.raw.layout[LayoutStatusH];
		if (ui.tab(UIStatus.inst.statustab, tr("Plugins")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			ui.beginSticky();
			ui.row([1 / 14]);
			if (ui.button(tr("Manager"))) {
				BoxPreferences.htab.position = 6; // Plugins
				BoxPreferences.show();
			}
			ui.endSticky();

			// Draw plugins
			for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
		}
	}
}
