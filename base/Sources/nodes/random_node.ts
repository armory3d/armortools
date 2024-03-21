
type random_node_t = {
	base?: logic_node_t;
};

let random_node_a: i32;
let random_node_b: i32;
let random_node_c: i32;
let random_node_d: i32 = -1;

function random_node_create(arg: any): random_node_t {
	let n: random_node_t = {};
	n.base = logic_node_create();
	n.base.get = random_node_get;
	return n;
}

function random_node_get(self: random_node_t, from: i32, done: (a: any)=>void) {
	logic_node_input_get(self.base.inputs[0], function (min: f32) {
		logic_node_input_get(self.base.inputs[1], function (max: f32) {
			done(min + random_node_get_float() * (max - min));
		});
	});
}

function random_node_get_int(): i32 {
	if (random_node_d == -1) {
		random_node_set_seed(352124);
	}

	// Courtesy of https://github.com/Kode/Kha/blob/main/Sources/kha/math/Random.hx
	let t: i32 = (random_node_a + random_node_b | 0) + random_node_d | 0;
	random_node_d = random_node_d + 1 | 0;
	random_node_a = random_node_b ^ random_node_b >>> 9;
	random_node_b = random_node_c + (random_node_c << 3) | 0;
	random_node_c = random_node_c << 21 | random_node_c >>> 11;
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

let random_node_def: zui_node_t = {
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
			default_value: 0.0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Max"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 1.0
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Value"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.5
		}
	],
	buttons: []
};
