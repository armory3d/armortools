
function import_usd(path) {
	let b = data_get_blob(path);
	data_delete_blob(path);
	return io_usd_parse(b);
}

let plugin = plugin_create();
path_mesh_importers_set("usdc", import_usd);
plugin_notify_on_delete(plugin, function() {
	path_mesh_importers_delete("usdc");
});
