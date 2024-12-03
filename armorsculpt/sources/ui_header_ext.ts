
function ui_header_draw_tool_properties(ui: ui_t) {
	if (context_raw.tool == workspace_tool_t.BRUSH) {
		context_raw.brush_radius = ui_slider(context_raw.brush_radius_handle, tr("Radius"), 0.01, 2.0, true);
		if (ui.is_hovered) {
			let vars: map_t<string, string> = map_create();
			map_set(vars, "brush_radius", map_get(config_keymap, "brush_radius"));
			map_set(vars, "brush_radius_decrease", map_get(config_keymap, "brush_radius_decrease"));
			map_set(vars, "brush_radius_increase", map_get(config_keymap, "brush_radius_increase"));
			ui_tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", vars));
		}
	}
}
