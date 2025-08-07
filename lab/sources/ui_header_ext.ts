
function ui_header_draw_tool_properties(ui: ui_t) {
	if (context_raw.tool == tool_type_t.PICKER) {

	}
	else if (context_raw.tool == tool_type_t.ERASER ||
			 context_raw.tool == tool_type_t.CLONE  ||
			 context_raw.tool == tool_type_t.BLUR   ||
			 context_raw.tool == tool_type_t.SMUDGE) {

		let nodes: ui_nodes_t = ui_nodes_get_nodes();
		let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
		let inpaint: bool = nodes.nodes_selected_id.length > 0 && ui_get_node(canvas.nodes, nodes.nodes_selected_id[0]).type == "inpaint_node";
		if (inpaint) {
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
}
