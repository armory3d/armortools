package arm.io;

import kha.Blob;
import iron.data.Data;
import arm.format.FbxParser;
import arm.ui.UITrait;

class ImportFbx {

	public static function run(path: String) {
		Data.getBlob(path, function(b: Blob) {
			FbxParser.parseTransform = UITrait.inst.parseTransform;
			var obj = new FbxParser(b);
			ImportMesh.makeMesh(obj, path);
			while (obj.next()) {
				ImportMesh.addMesh(obj);
			}
			Data.deleteBlob(path);
		});
	}
}
