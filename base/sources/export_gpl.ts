
function export_gpl_run(path: string, name: string, swatches: swatch_color_t[]) {
	let o: string = "";
	o += "GIMP Palette\n";
	o += "Name: " + name + "\n";
	o += "# armorpaint.org\n";
	o += "#\n";

	for (let i: i32 = 0; i < swatches.length; ++i) {
		let swatch: swatch_color_t = swatches[i];
		let rb: i32 = color_get_rb(swatch.base);
		let gb: i32 = color_get_gb(swatch.base);
		let bb: i32 = color_get_bb(swatch.base);
		o += rb + " " + gb + " " + bb + "\n";
	}

	iron_file_save_bytes(path, sys_string_to_buffer(o), o.length);
}
