package arm;

import iron.Data;

class ImportFbx {

	public static function run(path: String, replaceExisting = true) {
		Data.getBlob(path, function(b: js.lib.ArrayBuffer) {
			ParserFbx.parseTransform = Context.raw.parseTransform;
			ParserFbx.parseVCols = Context.raw.parseVCols;
			var obj = new ParserFbx(b);
			replaceExisting ? ImportMesh.makeMesh(obj, path) : ImportMesh.addMesh(obj);
			while (obj.next()) {
				ImportMesh.addMesh(obj);
			}
			Data.deleteBlob(path);
		});
	}
}
