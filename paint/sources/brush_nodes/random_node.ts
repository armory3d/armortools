
type random_node_t = {
	base?: logic_node_t;
};

let random_node_a: i32;
let random_node_b: i32;
let random_node_c: i32;
let random_node_d: i32 = -1;

function random_node_create(raw: ui_node_t, args: f32_array_t): random_node_t {
	let n: random_node_t = {};
	n.base = logic_node_create(n);
	n.base.get = random_node_get;
	return n;
}

function random_node_get(self: random_node_t, from: i32): logic_node_value_t {
	let min: f32 = logic_node_input_get(self.base.inputs[0])._f32;
	let max: f32 = logic_node_input_get(self.base.inputs[1])._f32;
	let v: logic_node_value_t = { _f32: min + random_node_get_float() * (max - min) };
	return v;
}

function random_node_get_int(): i32 {
	if (random_node_d == -1) {
		random_node_set_seed(352124);
	}

	// Courtesy of https://github.com/Kode/Kha/blob/main/Sources/kha/math/Random.hx
	let t: i32 = (random_node_a + random_node_b | 0) + random_node_d | 0;
	random_node_d = random_node_d + 1 | 0;
	let urandom_node_b: u32 = random_node_b;
	random_node_a = random_node_b ^ urandom_node_b >> 9;
	random_node_b = random_node_c + (random_node_c << 3) | 0;
	let urandom_node_c: u32 = random_node_c;
	random_node_c = random_node_c << 21 | urandom_node_c >> 11;
	random_node_c = random_node_c + t | 0;
	return t & 0x7fffffff;
}

function random_node_set_seed(seed: i32) {
	random_node_d = seed;
	random_node_a = 0x36aef51a;
	random_node_b = 0x21d4b3eb;
	random_node_c = 0xf2517abf;
	// Immediately skip a few possibly poor results the easy way
	for (let i: i32 = 0; i < 15; ++i) {
		random_node_get_int();
	}
}

function random_node_get_seed(): i32 {
	return random_node_d;
}

function random_node_get_float(): f32 {
	return random_node_get_int() / 0x7fffffff;
}

let random_node_def: ui_node_t = {
	id: 0,
	name: _tr("Random"),
	type: "random_node",
	x: 0,
	y: 0,
	color: 0xffb34f5a,
	inputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Min"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Max"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(1.0),
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
			default_value: f32_array_create_x(0.5),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [],
	width: 0
};
