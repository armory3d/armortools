
function import_exr(path) {
	let b = data_get_blob(path);
	data_delete_blob(path);
	return io_exr_parse(b);
}

let plugin = plugin_create();
path_texture_importers_set("exr", import_exr);
plugin_notify_on_delete(plugin, function() {
	path_texture_importers_delete("exr");
});
