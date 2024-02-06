
class SeparateVectorNode extends LogicNode {

	constructor() {
		super();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		this.inputs[0].get((vector: vec4_t) => {
			if (from == 0) done(vector.x);
			else if (from == 1) done(vector.y);
			else done(vector.z);
		});
	}

	static def: zui_node_t = {
		id: 0,
		name: _tr("Separate Vector"),
		type: "SeparateVectorNode",
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
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("X"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Y"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Z"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			}
		],
		buttons: []
	};
}
