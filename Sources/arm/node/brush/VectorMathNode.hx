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
		}

		if (from == 0) return v;
		else return f;
	}
}
