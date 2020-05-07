package arm.format.proc;

class Plane {

	public var posa: kha.arrays.Int16Array = null;
	public var nora: kha.arrays.Int16Array = null;
	public var texa: kha.arrays.Int16Array = null;
	public var inda: kha.arrays.Uint32Array = null;
	public var scalePos = 1.0;
	public var scaleTex = 1.0;
	public var name = "";
	public var hasNext = false;

	public function new(sizeX = 1.0, sizeY = 1.0, vertsX = 2, vertsY = 2) {
		// Pack positions to (-1, 1) range
		var halfX = sizeX / 2;
		var halfY = sizeY / 2;
		scalePos = Math.max(halfX, halfY);
		var inv = (1 / scalePos) * 32767;

		posa = new kha.arrays.Int16Array(vertsX * vertsY * 4);
		nora = new kha.arrays.Int16Array(vertsX * vertsY * 2);
		texa = new kha.arrays.Int16Array(vertsX * vertsY * 2);
		inda = new kha.arrays.Uint32Array((vertsX - 1) * (vertsY - 1) * 6);
		var stepX = sizeX / (vertsX - 1);
		var stepY = sizeY / (vertsY - 1);
		for (i in 0...vertsX * vertsY) {
			var x = (i % vertsX) * stepX - halfX;
			var y = Std.int(i / vertsX) * stepY - halfY;
			posa[i * 4    ] = Std.int(x * inv);
			posa[i * 4 + 1] = Std.int(y * inv);
			posa[i * 4 + 2] = 0;
			nora[i * 2    ] = 0;
			nora[i * 2 + 1] = 0;
			posa[i * 4 + 3] = 32767;
			x = (i % vertsX) / vertsX;
			y = (Std.int(i / vertsX)) / vertsY;
			texa[i * 2    ] = Std.int(x * 32767);
			texa[i * 2 + 1] = Std.int(y * 32767);
		}
		for (i in 0...(vertsX - 1) * (vertsY - 1)) {
			var x = i % (vertsX - 1);
			var y = Std.int(i / (vertsY - 1));
			inda[i * 6    ] = y * vertsX + x;
			inda[i * 6 + 1] = y * vertsX + x + 1;
			inda[i * 6 + 2] = (y + 1) * vertsX + x;
			inda[i * 6 + 3] = y * vertsX + x + 1;
			inda[i * 6 + 4] = (y + 1) * vertsX + x + 1;
			inda[i * 6 + 5] = (y + 1) * vertsX + x;
		}
	}
}
