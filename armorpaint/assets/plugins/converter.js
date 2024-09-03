
let plugin = plugin_create();
let h1 = ui_handle_create();

plugin_notify_on_ui(plugin, function() {
	if (ui_panel(h1, "Converter")) {
		ui_row([1 / 2, 1 / 2]);
		if (ui_button(".arm to .json")) {
			ui_files_show("arm", false, true, function(path) {
				let b = data_get_blob(path);
				let parsed = armpack_decode(b);
				let out = string_to_buffer(JSON.stringify(parsed, null, "	"));
				iron_file_save_bytes(path.substr(0, path.length - 3) + "json", out);
			});
		}
		if (ui_button(".json to .arm")) {
			ui_files_show("json", false, true, function(path) {
				let b = data_get_blob(path);
				let parsed = JSON.parse(buffer_to_string(b));
				let out = armpack_encode(parsed);
				iron_file_save_bytes(path.substr(0, path.length - 4) + "arm", out);
			});
		}
	}
});


