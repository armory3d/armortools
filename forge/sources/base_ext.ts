
function base_ext_init() {
	sim_init();

	transform_move(project_paint_objects[0].base.transform, vec4_z_axis(), -999); // Move default cube away
	tab_scene_new_object("box.arm");
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
	raw.window_w = 1920;
	raw.window_h = 1080;
	raw.atlas_res = texture_res_t.RES8192;
	raw.undo_steps = 1;
}

function base_ext_update() {
	if (keyboard_down("control") && keyboard_started("d")) {
		sim_duplicate();
	}

	if (keyboard_started("delete")) {
		sim_delete();
	}
}
