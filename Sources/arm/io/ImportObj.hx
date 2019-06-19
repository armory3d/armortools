package arm.io;

import arm.ui.UITrait;

class ImportObj {

	public static function run(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			if (UITrait.inst.isUdim) {
				var obj = new iron.format.obj.ObjParser(b, 0, UITrait.inst.isUdim);
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
				var obj = new iron.format.obj.ObjParser(b);
				Importer.makeMesh(obj, path);
				while (obj.hasNext) {
					obj = new iron.format.obj.ObjParser(b, obj.pos);
					Importer.addMesh(obj);
				}
			}
			iron.data.Data.deleteBlob(path);
		});
	}
}
