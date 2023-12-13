package arm.nodes;

import iron.Vec4;
import zui.Zui.Nodes;
import zui.Zui.TNode;
import arm.LogicNode;
import arm.ParserLogic.f32;
import arm.Translator._tr;

@:keep
class SeparateVectorNode extends LogicNode {

	public function new() {
		super();
	}

	override function get(from: Int, done: Dynamic->Void) {
		inputs[0].get(function(vector: Vec4) {
			if (from == 0) done(vector.x);
			else if (from == 1) done(vector.y);
			else done(vector.z);
		});
	}

	public static var def: TNode = {
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
				default_value: f32([0.0, 0.0, 0.0])
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
