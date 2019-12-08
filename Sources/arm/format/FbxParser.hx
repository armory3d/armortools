package arm.format;

import arm.format.FbxLibrary;

@:access(arm.format.FbxLibrary)
@:access(arm.format.Geometry)
class FbxParser {

	public var posa: kha.arrays.Int16Array = null;
	public var nora: kha.arrays.Int16Array = null;
	public var texa: kha.arrays.Int16Array = null;
	public var inda: kha.arrays.Uint32Array = null;
	public var scalePos = 1.0;
	public var scaleTex = 1.0;
	public var name = "";

	// Transform
	public static var parseTransform = false;
	public var tx = 0.0;
	public var ty = 0.0;
	public var tz = 0.0;
	public var rx = 0.0;
	public var ry = 0.0;
	public var rz = 0.0;
	public var sx = 1.0;
	public var sy = 1.0;
	public var sz = 1.0;

	var geoms: Array<Geometry>;
	var current = 0;
	var binary = true;

	public function new(blob: kha.Blob) {
		var magic = "Kaydara FBX Binary\x20\x20\x00\x1a\x00";
		var s = "";
		for (i in 0...magic.length) s += String.fromCharCode(blob.readU8(i));
		binary = s == magic;

		var fbx = binary ? FbxBinaryParser.parse(blob) : Parser.parse(blob.toString());
		var lib = new FbxLibrary();
		try { lib.load(fbx); }
		catch (e: Dynamic) { trace(e); }

		geoms = lib.getAllGeometries();
		next();
	}

	public function next(): Bool {
		if (current >= geoms.length) return false;
		var geom = geoms[current];
		var lib = geom.lib;

		tx = ty = tz = 0;
		rx = ry = rz = 0;
		sx = sy = sz = 1;
		if (parseTransform) {
			var connects = lib.invConnect.get(FbxTools.getId(geom.getRoot()));
			for (c in connects) {
				var node = lib.ids.get(c);
				for (p in FbxTools.getAll(node, "Properties70.P")) {
					switch (FbxTools.toString(p.props[0])) {
					case "Lcl Translation":
						tx = FbxTools.toFloat(p.props[4]) / 100;
						ty = FbxTools.toFloat(p.props[5]) / 100;
						tz = FbxTools.toFloat(p.props[6]) / 100;
					case "Lcl Rotation":
						rx = FbxTools.toFloat(p.props[4]) * Math.PI / 180;
						ry = FbxTools.toFloat(p.props[5]) * Math.PI / 180;
						rz = FbxTools.toFloat(p.props[6]) * Math.PI / 180;
					case "Lcl Scaling":
						sx = FbxTools.toFloat(p.props[4]) / 100;
						sy = FbxTools.toFloat(p.props[5]) / 100;
						sz = FbxTools.toFloat(p.props[6]) / 100;
					default:
					}
				}
			}
		}

		var res = geom.getBuffers(binary, this);
		scalePos = res.scalePos;
		posa = res.posa;
		nora = res.nora;
		texa = res.texa;
		inda = res.inda;
		name = FbxTools.getName(geom.getRoot());
		if (name.charCodeAt(0) == 0) name = name.substring(1); // null
		if (name.charCodeAt(0) == 1) name = name.substring(1); // start of heading
		if (name == "Geometry") name = "Object -Geometry";
		name = name.substring(0, name.length - 10); // -Geometry

		current++;
		return true;
	}
}
