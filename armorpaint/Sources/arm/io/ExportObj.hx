package arm.io;

import haxe.io.Bytes;
import haxe.io.BytesOutput;
import kha.arrays.Int16Array;
import iron.object.MeshObject;

class ExportObj {

	public static function run(path: String, paintObjects: Array<MeshObject>, applyDisplacement = false) {
		var o = new BytesOutput();
		o.bigEndian = false;
		o.writeString("# armorpaint.org\n");

		var poff = 0;
		var noff = 0;
		var toff = 0;
		for (p in paintObjects) {
			var mesh = p.data.raw;
			var inv = 1 / 32767;
			var sc = p.data.scalePos * inv;
			var posa = mesh.vertex_arrays[0].values;
			var nora = mesh.vertex_arrays[1].values;
			var texa = mesh.vertex_arrays[2].values;
			var len = Std.int(posa.length / 4);

			// Merge shared vertices and remap indices
			var posa2 = new Int16Array(len * 3);
			var nora2 = new Int16Array(len * 3);
			var texa2 = new Int16Array(len * 2);
			var posmap = new Map<Int, Int>();
			var normap = new Map<Int, Int>();
			var texmap = new Map<Int, Int>();

			var pi = 0;
			var ni = 0;
			var ti = 0;
			for (i in 0...len) {
				var found = false;
				for (j in 0...pi) {
					if (posa2[j * 3    ] == posa[i * 4    ] &&
						posa2[j * 3 + 1] == posa[i * 4 + 1] &&
						posa2[j * 3 + 2] == posa[i * 4 + 2]) {
						posmap.set(i, j);
						found = true;
						break;
					}
				}
				if (!found) {
					posmap.set(i, pi);
					posa2[pi * 3    ] = posa[i * 4    ];
					posa2[pi * 3 + 1] = posa[i * 4 + 1];
					posa2[pi * 3 + 2] = posa[i * 4 + 2];
					pi++;
				}

				found = false;
				for (j in 0...ni) {
					if (nora2[j * 3    ] == nora[i * 2    ] &&
						nora2[j * 3 + 1] == nora[i * 2 + 1] &&
						nora2[j * 3 + 2] == posa[i * 4 + 3]) {
						normap.set(i, j);
						found = true;
						break;
					}
				}
				if (!found) {
					normap.set(i, ni);
					nora2[ni * 3    ] = nora[i * 2    ];
					nora2[ni * 3 + 1] = nora[i * 2 + 1];
					nora2[ni * 3 + 2] = posa[i * 4 + 3];
					ni++;
				}

				found = false;
				for (j in 0...ti) {
					if (texa2[j * 2    ] == texa[i * 2    ] &&
						texa2[j * 2 + 1] == texa[i * 2 + 1]) {
						texmap.set(i, j);
						found = true;
						break;
					}
				}
				if (!found) {
					texmap.set(i, ti);
					texa2[ti * 2    ] = texa[i * 2    ];
					texa2[ti * 2 + 1] = texa[i * 2 + 1];
					ti++;
				}
			}

			if (applyDisplacement) {
				// var height = Project.layers[0].texpaint_pack.getPixels();
				// var res = Project.layers[0].texpaint_pack.width;
				// var strength = 0.1;
				// for (i in 0...len) {
				// 	var x = Std.int(texa2[i * 2    ] / 32767 * res);
				// 	var y = Std.int((1.0 - texa2[i * 2 + 1] / 32767) * res);
				// 	var h = (1.0 - height.get((y * res + x) * 4 + 3) / 255) * strength;
				// 	posa2[i * 3    ] -= Std.int(nora2[i * 3    ] * inv * h / sc);
				// 	posa2[i * 3 + 1] -= Std.int(nora2[i * 3 + 1] * inv * h / sc);
				// 	posa2[i * 3 + 2] -= Std.int(nora2[i * 3 + 2] * inv * h / sc);
				// }
			}

			o.writeString("o " + p.name + "\n");
			for (i in 0...pi) {
				o.writeString("v ");
				o.writeString(posa2[i * 3] * sc + "");
				o.writeString(" ");
				o.writeString(posa2[i * 3 + 2] * sc + "");
				o.writeString(" ");
				o.writeString(-posa2[i * 3 + 1] * sc + "");
				o.writeString("\n");
			}
			for (i in 0...ni) {
				o.writeString("vn ");
				o.writeString(nora2[i * 3] * inv + "");
				o.writeString(" ");
				o.writeString(nora2[i * 3 + 2] * inv + "");
				o.writeString(" ");
				o.writeString(-nora2[i * 3 + 1] * inv + "");
				o.writeString("\n");
			}
			for (i in 0...ti) {
				o.writeString("vt ");
				o.writeString(texa2[i * 2] * inv + "");
				o.writeString(" ");
				o.writeString(1.0 - texa2[i * 2 + 1] * inv + "");
				o.writeString("\n");
			}

			var inda = mesh.index_arrays[0].values;
			for (i in 0...Std.int(inda.length / 3)) {
				var pi1 = posmap.get(inda[i * 3    ]) + 1 + poff;
				var pi2 = posmap.get(inda[i * 3 + 1]) + 1 + poff;
				var pi3 = posmap.get(inda[i * 3 + 2]) + 1 + poff;
				var ni1 = normap.get(inda[i * 3    ]) + 1 + noff;
				var ni2 = normap.get(inda[i * 3 + 1]) + 1 + noff;
				var ni3 = normap.get(inda[i * 3 + 2]) + 1 + noff;
				var ti1 = texmap.get(inda[i * 3    ]) + 1 + toff;
				var ti2 = texmap.get(inda[i * 3 + 1]) + 1 + toff;
				var ti3 = texmap.get(inda[i * 3 + 2]) + 1 + toff;
				o.writeString("f ");
				o.writeString(pi1 + "");
				o.writeString("/");
				o.writeString(ti1 + "");
				o.writeString("/");
				o.writeString(ni1 + "");
				o.writeString(" ");
				o.writeString(pi2 + "");
				o.writeString("/");
				o.writeString(ti2 + "");
				o.writeString("/");
				o.writeString(ni2 + "");
				o.writeString(" ");
				o.writeString(pi3 + "");
				o.writeString("/");
				o.writeString(ti3 + "");
				o.writeString("/");
				o.writeString(ni3 + "");
				o.writeString("\n");
			}
			poff += pi;
			noff += ni;
			toff += ti;
		}

		if (!path.endsWith(".obj")) path += ".obj";

		Krom.fileSaveBytes(path, o.getBytes().getData(), o.getBytes().length);
	}
}
