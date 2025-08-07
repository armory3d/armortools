
function import_fbx(path) {
	let b = data_get_blob(path);
	data_delete_blob(path);
	return io_fbx_parse(b);
}

let plugin = plugin_create();
path_mesh_importers_set("fbx", import_fbx);
plugin_notify_on_delete(plugin, function() {
	path_mesh_importers_delete("fbx");
});
