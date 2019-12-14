package arm.util;

import kha.Image;

class UVUtil {

	public static var uvmap: Image = null;
	public static var uvmapCached = false;
	public static var trianglemap: Image = null;
	public static var trianglemapCached = false;

	public static function cacheUVMap() {
		if (uvmapCached) return;

		var res = Config.getTextureRes();
		if (uvmap == null) {
			uvmap = Image.createRenderTarget(res, res);
		}

		uvmapCached = true;
		var merged = Context.mergedObject != null ? Context.mergedObject.data.raw : Context.paintObject.data.raw;
		var mesh = merged;
		var texa = mesh.vertex_arrays[2].values;
		var inda = mesh.index_arrays[0].values;
		uvmap.g2.begin(true, 0x00000000);
		uvmap.g2.color = 0xffcccccc;
		var strength = res > 2048 ? 2.0 : 1.0;
		var f = (1 / 32767) * uvmap.width;
		for (i in 0...Std.int(inda.length / 3)) {
			var x1 = (texa[inda[i * 3    ] * 2    ]) * f;
			var x2 = (texa[inda[i * 3 + 1] * 2    ]) * f;
			var x3 = (texa[inda[i * 3 + 2] * 2    ]) * f;
			var y1 = (texa[inda[i * 3    ] * 2 + 1]) * f;
			var y2 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
			var y3 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
			uvmap.g2.drawLine(x1, y1, x2, y2, strength);
			uvmap.g2.drawLine(x2, y2, x3, y3, strength);
			uvmap.g2.drawLine(x3, y3, x1, y1, strength);
		}
		uvmap.g2.end();
	}

	public static function cacheTriangleMap() {
		if (trianglemapCached) return;

		var res = Config.getTextureRes();
		if (trianglemap == null) {
			trianglemap = Image.createRenderTarget(res, res);
		}

		trianglemapCached = true;
		var merged = Context.mergedObject != null ? Context.mergedObject.data.raw : Context.paintObject.data.raw;
		var mesh = merged;
		var texa = mesh.vertex_arrays[2].values;
		var inda = mesh.index_arrays[0].values;
		trianglemap.g2.begin(true, 0xff000000);
		var f = (1 / 32767) * trianglemap.width;
		var color = 0xff000000;
		for (i in 0...Std.int(inda.length / 3)) {
			if (color == 0xffffffff) color = 0xff000000;
			color++;
			trianglemap.g2.color = color;
			var x1 = (texa[inda[i * 3    ] * 2    ]) * f;
			var x2 = (texa[inda[i * 3 + 1] * 2    ]) * f;
			var x3 = (texa[inda[i * 3 + 2] * 2    ]) * f;
			var y1 = (texa[inda[i * 3    ] * 2 + 1]) * f;
			var y2 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
			var y3 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
			trianglemap.g2.fillTriangle(x1, y1, x2, y2, x3, y3);
		}
		trianglemap.g2.end();
	}
}
