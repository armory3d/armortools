
class TimeNode extends LogicNode {

	constructor() {
		super();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		if (from == 0) done(time_time());
		else if (from == 1) done(time_delta());
		else done(Context.raw.brushTime);
	}

	static def: zui_node_t = {
		id: 0,
		name: _tr("Time"),
		type: "TimeNode",
		x: 0,
		y: 0,
		color: 0xff4982a0,
		inputs: [],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Time"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Delta"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			},
			{
				id: 0,
				node_id: 0,
				name: _tr("Brush"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.0
			}
		],
		buttons: []
	};
}
