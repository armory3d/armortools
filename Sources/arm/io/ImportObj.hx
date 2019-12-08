package arm.io;

import kha.Blob;
import iron.data.Data;
import arm.format.ObjParser;
import arm.ui.UITrait;

class ImportObj {

	public static function run(path: String) {
		var i = UITrait.inst.splitBy;
		var isUdim = i == SplitUdim;
		ObjParser.splitCode =
			(i == SplitObject || isUdim) ? "o".code :
			 i == SplitGroup 			 ? "g".code :
			 				 			   "u".code; // usemtl
		Data.getBlob(path, function(b: Blob) {
			if (isUdim) {
				var obj = new ObjParser(b, 0, isUdim);
				var name = obj.name;
				for (i in 0...obj.udims.length) {
					var u = i % obj.udimsU;
					var v = Std.int(i / obj.udimsU);
					obj.name = name + "." + (1000 + v * 10 + u + 1);
					obj.inda = obj.udims[i];
					i == 0 ? ImportMesh.makeMesh(obj, path) : ImportMesh.addMesh(obj);
				}
			}
			else {
				var obj = new ObjParser(b);
				ImportMesh.makeMesh(obj, path);
				while (obj.hasNext) {
					obj = new ObjParser(b, obj.pos);
					ImportMesh.addMesh(obj);
				}
			}
			Data.deleteBlob(path);
		});
	}
}
