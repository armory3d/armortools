
class RandomNode extends LogicNode {

	constructor() {
		super();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		this.inputs[0].get((min: f32) => {
			this.inputs[1].get((max: f32) => {
				done(min + RandomNode.getFloat() * (max - min));
			});
		});
	}

	// Courtesy of https://github.com/Kode/Kha/blob/main/Sources/kha/math/Random.hx
	static getInt = (): i32 => {
		let t = (RandomNode.a + RandomNode.b | 0) + RandomNode.d | 0;
		RandomNode.d = RandomNode.d + 1 | 0;
		RandomNode.a = RandomNode.b ^ RandomNode.b >>> 9;
		RandomNode.b = RandomNode.c + (RandomNode.c << 3) | 0;
		RandomNode.c = RandomNode.c << 21 | RandomNode.c >>> 11;
		RandomNode.c = RandomNode.c + t | 0;
		return t & 0x7fffffff;
	}

	static setSeed = (seed: i32): i32 => {
		RandomNode.d = seed;
		RandomNode.a = 0x36aef51a;
		RandomNode.b = 0x21d4b3eb;
		RandomNode.c = 0xf2517abf;
		// Immediately skip a few possibly poor results the easy way
		for (let i = 0; i < 15; ++i) {
			RandomNode.getInt();
		}
		return RandomNode.d;
	}

	static getSeed = (): i32 => {
		return RandomNode.d;
	}

	static a: i32;
	static b: i32;
	static c: i32;
	static d = RandomNode.setSeed(352124);



	static getFloat = (): f32 => {
		return RandomNode.getInt() / 0x7fffffff;
	}

	static def: zui_node_t = {
		id: 0,
		name: _tr("Random"),
		type: "RandomNode",
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
}
