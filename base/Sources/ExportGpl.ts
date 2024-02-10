
class ExportGpl {

	static run = (path: string, name: string, swatches: TSwatchColor[]) => {
		let o = "";
		o += "GIMP Palette\n";
		o += "Name: " + name + "\n";
		o += "# armorpaint.org\n";
		o += "#\n";

		for (let swatch of swatches) {
			o += String(color_get_rb(swatch.base)) + " " + String(color_get_gb(swatch.base)) + " " + String(color_get_bb(swatch.base)) + "\n";
		}

		krom_file_save_bytes(path, sys_string_to_buffer(o), o.length);
	}
}
