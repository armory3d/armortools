package arm.io;

import kha.Blob;
import iron.data.Data;
import iron.format.fbx.FbxParser;
import arm.ui.UITrait;

class ImportFbx {

	public static function run(path:String) {
		Data.getBlob(path, function(b:Blob) {
			FbxParser.parseTransform = UITrait.inst.parseTransform;
			var obj = new FbxParser(b);
			Importer.makeMesh(obj, path);
			while (obj.next()) {
				Importer.addMesh(obj);
			}
			Data.deleteBlob(path);
		});
	}
}
