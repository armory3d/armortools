package arm.node.brush;

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
}
