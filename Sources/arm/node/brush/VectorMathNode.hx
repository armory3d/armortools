package arm.node.brush;

import iron.math.Vec4;

@:keep
class VectorMathNode extends LogicNode {

	public var operation: String;
	var v = new Vec4();

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function get(from: Int): Dynamic {
		var v1: Vec4 = inputs[0].get();
		var v2: Vec4 = inputs[1].get();
		v.setFrom(v1);
		var f = 0.0;
		switch (operation) {
			case "Add":
				v.add(v2);
			case "Subtract":
				v.sub(v2);
			case "Average":
				v.add(v2);
				v.x *= 0.5;
				v.y *= 0.5;
				v.z *= 0.5;
			case "Dot Product":
				f = v.dot(v2);
				v.set(f, f, f);
			case "Cross Product":
				v.cross(v2);
			case "Normalize":
				v.normalize();
			case "Multiply":
				v.x *= v2.x;
				v.y *= v2.y;
				v.z *= v2.z;
			case "Divide":
				v.x /= v2.x == 0.0 ? 0.000001 : v2.x;
				v.y /= v2.y == 0.0 ? 0.000001 : v2.y;
				v.z /= v2.z == 0.0 ? 0.000001 : v2.z;
			case "Length":
				f = v.length();
				v.set(f, f, f);
			case "Distance":
				f = v.distanceTo(v2);
				v.set(f, f, f);
			case "Project":
				v.setFrom(v2);
				v.mult(v1.dot(v2) / v2.dot(v2));
			case "Reflect":
				var tmp = new Vec4();
				tmp.setFrom(v2);
				tmp.normalize();
				v.reflect(tmp);
			case "Scale":
				v.x *= v2.x;
				v.y *= v2.x;
				v.z *= v2.x;
			case "Absolute":
				v.x = Math.abs(v.x);
				v.y = Math.abs(v.y);
				v.z = Math.abs(v.z);
			case "Minimum":
				v.x = Math.min(v1.x, v2.x);
				v.y = Math.min(v1.y, v2.y);
				v.z = Math.min(v1.z, v2.z);
			case "Maximum":
				v.x = Math.max(v1.x, v2.x);
				v.y = Math.max(v1.y, v2.y);
				v.z = Math.max(v1.z, v2.z);
			case "Floor": 
				v.x = Math.floor(v1.x);
				v.y = Math.floor(v1.y);
				v.z = Math.floor(v1.z);
			case "Ceil":
				v.x = Math.ceil(v1.x);
				v.y = Math.ceil(v1.y);
				v.z = Math.ceil(v1.z);
			case "Fraction":
				v.x = v1.x - Math.floor(v1.x);
				v.y = v1.y - Math.floor(v1.y);
				v.z = v1.z - Math.floor(v1.z);
			case "Modulo":
				v.x = v1.x % v2.x;
				v.y = v1.y % v2.y;
				v.z = v1.z % v2.z;
			case "Snap":
				v.x = Math.floor(v1.x / v2.x) * v2.x;
				v.y = Math.floor(v1.y / v2.y) * v2.y;
				v.z = Math.floor(v1.z / v2.z) * v2.z;
			case "Sine":
				v.x = Math.sin(v1.x);
				v.y = Math.sin(v1.y);
				v.z = Math.sin(v1.z);
			case "Cosine":
				v.x = Math.cos(v1.x);
				v.y = Math.cos(v1.y);
				v.z = Math.cos(v1.z);
			case "Tangent":
				v.x = Math.tan(v1.x);
				v.y = Math.tan(v1.y);
				v.z = Math.tan(v1.z);
		}

		if (from == 0) return v;
		else return f;
	}
}
