package arm.logic;

import iron.Time;
import zui.Zui.Nodes;
import zui.Zui.TNode;
import arm.logic.LogicNode;
import arm.Translator._tr;

@:keep
class TimeNode extends LogicNode {

	public function new() {
		super();
	}

	override function get(from: Int, done: Dynamic->Void) {
		if (from == 0) done(Time.time());
		else if (from == 1) done(Time.delta);
		else done(Context.raw.brushTime);
	}

	public static var def: TNode = {
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
