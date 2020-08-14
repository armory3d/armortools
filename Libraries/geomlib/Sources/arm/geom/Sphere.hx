package arm.geom;

class Sphere {

	public var posa: kha.arrays.Int16Array = null;
	public var nora: kha.arrays.Int16Array = null;
	public var texa: kha.arrays.Int16Array = null;
	public var inda: kha.arrays.Uint32Array = null;
	public var scalePos = 1.0;
	public var scaleTex = 1.0;
	public var name = "";
	public var hasNext = false;

	public function new(radius = 1.0, widthSegments = 32, heightSegments = 16) {
		// Pack positions to (-1, 1) range
		scalePos = radius;
		var inv = (1 / scalePos) * 32767;
		var pi2 = Math.PI * 2;

		var widthVerts = widthSegments + 1;
		var heightVerts = heightSegments + 1;
		posa = new kha.arrays.Int16Array(widthVerts * heightVerts * 4);
		nora = new kha.arrays.Int16Array(widthVerts * heightVerts * 2);
		texa = new kha.arrays.Int16Array(widthVerts * heightVerts * 2);
		inda = new kha.arrays.Uint32Array(widthSegments * heightSegments * 6 - widthSegments * 6);

		var nor = new iron.math.Vec4();
		var pos = 0;
		for (y in 0...heightVerts) {
			var v = y / heightSegments;
			var vFlip = 1.0 - v;
			var uOff = y == 0 ? 0.5 / widthSegments : y == heightSegments ? -0.5 / widthSegments : 0.0;
			for (x in 0...widthVerts) {
				var u = x / widthSegments;
				var uPI2 = u * pi2;
				var vPI  = v * Math.PI;
				var vPIsin = Math.sin(vPI);
				var vx = -radius * Math.cos(uPI2) * vPIsin;
				var vy =  radius * Math.cos(vPI);
				var vz =  radius * Math.sin(uPI2) * vPIsin;
				var i4 = pos * 4;
				var i2 = pos * 2;
				posa[i4    ] = Std.int(vx * inv);
				posa[i4 + 1] = Std.int(vy * inv);
				posa[i4 + 2] = Std.int(vz * inv);
				nor.set(vx, vy, vz).normalize();
				posa[i4 + 3] = Std.int(nor.z * 32767);
				nora[i2    ] = Std.int(nor.x * 32767);
				nora[i2 + 1] = Std.int(nor.y * 32767);
				texa[i2    ] = Std.int((u + uOff) * 32767);
				texa[i2 + 1] = Std.int(vFlip      * 32767);
				pos++;
			}
		}

		pos = 0;
		var heightSegments1 = heightSegments - 1;
		for (y in 0...heightSegments) {
			for (x in 0...widthSegments) {
				var x1 = x + 1;
				var y1 = y + 1;
				var a = y  * widthVerts + x1;
				var b = y  * widthVerts + x;
				var c = y1 * widthVerts + x;
				var d = y1 * widthVerts + x1;
				if (y > 0) {
					inda[pos++] = a;
					inda[pos++] = b;
					inda[pos++] = d;
				}
				if (y < heightSegments1) {
					inda[pos++] = b;
					inda[pos++] = c;
					inda[pos++] = d;
				}
			}
		}
	}
}
