package arm.io;

import haxe.io.Bytes;
import haxe.io.BytesOutput;

class ExportObj {

	public static function run(path: String, applyDisplacement = false) {

		var height = applyDisplacement ? Project.layers[0].texpaint_pack.getPixels() : null;
		var res = Project.layers[0].texpaint_pack.width;

		var o = new BytesOutput();
		o.bigEndian = false;

		var off = 0;
		for (p in Project.paintObjects) {
			var mesh = p.data.raw;
			var inv = 1 / 32767;
			var sc = p.data.scalePos * inv;
			var posa = mesh.vertex_arrays[0].values;
			var nora = mesh.vertex_arrays[1].values;
			var texa = mesh.vertex_arrays[2].values;
			var len = Std.int(posa.length / 4);

			// if (applyDisplacement) {
			// }

			o.writeString("o " + p.name + "\n");
			for (i in 0...len) {
				o.writeString("v ");
				o.writeString(posa[i * 4] * sc + "");
				o.writeString(" ");
				o.writeString(posa[i * 4 + 2] * sc + "");
				o.writeString(" ");
				o.writeString(-posa[i * 4 + 1] * sc + "");
				o.writeString("\n");
			}
			for (i in 0...len) {
				o.writeString("vn ");
				o.writeString(nora[i * 2] * inv + "");
				o.writeString(" ");
				o.writeString(posa[i * 4 + 3] * inv + "");
				o.writeString(" ");
				o.writeString(-nora[i * 2 + 1] * inv + "");
				o.writeString("\n");
			}
			for (i in 0...len) {
				o.writeString("vt ");
				o.writeString(texa[i * 2] * inv + "");
				o.writeString(" ");
				o.writeString(1.0 - texa[i * 2 + 1] * inv + "");
				o.writeString("\n");
			}

			var inda = mesh.index_arrays[0].values;
			for (i in 0...Std.int(inda.length / 3)) {
				var i1 = inda[i * 3    ] + 1 + off;
				var i2 = inda[i * 3 + 1] + 1 + off;
				var i3 = inda[i * 3 + 2] + 1 + off;
				o.writeString("f ");
				o.writeString(i1 + "");
				o.writeString("/");
				o.writeString(i1 + "");
				o.writeString("/");
				o.writeString(i1 + "");
				o.writeString(" ");
				o.writeString(i2 + "");
				o.writeString("/");
				o.writeString(i2 + "");
				o.writeString("/");
				o.writeString(i2 + "");
				o.writeString(" ");
				o.writeString(i3 + "");
				o.writeString("/");
				o.writeString(i3 + "");
				o.writeString("/");
				o.writeString(i3 + "");
				o.writeString("\n");
			}
			off += inda.length;
		}

		if (!path.endsWith(".obj")) path += ".obj";

		Krom.fileSaveBytes(path, o.getBytes().getData());
	}
}
