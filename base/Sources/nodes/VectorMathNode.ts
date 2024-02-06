
class VectorMathNode extends LogicNode {

	operation: string;
	v = vec4_create();

	constructor() {
		super();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		this.inputs[0].get((v1: vec4_t) => {
			this.inputs[1].get((v2: vec4_t) => {
				vec4_set_from(this.v, v1);
				let f = 0.0;

				switch (this.operation) {
					case "Add":
						vec4_add(this.v, v2);
						break;
					case "Subtract":
						vec4_sub(this.v, v2);
						break;
					case "Average":
						vec4_add(this.v, v2);
						this.v.x *= 0.5;
						this.v.y *= 0.5;
						this.v.z *= 0.5;
						break;
					case "Dot Product":
						f = vec4_dot(this.v, v2);
						vec4_set(this.v, f, f, f);
						break;
					case "Cross Product":
						vec4_cross(this.v, v2);
						break;
					case "Normalize":
						vec4_normalize(this.v, );
						break;
					case "Multiply":
						this.v.x *= v2.x;
						this.v.y *= v2.y;
						this.v.z *= v2.z;
						break;
					case "Divide":
						this.v.x /= v2.x == 0.0 ? 0.000001 : v2.x;
						this.v.y /= v2.y == 0.0 ? 0.000001 : v2.y;
						this.v.z /= v2.z == 0.0 ? 0.000001 : v2.z;
						break;
					case "Length":
						f = vec4_len(this.v);
						vec4_set(this.v, f, f, f);
						break;
					case "Distance":
						f = vec4_dist_to(this.v, v2);
						vec4_set(this.v, f, f, f);
						break;
					case "Project":
						vec4_set_from(this.v, v2);
						vec4_mult(this.v, vec4_dot(v1, v2) / vec4_dot(v2, v2));
						break;
					case "Reflect":
						let tmp = vec4_create();
						vec4_set_from(tmp, v2);
						vec4_normalize(tmp);
						vec4_reflect(this.v, tmp);
						break;
					case "Scale":
						this.v.x *= v2.x;
						this.v.y *= v2.x;
						this.v.z *= v2.x;
						break;
					case "Absolute":
						this.v.x = Math.abs(this.v.x);
						this.v.y = Math.abs(this.v.y);
						this.v.z = Math.abs(this.v.z);
						break;
					case "Minimum":
						this.v.x = Math.min(v1.x, v2.x);
						this.v.y = Math.min(v1.y, v2.y);
						this.v.z = Math.min(v1.z, v2.z);
						break;
					case "Maximum":
						this.v.x = Math.max(v1.x, v2.x);
						this.v.y = Math.max(v1.y, v2.y);
						this.v.z = Math.max(v1.z, v2.z);
						break;
					case "Floor":
						this.v.x = Math.floor(v1.x);
						this.v.y = Math.floor(v1.y);
						this.v.z = Math.floor(v1.z);
						break;
					case "Ceil":
						this.v.x = Math.ceil(v1.x);
						this.v.y = Math.ceil(v1.y);
						this.v.z = Math.ceil(v1.z);
						break;
					case "Fraction":
						this.v.x = v1.x - Math.floor(v1.x);
						this.v.y = v1.y - Math.floor(v1.y);
						this.v.z = v1.z - Math.floor(v1.z);
						break;
					case "Modulo":
						this.v.x = v1.x % v2.x;
						this.v.y = v1.y % v2.y;
						this.v.z = v1.z % v2.z;
						break;
					case "Snap":
						this.v.x = Math.floor(v1.x / v2.x) * v2.x;
						this.v.y = Math.floor(v1.y / v2.y) * v2.y;
						this.v.z = Math.floor(v1.z / v2.z) * v2.z;
						break;
					case "Sine":
						this.v.x = Math.sin(v1.x);
						this.v.y = Math.sin(v1.y);
						this.v.z = Math.sin(v1.z);
						break;
					case "Cosine":
						this.v.x = Math.cos(v1.x);
						this.v.y = Math.cos(v1.y);
						this.v.z = Math.cos(v1.z);
						break;
					case "Tangent":
						this.v.x = Math.tan(v1.x);
						this.v.y = Math.tan(v1.y);
						this.v.z = Math.tan(v1.z);
						break;
				}

				if (from == 0) done(this.v);
				else done(f);
			});
		});
	}

	static def: zui_node_t = {
		id: 0,
		name: _tr("Vector Math"),
		type: "VectorMathNode",
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
				default_value: new Float32Array([0.0, 0.0, 0.0])
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Vector"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: new Float32Array([0.0, 0.0, 0.0])
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Vector"),
				type: "VECTOR",
				color: 0xff6363c7,
				default_value: new Float32Array([0.0, 0.0, 0.0])
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
}
