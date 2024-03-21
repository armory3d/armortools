
type vector_math_node_t = {
	base?: logic_node_t;
	operation?: string;
	v?: vec4_t;
};

function vector_math_node_create(arg: any): vector_math_node_t {
	let n: vector_math_node_t = {};
	n.base = logic_node_create();
	n.base.get = vector_math_node_get;
	n.v = vec4_create();
	return n;
}

function vector_math_node_get(self: vector_math_node_t, from: i32, done: (a: any)=>void) {
	logic_node_input_get(self.base.inputs[0], function (v1: vec4_t) {
		logic_node_input_get(self.base.inputs[1], function (v2: vec4_t) {
			vec4_set_from(self.v, v1);
			let f: f32 = 0.0;
			let op: string = self.operation;
			if (op == "Add") {
				vec4_add(self.v, v2);
			}
			else if (op == "Subtract") {
				vec4_sub(self.v, v2);
			}
			else if (op == "Average") {
				vec4_add(self.v, v2);
				self.v.x *= 0.5;
				self.v.y *= 0.5;
				self.v.z *= 0.5;
			}
			else if (op == "Dot Product") {
				f = vec4_dot(self.v, v2);
				vec4_set(self.v, f, f, f);
			}
			else if (op == "Cross Product") {
				vec4_cross(self.v, v2);
			}
			else if (op == "Normalize") {
				vec4_normalize(self.v, );
			}
			else if (op == "Multiply") {
				self.v.x *= v2.x;
				self.v.y *= v2.y;
				self.v.z *= v2.z;
			}
			else if (op == "Divide") {
				self.v.x /= v2.x == 0.0 ? 0.000001 : v2.x;
				self.v.y /= v2.y == 0.0 ? 0.000001 : v2.y;
				self.v.z /= v2.z == 0.0 ? 0.000001 : v2.z;
			}
			else if (op == "Length") {
				f = vec4_len(self.v);
				vec4_set(self.v, f, f, f);
			}
			else if (op == "Distance") {
				f = vec4_dist_to(self.v, v2);
				vec4_set(self.v, f, f, f);
			}
			else if (op == "Project") {
				vec4_set_from(self.v, v2);
				vec4_mult(self.v, vec4_dot(v1, v2) / vec4_dot(v2, v2));
			}
			else if (op == "Reflect") {
				let tmp: vec4_t = vec4_create();
				vec4_set_from(tmp, v2);
				vec4_normalize(tmp);
				vec4_reflect(self.v, tmp);
			}
			else if (op == "Scale") {
				self.v.x *= v2.x;
				self.v.y *= v2.x;
				self.v.z *= v2.x;
			}
			else if (op == "Absolute") {
				self.v.x = math_abs(self.v.x);
				self.v.y = math_abs(self.v.y);
				self.v.z = math_abs(self.v.z);
			}
			else if (op == "Minimum") {
				self.v.x = math_min(v1.x, v2.x);
				self.v.y = math_min(v1.y, v2.y);
				self.v.z = math_min(v1.z, v2.z);
			}
			else if (op == "Maximum") {
				self.v.x = math_max(v1.x, v2.x);
				self.v.y = math_max(v1.y, v2.y);
				self.v.z = math_max(v1.z, v2.z);
			}
			else if (op == "Floor") {
				self.v.x = math_floor(v1.x);
				self.v.y = math_floor(v1.y);
				self.v.z = math_floor(v1.z);
			}
			else if (op == "Ceil") {
				self.v.x = math_ceil(v1.x);
				self.v.y = math_ceil(v1.y);
				self.v.z = math_ceil(v1.z);
			}
			else if (op == "Fraction") {
				self.v.x = v1.x - math_floor(v1.x);
				self.v.y = v1.y - math_floor(v1.y);
				self.v.z = v1.z - math_floor(v1.z);
			}
			else if (op == "Modulo") {
				self.v.x = v1.x % v2.x;
				self.v.y = v1.y % v2.y;
				self.v.z = v1.z % v2.z;
			}
			else if (op == "Snap") {
				self.v.x = math_floor(v1.x / v2.x) * v2.x;
				self.v.y = math_floor(v1.y / v2.y) * v2.y;
				self.v.z = math_floor(v1.z / v2.z) * v2.z;
			}
			else if (op == "Sine") {
				self.v.x = math_sin(v1.x);
				self.v.y = math_sin(v1.y);
				self.v.z = math_sin(v1.z);
			}
			else if (op == "Cosine") {
				self.v.x = math_cos(v1.x);
				self.v.y = math_cos(v1.y);
				self.v.z = math_cos(v1.z);
			}
			else if (op == "Tangent") {
				self.v.x = math_tan(v1.x);
				self.v.y = math_tan(v1.y);
				self.v.z = math_tan(v1.z);
			}

			if (from == 0) {
				done(self.v);
			}
			else {
				done(f);
			}
		});
	});
}

let vector_math_node_def: zui_node_t = {
	id: 0,
	name: _tr("Vector Math"),
	type: "vector_math_node",
	x: 0,
	y: 0,
	color: 0xff4982a0,
	inputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Vector"),
			type: "VECTOR",
			color: 0xff6363c7,
			default_value: new f32_array_t([0.0, 0.0, 0.0])
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Vector"),
			type: "VECTOR",
			color: 0xff6363c7,
			default_value: new f32_array_t([0.0, 0.0, 0.0])
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Vector"),
			type: "VECTOR",
			color: 0xff6363c7,
			default_value: new f32_array_t([0.0, 0.0, 0.0])
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Value"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.0
		}
	],
	buttons: [
		{
			name: _tr("operation"),
			type: "ENUM",
			data: ["Add", "Subtract", "Multiply", "Divide", "Average", "Cross Product", "Project", "Reflect", "Dot Product", "Distance", "Length", "Scale", "Normalize", "Absolute", "Minimum", "Maximum", "Floor", "Ceil", "Fraction", "Modulo", "Snap", "Sine", "Cosine", "Tangent"],
			default_value: 0,
			output: 0
		}
	]
};
