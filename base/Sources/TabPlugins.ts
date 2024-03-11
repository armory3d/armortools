
class TabPlugins {

	static draw = (htab: zui_handle_t) => {
		let ui: zui_t = ui_base_ui;
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
			for (let p of plugin_map.values()) if (p.draw_ui != null) p.draw_ui(ui);
		}
	}
}
