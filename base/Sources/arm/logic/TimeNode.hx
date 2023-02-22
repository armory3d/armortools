package arm.logic;

import zui.Nodes;
import arm.logic.LogicNode;
import arm.Translator._tr;

@:keep
class TimeNode extends LogicNode {

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function get(from: Int, done: Dynamic->Void) {
		if (from == 0) done(iron.system.Time.time());
		else if (from == 1) done(iron.system.Time.delta);
		else done(Context.brushTime);
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
