package arm.io;

class ImportFont {

	public static function run(path:String) {
		iron.data.Data.getFont(path, function(font:kha.Font) {
			var ar = path.split("/");
			ar = ar[ar.length - 1].split("\\");
			var name = ar[ar.length - 1];
			Importer.fontList.push(name);
			Importer.fontMap.set(name, font);
		});
	}
}
