package arm.io;

import kha.Blob;
import iron.data.Data;
import iron.format.GltfParser;

class ImportGltf {

	public static function run(path:String) {
		Data.getBlob(path, function(b:Blob) {
			var obj = new GltfParser(b);
			Importer.makeMesh(obj, path);
			Data.deleteBlob(path);
		});
	}
}
