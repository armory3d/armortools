package arm.io;

import kha.Font;
import iron.data.Data;

class ImportFont {

	public static function run(path:String) {
		Data.getFont(path, function(font:Font) {
			var ar = path.split("/");
			ar = ar[ar.length - 1].split("\\");
			var name = ar[ar.length - 1];
			Importer.fontList.push(name);
			Importer.fontMap.set(name, font);
		});
	}
}
