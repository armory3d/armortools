
function base_ext_init() {
	sim_init();
}

function base_ext_render() {
    if (context_raw.frame == 2) {
		util_render_make_material_preview();
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;

		base_init_undo_layers();
    }

	sim_update();
}

function base_ext_init_config(raw: config_t) {
    raw.layer_res = texture_res_t.RES8192;
}
