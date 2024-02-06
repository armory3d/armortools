
class MathNode extends LogicNode {

	operation: string;
	use_clamp: bool;

	constructor() {
		super();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		this.inputs[0].get((v1: f32) => {
			this.inputs[1].get((v2: f32) => {
				let f = 0.0;
				switch (this.operation) {
					case "Add":
						f = v1 + v2;
						break;
					case "Multiply":
						f = v1 * v2;
						break;
					case "Sine":
						f = Math.sin(v1);
						break;
					case "Cosine":
						f = Math.cos(v1);
						break;
					case "Max":
						f = Math.max(v1, v2);
						break;
					case "Min":
						f = Math.min(v1, v2);
						break;
					case "Absolute":
						f = Math.abs(v1);
						break;
					case "Subtract":
						f = v1 - v2;
						break;
					case "Divide":
						f = v1 / (v2 == 0.0 ? 0.000001 : v2);
						break;
					case "Tangent":
						f = Math.tan(v1);
						break;
					case "Arcsine":
						f = Math.asin(v1);
						break;
					case "Arccosine":
						f = Math.acos(v1);
						break;
					case "Arctangent":
						f = Math.atan(v1);
						break;
					case "Arctan2":
					    f = Math.atan2(v2, v1);
						break;
					case "Power":
						f = Math.pow(v1, v2);
						break;
					case "Logarithm":
						f = Math.log(v1);
						break;
					case "Round":
						f = Math.round(v1);
						break;
					case "Floor":
					    f = Math.floor(v1);
						break;
					case "Ceil":
					    f = Math.ceil(v1);
						break;
					case "Truncate":
						f = Math.floor(v1);
						break;
					case "Fraction":
					    f = v1 - Math.floor(v1);
						break;
					case "Less Than":
						f = v1 < v2 ? 1.0 : 0.0;
						break;
					case "Greater Than":
						f = v1 > v2 ? 1.0 : 0.0;
						break;
					case "Modulo":
						f = v1 % v2;
						break;
					case "Snap":
						f = Math.floor(v1 / v2) * v2;
						break;
					case "Square Root":
					    f = Math.sqrt(v1);
						break;
					case "Inverse Square Root":
						f = 1.0 / Math.sqrt(v1);
						break;
					case "Exponent":
						f = Math.exp(v1);
						break;
					case "Sign":
						f = v1 > 0 ? 1.0 : (v1 < 0 ? -1.0 : 0);
						break;
					case "Ping-Pong":
					    f = (v2 != 0.0) ? v2 - Math.abs((Math.abs(v1) % (2 * v2)) - v2) : 0.0;
						break;
					case "Hyperbolic Sine":
						f = (Math.exp(v1) - Math.exp(-v1)) / 2.0;
						break;
					case "Hyperbolic Cosine":
						f = (Math.exp(v1) + Math.exp(-v1)) / 2.0;
						break;
					case "Hyperbolic Tangent":
						f = 1.0 - (2.0 / (Math.exp(2 * v1) + 1));
						break;
					case "To Radians":
						f = v1 / 180.0 * Math.PI;
						break;
					case "To Degrees":
						f = v1 / Math.PI * 180.0;
						break;
				}

				if (this.use_clamp) f = f < 0.0 ? 0.0 : (f > 1.0 ? 1.0 : f);

				done(f);
			});
		});
	}

	static def: zui_node_t = {
		id: 0,
		name: _tr("Math"),
		type: "MathNode",
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.5
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.5
			}
		],
		outputs: [
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
				data: ["Add", "Subtract", "Multiply", "Divide", "Power", "Logarithm", "Square Root", "Inverse Square Root", "Absolute", "Exponent", "Minimum", "Maximum", "Less Than", "Greater Than", "Sign", "Round", "Floor", "Ceil", "Truncate", "Fraction", "Modulo", "Snap", "Ping-Pong", "Sine", "Cosine", "Tangent", "Arcsine", "Arccosine", "Arctangent", "Arctan2", "Hyperbolic Sine", "Hyperbolic Cosine", "Hyperbolic Tangent", "To Radians", "To Degrees"],
				default_value: 0,
				output: 0
			},
			{
				name: _tr("use_clamp"),
				type: "BOOL",
				default_value: false,
				output: 0
			}
		]
	};
}
