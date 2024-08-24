
function import_gltf_glb(path) {
	let b = data_get_blob(path);
	data_delete_blob(path);
	return io_gltf_parse(b);
}

let plugin = plugin_create();
path_mesh_importers_set("gltf", import_gltf_glb);
path_mesh_importers_set("glb", import_gltf_glb);
plugin_notify_on_delete(plugin, function() {
	path_mesh_importers_delete("gltf");
	path_mesh_importers_delete("glb");
});
