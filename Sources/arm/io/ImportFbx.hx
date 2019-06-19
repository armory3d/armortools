package arm.io;

import arm.ui.UITrait;

class ImportFbx {

	public static function run(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			iron.format.fbx.FbxParser.parseTransform = UITrait.inst.parseTransform;
			var obj = new iron.format.fbx.FbxParser(b);
			Importer.makeMesh(obj, path);
			while (obj.next()) {
				Importer.addMesh(obj);
			}
			iron.data.Data.deleteBlob(path);
		});
	}
}
