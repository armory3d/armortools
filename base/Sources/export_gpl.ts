
function export_gpl_run(path: string, name: string, swatches: swatch_color_t[]) {
	let o: string = "";
	o += "GIMP Palette\n";
	o += "Name: " + name + "\n";
	o += "# armorpaint.org\n";
	o += "#\n";

	for (let swatch of swatches) {
		o += any_to_string(color_get_rb(swatch.base)) + " " + any_to_string(color_get_gb(swatch.base)) + " " + any_to_string(color_get_bb(swatch.base)) + "\n";
	}

	krom_file_save_bytes(path, sys_string_to_buffer(o), o.length);
}
