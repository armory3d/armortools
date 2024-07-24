
function import_gpl_run(path: string, replace_existing: bool) {
	let b: buffer_t = data_get_blob(path);
	// let swatches: TSwatchColor[] = [];

	// let str: string = sys_buffer_to_string(b);
	// let lines: string[] = string_split(str, "\n");

	// // GIMP's color palette importer: https://gitlab.gnome.org/GNOME/gimp/-/blob/gimp-2-10/app/core/gimppalette-load.c#L39
	// if (!starts_with(lines[0], "GIMP Palette")) {
	// 	console_error(tr("Not a valid GIMP color palette"));
	// 	return;
	// }

	// let delimiter: string = ~/\s+/ig;
	// for (let line of lines) {
	// 	if (starts_with(line, "Name:")) continue;
	// 	else if (starts_with(line, "Columns:")) continue;
	// 	else if (starts_with(line, "#")) continue;
	// 	else {
	// 		let tokens: string[] = string_split(delimiter, line);
	// 		if (tokens.length < 3) continue;
	// 		let swatch: TSwatchColor = makeSwatch(Color.fromBytes(tokens[0], tokens[1], tokens[2]));
	// 		array_push(swatches, swatch);
	// 	}
	// }

	// if (replaceExisting) {
	// 	raw.swatches = [];

	// 	if (swatches.length == 0) { // No swatches contained
	// 		array_push(raw.swatches, makeSwatch());
	// 	}
	// }

	// if (swatches.length > 0) {
	// 	for (let s of swatches) {
	// 		array_push(raw.swatches, s);
	// 	}
	// }
}
