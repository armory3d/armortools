package arm.io;

import kha.Blob;
import iron.data.Data;
import arm.format.ObjParser;
import arm.ui.UITrait;

class ImportObj {

	public static function run(path:String) {
		var i = UITrait.inst.splitBy;
		ObjParser.splitCode = i == 0 ? "o".code : i == 1 ? "g".code : "u".code; // object, group, usemtl
		Data.getBlob(path, function(b:Blob) {
			if (UITrait.inst.isUdim) {
				var obj = new ObjParser(b, 0, UITrait.inst.isUdim);
				var name = obj.name;
				for (i in 0...obj.udims.length) {
					var u = i % obj.udimsU;
					var v = Std.int(i / obj.udimsU);
					obj.name = name + "." + (1000 + v * 10 + u + 1);
					obj.inda = obj.udims[i];
					i == 0 ? Importer.makeMesh(obj, path) : Importer.addMesh(obj);
				}
			}
			else {
				var obj = new ObjParser(b);
				Importer.makeMesh(obj, path);
				while (obj.hasNext) {
					obj = new ObjParser(b, obj.pos);
					Importer.addMesh(obj);
				}
			}
			Data.deleteBlob(path);
		});
	}
}
