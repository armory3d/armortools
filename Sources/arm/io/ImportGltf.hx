package arm.io;

class ImportGltf {

	public static function run(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.gltf.GltfParser(b);
			Importer.makeMesh(obj, path);
			iron.data.Data.deleteBlob(path);
		});
	}
}
