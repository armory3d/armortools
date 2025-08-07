
function ui_header_draw_tool_properties(ui: ui_t) {
	if (context_raw.tool == workspace_tool_t.GIZMO) {

        if (!sim_running && ui_button("Play")) {
			sim_play();
			context_raw.selected_object = scene_camera.base;
		}

		if (sim_running && ui_button("Stop")) {
			sim_stop();
		}

        let h_record: ui_handle_t = ui_handle(__ID__);
        sim_record = ui_check(h_record, tr("Record"));
	}

	else if (context_raw.tool == workspace_tool_t.FILL) {
		let brush_scale_handle: ui_handle_t = ui_handle(__ID__);
		if (brush_scale_handle.init) {
			brush_scale_handle.value = context_raw.brush_scale;
		}
		context_raw.brush_scale = ui_slider(brush_scale_handle, tr("UV Scale"), 0.01, 5.0, true);
	}
}
