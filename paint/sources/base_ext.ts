
function base_ext_init() {
}

function base_ext_render() {
    if (context_raw.frame == 2) {
		util_render_make_material_preview();
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;

		base_init_undo_layers();
    }

    if (context_raw.tool == tool_type_t.GIZMO) {
		sim_init();
		sim_update();
    }
}

function base_ext_init_config(raw: config_t) {

}

function base_ext_update() {
	if (context_raw.tool == tool_type_t.GIZMO) {
		if (keyboard_down("control") && keyboard_started("d")) {
			sim_duplicate();
		}

		if (keyboard_started("delete")) {
			sim_delete();
		}
	}
}
