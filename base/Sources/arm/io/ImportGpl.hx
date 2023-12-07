package arm.io;

import js.lib.ArrayBuffer;
import js.lib.DataView;
import kha.Color;
import iron.data.Data;

class ImportGpl {

	public static function run(path: String, replaceExisting: Bool) {
		Data.getBlob(path, function(b: ArrayBuffer) {
			var swatches = [];

			var str = kha.System.bufferToString(b);
			var lines = str.split("\n");

			var view = new DataView(b);
			// GIMP's color palette importer: https://gitlab.gnome.org/GNOME/gimp/-/blob/gimp-2-10/app/core/gimppalette-load.c#L39
			if (!lines[0].startsWith("GIMP Palette")) {
				Console.error(tr("Not a valid GIMP color palette"));
				return;
			}

			var delimiter = ~/\s+/ig;
			for (line in lines) {
				if (line.startsWith("Name:")) continue;
				else if (line.startsWith("Columns:")) continue;
				else if (line.startsWith("#")) continue;
				else {
					var tokens = delimiter.split(line);
					if (tokens.length < 3) continue;
					var swatch = Project.makeSwatch(Color.fromBytes(Std.parseInt(tokens[0]), Std.parseInt(tokens[1]), Std.parseInt(tokens[2])));
					swatches.push(swatch);
				}
			}

			if (replaceExisting) {
				Project.raw.swatches = [];

				if (swatches.length == 0) { // No swatches contained
					Project.raw.swatches.push(Project.makeSwatch());
				}
			}

			if (swatches.length > 0) {
				for (s in swatches) {
					Project.raw.swatches.push(s);
				}
			}
		});
	}
}
