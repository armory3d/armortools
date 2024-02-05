
class TabPlugins {

	static draw = (htab: HandleRaw) => {
		let ui = UIBase.ui;
		if (Zui.tab(htab, tr("Plugins"))) {

			Zui.beginSticky();

			///if (is_paint || is_sculpt)
			Zui.row([1 / 4]);
			///end
			///if is_lab
			Zui.row([1 / 14]);
			///end

			if (Zui.button(tr("Manager"))) {
				BoxPreferences.htab.position = 6; // Plugins
				BoxPreferences.show();
			}
			Zui.endSticky();

			// Draw plugins
			for (let p of Plugin.plugins.values()) if (p.drawUI != null) p.drawUI(ui);
		}
	}
}
