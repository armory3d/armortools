
class TabPlugins {

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		if (zui_tab(htab, tr("Plugins"))) {

			zui_begin_sticky();

			///if (is_paint || is_sculpt)
			zui_row([1 / 4]);
			///end
			///if is_lab
			zui_row([1 / 14]);
			///end

			if (zui_button(tr("Manager"))) {
				BoxPreferences.htab.position = 6; // Plugins
				BoxPreferences.show();
			}
			zui_end_sticky();

			// Draw plugins
			for (let p of Plugin.plugins.values()) if (p.drawUI != null) p.drawUI(ui);
		}
	}
}
