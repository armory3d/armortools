
function export_gpl(path, name, swatches) {
	let o: string = "";
	o += "GIMP Palette\n";
	o += "Name: " + name + "\n";
	o += "# armorpaint.org\n";
	o += "#\n";

	for (let i = 0; i < swatches.length; ++i) {
		let swatch = swatches[i];
		let rb = color_get_rb(swatch.base);
		let gb = color_get_gb(swatch.base);
		let bb = color_get_bb(swatch.base);
		o += rb + " " + gb + " " + bb + "\n";
	}

	iron_file_save_bytes(path, sys_string_to_buffer(o), o.length);
}

let plugin = plugin_create();
path_swatch_exporters_set("gpl", export_gpl);
plugin_notify_on_delete(plugin, function() {
	path_swatch_exporters_delete("gpl");
});
