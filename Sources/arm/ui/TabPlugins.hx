package arm.ui;

import arm.App.tr;

class TabPlugins {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, tr("Plugins"))) {

			ui.row([1 / 4]);
			if (ui.button(tr("Manager"))) {
				BoxPreferences.htab.position = 5; // Plugins
				BoxPreferences.show();
			}

			// Draw plugins
			for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
		}
	}
}
