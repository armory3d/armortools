package arm.ui;

class TabPlugins {

	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab, tr("Plugins"))) {

			ui.beginSticky();
			ui.row([1 / 4]);
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
