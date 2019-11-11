package arm.io;

import kha.Blob;
import iron.data.Data;
import arm.format.StlParser;
import arm.ui.UITrait;
using StringTools;

class ImportStl {

	public static function run(path:String) {
		Data.getBlob(path, function(b:Blob) {
			var obj = new StlParser(b);
			var p = path.replace("\\", "/");
			var index = p.lastIndexOf("/");
			obj.name = p.substring(index >= 0 ? index + 1 : 0, p.length - 4);
			ImportMesh.makeMesh(obj, path);
			Data.deleteBlob(path);
		});
	}
}
