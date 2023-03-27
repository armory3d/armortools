package arm.ui;

import zui.Zui;

class TabPlugins {

	public static function draw(htab: Handle) {
		var ui = UIBase.inst.ui;
		var statush = Config.raw.layout[LayoutStatusH];
		if (ui.tab(htab, tr("Plugins")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

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
