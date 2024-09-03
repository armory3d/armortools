
let plugin = plugin_create();

function unwrap_mesh(mesh) {
	proc_xatlas_unwrap(mesh);
}

// let h1 = ui_handle_create();
// plugin_notify_on_ui(plugin, function() {
// 	if (ui_panel(h1, "UV Unwrap")) {
// 		if (ui_button("Unwrap Mesh")) {
// 			for (let po of project_paint_objects) {
// 				unwrap_mesh(mesh);
// 				mesh_data_build();
// 			}
// 			util_mesh_merge_mesh();
// 		}
// 	}
// });

util_mesh_unwrappers_set("uv_unwrap.js", unwrap_mesh);
plugin_notify_on_delete(plugin, function() {
	util_mesh_unwrappers_delete("uv_unwrap.js");
});
