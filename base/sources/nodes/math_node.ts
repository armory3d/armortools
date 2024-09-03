
type math_node_t = {
	base?: logic_node_t;
	operation?: string;
	use_clamp?: bool;
};

function math_node_create(args: f32_array_t): math_node_t {
	let n: math_node_t = {};
	n.base = logic_node_create();
	n.base.get = math_node_get;
	return n;
}

function math_node_get(self: math_node_t, from: i32): logic_node_value_t {
	let v1: f32 = logic_node_input_get(self.base.inputs[0])._f32;
	let v2: f32 = logic_node_input_get(self.base.inputs[1])._f32;
	let f: f32 = 0.0;
	let op: string = self.operation;
	if (op == "Add") {
		f = v1 + v2;
	}
	else if (op == "Multiply") {
		f = v1 * v2;
	}
	else if (op == "Sine") {
		f = math_sin(v1);
	}
	else if (op == "Cosine") {
		f = math_cos(v1);
	}
	else if (op == "Max") {
		f = math_max(v1, v2);
	}
	else if (op == "Min") {
		f = math_min(v1, v2);
	}
	else if (op == "Absolute") {
		f = math_abs(v1);
	}
	else if (op == "Subtract") {
		f = v1 - v2;
	}
	else if (op == "Divide") {
		f = v1 / (v2 == 0.0 ? 0.000001 : v2);
	}
	else if (op == "Tangent") {
		f = math_tan(v1);
	}
	else if (op == "Arcsine") {
		f = math_asin(v1);
	}
	else if (op == "Arccosine") {
		f = math_acos(v1);
	}
	else if (op == "Arctangent") {
		f = math_atan(v1);
	}
	else if (op == "Arctan2") {
		f = math_atan2(v2, v1);
	}
	else if (op == "Power") {
		f = math_pow(v1, v2);
	}
	else if (op == "Logarithm") {
		f = math_log(v1);
	}
	else if (op == "Round") {
		f = math_round(v1);
	}
	else if (op == "Floor") {
		f = math_floor(v1);
	}
	else if (op == "Ceil") {
		f = math_ceil(v1);
	}
	else if (op == "Truncate") {
		f = math_floor(v1);
	}
	else if (op == "Fraction") {
		f = v1 - math_floor(v1);
	}
	else if (op == "Less Than") {
		f = v1 < v2 ? 1.0 : 0.0;
	}
	else if (op == "Greater Than") {
		f = v1 > v2 ? 1.0 : 0.0;
	}
	else if (op == "Modulo") {
		f = math_fmod(v1, v2);
	}
	else if (op == "Snap") {
		f = math_floor(v1 / v2) * v2;
	}
	else if (op == "Square Root") {
		f = math_sqrt(v1);
	}
	else if (op == "Inverse Square Root") {
		f = 1.0 / math_sqrt(v1);
	}
	else if (op == "Exponent") {
		f = math_exp(v1);
	}
	else if (op == "Sign") {
		f = v1 > 0 ? 1.0 : (v1 < 0 ? -1.0 : 0);
	}
	else if (op == "Ping-Pong") {
		f = (v2 != 0.0) ? v2 - math_abs(math_fmod(math_abs(v1), (2 * v2)) - v2) : 0.0;
	}
	else if (op == "Hyperbolic Sine") {
		f = (math_exp(v1) - math_exp(-v1)) / 2.0;
	}
	else if (op == "Hyperbolic Cosine") {
		f = (math_exp(v1) + math_exp(-v1)) / 2.0;
	}
	else if (op == "Hyperbolic Tangent") {
		f = 1.0 - (2.0 / (math_exp(2 * v1) + 1));
	}
	else if (op == "To Radians") {
		f = v1 / 180.0 * math_pi();
	}
	else if (op == "To Degrees") {
		f = v1 / math_pi() * 180.0;
	}

	if (self.use_clamp) {
		f = f < 0.0 ? 0.0 : (f > 1.0 ? 1.0 : f);
	}

	let v: logic_node_value_t = {
		_f32: f
	};
	return v;
}

let math_node_def: ui_node_t = {
	id: 0,
	name: _tr("Math"),
	type: "math_node",
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
			default_value: f32_array_create_x(0.5),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Value"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(0.5),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Value"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [
		{
			name: _tr("operation"),
			type: "ENUM",
			output: 0,
			default_value: f32_array_create_x(0),
			data: u8_array_create_from_string("Add\nSubtract\nMultiply\nDivide\nPower\nLogarithm\nSquare Root\nInverse Square Root\nAbsolute\nExponent\nMinimum\nMaximum\nLess Than\nGreater Than\nSign\nRound\nFloor\nCeil\nTruncate\nFraction\nModulo\nSnap\nPing-Pong\nSine\nCosine\nTangent\nArcsine\nArccosine\nArctangent\nArctan2\nHyperbolic Sine\nHyperbolic Cosine\nHyperbolic Tangent\nTo Radians\nTo Degrees"),
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		},
		{
			name: _tr("use_clamp"),
			type: "BOOL",
			output: 0,
			default_value: f32_array_create_x(0),
			data: null,
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		}
	],
	width: 0
};
