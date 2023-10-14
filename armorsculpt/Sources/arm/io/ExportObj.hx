package arm.io;

import haxe.io.BytesOutput;
import kha.arrays.Int16Array;
import iron.object.MeshObject;

class ExportObj {

	public static function run(path: String, paintObjects: Array<MeshObject>, applyDisplacement = false) {
		var o = new BytesOutput();
		o.bigEndian = false;
		o.writeString("# armorsculpt.org\n");

		var texpaint = Project.layers[0].texpaint;
		var pixels = texpaint.getPixels();
		var mesh = paintObjects[0].data.raw;
		var inda = mesh.index_arrays[0].values;

		var posa = new Int16Array(inda.length * 4);
		for (i in 0...inda.length) {
			var index = inda[i];
			posa[index * 4    ] = Std.int(pixels.getFloat(i * 16    ) * 32767);
			posa[index * 4 + 1] = Std.int(pixels.getFloat(i * 16 + 4) * 32767);
			posa[index * 4 + 2] = Std.int(pixels.getFloat(i * 16 + 8) * 32767);
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

			o.writeString("o " + p.name + "\n");
			for (i in 0...pi) {
				o.writeString("v ");
				var vx = posa2[i * 3] * sc + "";
				o.writeString(vx.substr(0, vx.indexOf(".") + 7));
				o.writeString(" ");
				var vy = posa2[i * 3 + 2] * sc + "";
				o.writeString(vy.substr(0, vy.indexOf(".") + 7));
				o.writeString(" ");
				var vz = -posa2[i * 3 + 1] * sc + "";
				o.writeString(vz.substr(0, vz.indexOf(".") + 7));
				o.writeString("\n");
			}

			// var inda = mesh.index_arrays[0].values;
			for (i in 0...Std.int(inda.length / 3)) {
				var pi1 = posmap.get(inda[i * 3    ]) + 1 + poff;
				var pi2 = posmap.get(inda[i * 3 + 1]) + 1 + poff;
				var pi3 = posmap.get(inda[i * 3 + 2]) + 1 + poff;
				o.writeString("f ");
				o.writeString(pi1 + "");
				o.writeString(" ");
				o.writeString(pi2 + "");
				o.writeString(" ");
				o.writeString(pi3 + "");
				o.writeString("\n");
			}
			poff += pi;
		// }

		if (!path.endsWith(".obj")) path += ".obj";

		Krom.fileSaveBytes(path, o.getBytes().getData(), o.getBytes().length);
	}
}
