
class TabPlugins {

	static draw = (htab: Handle) => {
		let ui = UIBase.inst.ui;
		if (ui.tab(htab, tr("Plugins"))) {

			ui.beginSticky();

			///if (is_paint || is_sculpt)
			ui.row([1 / 4]);
			///end
			///if is_lab
			ui.row([1 / 14]);
			///end

			if (ui.button(tr("Manager"))) {
				BoxPreferences.htab.position = 6; // Plugins
				BoxPreferences.show();
			}
			ui.endSticky();

			// Draw plugins
			for (let p of Plugin.plugins.values()) if (p.drawUI != null) p.drawUI(ui);
		}
	}
}
