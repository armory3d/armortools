
class ImportGpl {

	static run = (path: string, replaceExisting: bool) => {
		data_get_blob(path, (b: ArrayBuffer) => {
			// let swatches = [];

			// let str = sys_buffer_to_string(b);
			// let lines = str.split("\n");

			// let view = new DataView(b);
			// // GIMP's color palette importer: https://gitlab.gnome.org/GNOME/gimp/-/blob/gimp-2-10/app/core/gimppalette-load.c#L39
			// if (!lines[0].startsWith("GIMP Palette")) {
			// 	Console.error(tr("Not a valid GIMP color palette"));
			// 	return;
			// }

			// let delimiter = ~/\s+/ig;
			// for (let line of lines) {
			// 	if (line.startsWith("Name:")) continue;
			// 	else if (line.startsWith("Columns:")) continue;
			// 	else if (line.startsWith("#")) continue;
			// 	else {
			// 		let tokens = delimiter.split(line);
			// 		if (tokens.length < 3) continue;
			// 		let swatch = Project.makeSwatch(Color.fromBytes(String(tokens[0]), String(tokens[1]), String(tokens[2])));
			// 		swatches.push(swatch);
			// 	}
			// }

			// if (replaceExisting) {
			// 	Project.raw.swatches = [];

			// 	if (swatches.length == 0) { // No swatches contained
			// 		Project.raw.swatches.push(Project.makeSwatch());
			// 	}
			// }

			// if (swatches.length > 0) {
			// 	for (let s of swatches) {
			// 		Project.raw.swatches.push(s);
			// 	}
			// }
		});
	}
}
