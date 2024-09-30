
function import_txt(path) {
	let b = data_get_blob(path);
	let filename = path.split("\\").pop().split("/").pop();
	ui_box_show_message(filename, buffer_to_string(b));
	data_delete_blob(path);
}

let plugin = plugin_create();
path_texture_importers_set("txt", import_txt);
plugin_notify_on_delete(plugin, function() {
	path_texture_importers_delete("txt");
});
