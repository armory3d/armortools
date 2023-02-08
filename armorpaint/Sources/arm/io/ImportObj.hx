package arm.io;

import kha.Blob;
import iron.data.Data;
import arm.format.ObjParser;
import arm.ui.UISidebar;

class ImportObj {

	public static function run(path: String, replaceExisting = true) {
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
					if (obj.udims[i].length == 0) continue;
					var u = i % obj.udimsU;
					var v = Std.int(i / obj.udimsU);
					obj.name = name + "." + (1000 + v * 10 + u + 1);
					obj.inda = obj.udims[i];
					i == 0 ? (replaceExisting ? ImportMesh.makeMesh(obj, path) : ImportMesh.addMesh(obj)) : ImportMesh.addMesh(obj);
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
								var voff = Std.int(posa0.length / 4);
								// Repack merged positions
								var posa32 = new kha.arrays.Float32Array(Std.int(posa0.length / 4) * 3 + Std.int(posa1.length / 4) * 3);
								for (k in 0...Std.int(posa0.length / 4)) {
									posa32[k * 3    ] = posa0[k * 4    ] / 32767 * parts[i].scalePos;
									posa32[k * 3 + 1] = posa0[k * 4 + 1] / 32767 * parts[i].scalePos;
									posa32[k * 3 + 2] = posa0[k * 4 + 2] / 32767 * parts[i].scalePos;
								}
								for (k in 0...Std.int(posa1.length / 4)) {
									posa32[voff * 3 + k * 3    ] = posa1[k * 4    ] / 32767 * parts[j].scalePos;
									posa32[voff * 3 + k * 3 + 1] = posa1[k * 4 + 1] / 32767 * parts[j].scalePos;
									posa32[voff * 3 + k * 3 + 2] = posa1[k * 4 + 2] / 32767 * parts[j].scalePos;
								}
								var scalePos = 0.0;
								for (k in 0...posa32.length) {
									var f = Math.abs(posa32[k]);
									if (scalePos < f) scalePos = f;
								}
								var inv = 32767 * (1 / scalePos);
								var posa = new kha.arrays.Int16Array(posa0.length + posa1.length);
								for (k in 0...Std.int(posa.length / 4)) {
									posa[k * 4    ] = Std.int(posa32[k * 3    ] * inv);
									posa[k * 4 + 1] = Std.int(posa32[k * 3 + 1] * inv);
									posa[k * 4 + 2] = Std.int(posa32[k * 3 + 2] * inv);
								}
								for (k in 0...Std.int(posa0.length / 4)) posa[k * 4 + 3] = posa0[k * 4 + 3];
								for (k in 0...Std.int(posa1.length / 4)) posa[posa0.length + k * 4 + 3] = posa1[k * 4 + 3];
								// Merge normals and uvs
								var nora = new kha.arrays.Int16Array(nora0.length + nora1.length);
								var texa = (texa0 != null && texa1 != null) ? new kha.arrays.Int16Array(texa0.length + texa1.length) : null;
								var inda = new kha.arrays.Uint32Array(inda0.length + inda1.length);
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
								parts[i].scalePos = scalePos;
								parts.splice(j, 1);
							}
							else j++;
						}
					}
				}
				replaceExisting ? ImportMesh.makeMesh(parts[0], path) : ImportMesh.addMesh(parts[0]);
				for (i in 1...parts.length) {
					ImportMesh.addMesh(parts[i]);
				}
			}
			Data.deleteBlob(path);
		});
	}
}
