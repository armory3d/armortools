package arm.node.brush;

import iron.object.Object;

@:keep
class ObjectNode extends LogicNode {

	public var objectName: String;
	public var value: Object;

	public function new(tree: LogicTree, objectName: String = "") {
		this.objectName = objectName;
		super(tree);
	}

	override function get(from: Int): Dynamic {
		if (inputs.length > 0) return inputs[0].get();
		value = objectName != "" ? iron.Scene.active.getChild(objectName) : null;
		return value;
	}

	override function set(value: Dynamic) {
		if (inputs.length > 0) inputs[0].set(value);
		else this.value = value;
	}
}
