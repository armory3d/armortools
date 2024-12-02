
function base_ext_init() {
    random_node_set_seed(math_floor(time_time() * 4294967295));
}

function base_ext_render() {
    if (context_raw.frame == 2) {
		app_notify_on_next_frame(function () {
			app_notify_on_next_frame(function () {
				tab_meshes_set_default_mesh(".Sphere");
			});
		});
    }
}

function base_ext_init_config(raw: config_t) {
	raw.displace_strength = 1.0;
	raw.workspace = space_type_t.SPACE2D;
	raw.node_preview = false;
}
