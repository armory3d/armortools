package arm.io;

import haxe.io.BytesInput;
import kha.Color;
import kha.Blob;
import iron.data.Data;

class ImportGpl {

	public static function run(path: String, replaceExisting: Bool) {
		Data.getBlob(path, function(b: Blob) {
			var swatches = [];
			try {
				var input = new BytesInput(b.bytes);
				// GIMP's color palette importer: https://gitlab.gnome.org/GNOME/gimp/-/blob/gimp-2-10/app/core/gimppalette-load.c#L39
				if (!input.readLine().startsWith("GIMP Palette")) {
					Console.error(tr("Not a valid GIMP color palette"));
					return;
				}

				var delimiter = ~/\s+/ig;
				while (true) {
					var line = input.readLine();
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
			}
			catch (e: haxe.io.Eof) {
				// Is thrown if end of file is reached
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
