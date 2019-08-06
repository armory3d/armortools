package arm.nodes.brush;

@:keep
class TEX_IMAGE extends LogicNode {

	public var property0:String; // file

	public function new(tree:LogicTree) {
		super(tree);
	}

	override function get(from:Int):Dynamic {
		return property0;
	}
}
