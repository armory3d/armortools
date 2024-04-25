
function tab_plugins_draw(htab: zui_handle_t) {
	let ui: zui_t = ui_base_ui;
	if (zui_tab(htab, tr("Plugins"))) {

		zui_begin_sticky();

		///if (is_paint || is_sculpt)
		let row: f32[] = [1 / 4];
		zui_row(row);
		///end
		///if is_lab
		let row: f32[] = [1 / 14];
		zui_row(row);
		///end

		if (zui_button(tr("Manager"))) {
			box_preferences_htab.position = 6; // Plugins
			box_preferences_show();
		}
		zui_end_sticky();

		// Draw plugins
		let keys: string[] = map_keys(plugin_map);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let p: plugin_t = map_get(plugin_map, keys[i]);
			if (p.draw_ui != null) {
				p.draw_ui(ui);
			}
		}
	}
}
