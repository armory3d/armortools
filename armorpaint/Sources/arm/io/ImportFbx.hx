package arm.io;

import kha.Blob;
import iron.data.Data;
import arm.format.FbxParser;
import arm.ui.UISidebar;

class ImportFbx {

	public static function run(path: String, replaceExisting = true) {
		Data.getBlob(path, function(b: Blob) {
			FbxParser.parseTransform = Context.parseTransform;
			FbxParser.parseVCols = Context.parseVCols;
			var obj = new FbxParser(b);
			replaceExisting ? ImportMesh.makeMesh(obj, path) : ImportMesh.addMesh(obj);
			while (obj.next()) {
				ImportMesh.addMesh(obj);
			}
			Data.deleteBlob(path);
		});
	}
}
