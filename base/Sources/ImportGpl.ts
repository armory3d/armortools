
class ImportGpl {

	static run = (path: string, replace_existing: bool) => {
		let b: ArrayBuffer = data_get_blob(path);
		// let swatches: TSwatchColor[] = [];

		// let str: string = sys_buffer_to_string(b);
		// let lines: string[] = str.split("\n");

		// let view: DataView = new DataView(b);
		// // GIMP's color palette importer: https://gitlab.gnome.org/GNOME/gimp/-/blob/gimp-2-10/app/core/gimppalette-load.c#L39
		// if (!lines[0].startsWith("GIMP Palette")) {
		// 	console_error(tr("Not a valid GIMP color palette"));
		// 	return;
		// }

		// let delimiter: string = ~/\s+/ig;
		// for (let line of lines) {
		// 	if (line.startsWith("Name:")) continue;
		// 	else if (line.startsWith("Columns:")) continue;
		// 	else if (line.startsWith("#")) continue;
		// 	else {
		// 		let tokens: string[] = delimiter.split(line);
		// 		if (tokens.length < 3) continue;
		// 		let swatch: TSwatchColor = makeSwatch(Color.fromBytes(String(tokens[0]), String(tokens[1]), String(tokens[2])));
		// 		swatches.push(swatch);
		// 	}
		// }

		// if (replaceExisting) {
		// 	raw.swatches = [];

		// 	if (swatches.length == 0) { // No swatches contained
		// 		raw.swatches.push(makeSwatch());
		// 	}
		// }

		// if (swatches.length > 0) {
		// 	for (let s of swatches) {
		// 		raw.swatches.push(s);
		// 	}
		// }
	}
}
