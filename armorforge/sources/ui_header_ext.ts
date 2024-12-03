
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
        let record: bool = ui_check(h_record, tr("Record"));
	}
}
