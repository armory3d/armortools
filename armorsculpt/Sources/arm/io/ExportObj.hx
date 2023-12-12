package arm.io;

import js.lib.Int16Array;
import iron.MeshObject;

class ExportObj {

	static function writeString(out: Array<Int>, str: String) {
		for (i in 0...str.length) {
			out.push(str.charCodeAt(i));
		}
	}

	public static function run(path: String, paintObjects: Array<MeshObject>, applyDisplacement = false) {
		var o: Array<Int> = [];
		writeString(o, "# armorsculpt.org\n");

		var texpaint = Project.layers[0].texpaint;
		var pixels = texpaint.getPixels();
		var pixelsView = new js.lib.DataView(pixels);
		var mesh = paintObjects[0].data.raw;
		var inda = mesh.index_arrays[0].values;

		var posa = new Int16Array(inda.length * 4);
		for (i in 0...inda.length) {
			var index = inda[i];
			posa[index * 4    ] = Std.int(pixelsView.getFloat32(i * 16    , true) * 32767);
			posa[index * 4 + 1] = Std.int(pixelsView.getFloat32(i * 16 + 4, true) * 32767);
			posa[index * 4 + 2] = Std.int(pixelsView.getFloat32(i * 16 + 8, true) * 32767);
		}

		var poff = 0;
		// for (p in paintObjects) {
			var p = paintObjects[0];
			// var mesh = p.data.raw;
			var inv = 1 / 32767;
			var sc = p.data.scalePos * inv;
			// var posa = mesh.vertex_arrays[0].values;
			var len = Std.int(posa.length / 4);
			// var len = Std.int(inda.length);

			// Merge shared vertices and remap indices
			var posa2 = new Int16Array(len * 3);
			var posmap = new Map<Int, Int>();

			var pi = 0;
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
			}

			writeString(o, "o " + p.name + "\n");
			for (i in 0...pi) {
				writeString(o, "v ");
				var vx = posa2[i * 3] * sc + "";
				writeString(o, vx.substr(0, vx.indexOf(".") + 7));
				writeString(o, " ");
				var vy = posa2[i * 3 + 2] * sc + "";
				writeString(o, vy.substr(0, vy.indexOf(".") + 7));
				writeString(o, " ");
				var vz = -posa2[i * 3 + 1] * sc + "";
				writeString(o, vz.substr(0, vz.indexOf(".") + 7));
				writeString(o, "\n");
			}

			// var inda = mesh.index_arrays[0].values;
			for (i in 0...Std.int(inda.length / 3)) {
				var pi1 = posmap.get(inda[i * 3    ]) + 1 + poff;
				var pi2 = posmap.get(inda[i * 3 + 1]) + 1 + poff;
				var pi3 = posmap.get(inda[i * 3 + 2]) + 1 + poff;
				writeString(o, "f ");
				writeString(o, pi1 + "");
				writeString(o, " ");
				writeString(o, pi2 + "");
				writeString(o, " ");
				writeString(o, pi3 + "");
				writeString(o, "\n");
			}
			poff += pi;
		// }

		if (!path.endsWith(".obj")) path += ".obj";

		var b = js.lib.Uint8Array.from(o).buffer;
		Krom.fileSaveBytes(path, b, b.byteLength);
	}
}
