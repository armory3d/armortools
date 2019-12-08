package arm.io;

import haxe.io.Bytes;
using StringTools;

class ExportObj {

	public static function run(path: String) {
		var s = "";
		var off = 0;
		for (p in Project.paintObjects) {
			var mesh = p.data.raw;
			var inv = 1 / 32767;
			var sc = p.data.scalePos * inv;
			var posa = mesh.vertex_arrays[0].values;
			var nora = mesh.vertex_arrays[1].values;
			var texa = mesh.vertex_arrays[2].values;
			var len = Std.int(posa.length / 4);
			s += "o " + p.name + "\n";
			for (i in 0...len) {
				s += "v " + posa[i * 4    ] * sc  + " " +
							posa[i * 4 + 2] * sc  + " " +
						  (-posa[i * 4 + 1] * sc) + "\n";
			}
			for (i in 0...len) {
				s += "vn " + nora[i * 2    ] * inv  + " " +
							 posa[i * 4 + 3] * inv  + " " +
						   (-nora[i * 2 + 1] * inv) + "\n";
			}
			for (i in 0...len) {
				s += "vt " +        texa[i * 2    ] * inv  + " " +
							 (1.0 - texa[i * 2 + 1] * inv) + "\n";
			}
			var inda = mesh.index_arrays[0].values;
			for (i in 0...Std.int(inda.length / 3)) {
				var i1 = inda[i * 3    ] + 1 + off;
				var i2 = inda[i * 3 + 1] + 1 + off;
				var i3 = inda[i * 3 + 2] + 1 + off;
				s += "f " + i1 + "/" + i1 + "/" + i1 + " " +
							i2 + "/" + i2 + "/" + i2 + " " +
							i3 + "/" + i3 + "/" + i3 + "\n";
			}
			off += inda.length;
		}
		if (!path.endsWith(".obj")) path += ".obj";
		Krom.fileSaveBytes(path, Bytes.ofString(s).getData());
	}
}
