
let import_txt = function(path) {
	let b = data_get_blob(path);
	var filename = path.split('\\').pop().split('/').pop();
	try {
		ui_box_show_message(filename, sys_buffer_to_string(b), true);
		ui_box_click_to_hide = false;
		data_delete_blob(path);
	}
	catch(e) {
		console.error(e);
	}
}

let plugin = plugin_create();
let formats = path_texture_formats;
let importers = path_texture_importers;
formats.push("txt");
importers.set("txt", import_txt);

plugin.delete = function() {
	formats.splice(formats.indexOf("txt"), 1);
	importers.delete("txt");
};
