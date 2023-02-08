package arm.format;

import iron.math.Vec4;

class MeshParser {
	public static function pnpoly(v0x: Float, v0y: Float, v1x: Float, v1y: Float, v2x: Float, v2y: Float, px: Float, py: Float): Bool {
		// https://wrf.ecse.rpi.edu//Research/Short_Notes/pnpoly.html
		var c = false;
		if (((v0y > py) != (v2y > py)) && (px < (v2x - v0x) * (py - v0y) / (v2y - v0y) + v0x)) c = !c;
		if (((v1y > py) != (v0y > py)) && (px < (v0x - v1x) * (py - v1y) / (v0y - v1y) + v1x)) c = !c;
		if (((v2y > py) != (v1y > py)) && (px < (v1x - v2x) * (py - v2y) / (v1y - v2y) + v2x)) c = !c;
		return c;
	}

	public static function calcNormal(p0: Vec4, p1: Vec4, p2: Vec4): Vec4 {
		var cb = new iron.math.Vec4().subvecs(p2, p1);
		var ab = new iron.math.Vec4().subvecs(p0, p1);
		cb.cross(ab);
		cb.normalize();
		return cb;
	}
}
