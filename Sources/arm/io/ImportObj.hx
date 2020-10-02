package arm.io;

import kha.Blob;
import iron.data.Data;
import arm.format.ObjParser;
import arm.ui.UISidebar;

class ImportObj {

	public static function run(path: String) {
		var i = Context.splitBy;
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
				var parts: Array<ObjParser> = [];
				var obj = new ObjParser(b);
				parts.push(obj);
				while (obj.hasNext) {
					obj = new ObjParser(b, obj.pos);
					parts.push(obj);
				}
				if (Context.splitBy == SplitMaterial) {
					var posa0;
					var posa1;
					var nora0;
					var nora1;
					var texa0;
					var texa1;
					var inda0;
					var inda1;
					// Merge to single object per material
					for (i in 0...parts.length) {
						var j = i + 1;
						while (j < parts.length) {
							if (parts[i].name == parts[j].name) {
								posa0 = parts[i].posa;
								posa1 = parts[j].posa;
								nora0 = parts[i].nora;
								nora1 = parts[j].nora;
								texa0 = parts[i].texa != null ? parts[i].texa : null;
								texa1 = parts[j].texa != null ? parts[j].texa : null;
								inda0 = parts[i].inda;
								inda1 = parts[j].inda;
								var posa = new kha.arrays.Int16Array(posa0.length + posa1.length);
								var nora = new kha.arrays.Int16Array(nora0.length + nora1.length);
								var texa = (texa0 != null && texa1 != null) ? new kha.arrays.Int16Array(texa0.length + texa1.length) : null;
								var inda = new kha.arrays.Uint32Array(inda0.length + inda1.length);
								var voff = Std.int(posa0.length / 4);
								js.Syntax.code("posa.set(posa0)");
								js.Syntax.code("posa.set(posa1, posa0.length)");
								js.Syntax.code("nora.set(nora0)");
								js.Syntax.code("nora.set(nora1, nora0.length)");
								if (texa != null) {
									js.Syntax.code("texa.set(texa0)");
									js.Syntax.code("texa.set(texa1, texa0.length)");
								}
								js.Syntax.code("inda.set(inda0)");
								for (k in 0...inda1.length) inda[k + inda0.length] = inda1[k] + voff;
								parts[i].posa = posa;
								parts[i].nora = nora;
								parts[i].texa = texa;
								parts[i].inda = inda;
								parts.splice(j, 1);
							}
							else j++;
						}
					}
				}
				ImportMesh.makeMesh(parts[0], path);
				for (i in 1...parts.length) {
					ImportMesh.addMesh(parts[i]);
				}
			}
			Data.deleteBlob(path);
		});
	}
}
