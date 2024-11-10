
function base_ext_init() {
}

function base_ext_render() {
    if (context_raw.frame == 2) {
		util_render_make_material_preview();
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;

		if (history_undo_layers == null) {
			history_undo_layers = [];
			for (let i: i32 = 0; i < config_raw.undo_steps; ++i) {
				let len: i32 = history_undo_layers.length;
				let ext: string = "_undo" + len;
				let l: slot_layer_t = slot_layer_create(ext);
				array_push(history_undo_layers, l);
			}
		}

        ///if is_sculpt
		app_notify_on_next_frame(function () {
			app_notify_on_next_frame(function () {
				context_raw.project_type = project_model_t.SPHERE;
				project_new();
			});
		});
		///end
    }
}
