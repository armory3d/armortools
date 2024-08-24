
function import_svg(path) {
	let b = data_get_blob(path);
	data_delete_blob(path);
	return io_svg_parse(b);
}

let plugin = plugin_create();
path_texture_importers_set("svg", import_svg);
plugin_notify_on_delete(plugin, function() {
	path_texture_importers_delete("svg");
});
