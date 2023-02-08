package arm.node.brush;

@:keep
class TEX_IMAGE extends LogicNode {

	public var file: String;
	public var color_space: String;

	public function new(tree: LogicTree) {
		super(tree);
	}

	override function get(from: Int): Dynamic {
		if (from == 0) return file + ".rgb";
		return file + ".a";
	}
}
