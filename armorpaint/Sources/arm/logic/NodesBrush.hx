package arm.logic;

import zui.Nodes;
import arm.Translator._tr;

class NodesBrush {

	public static var categories = [_tr("Nodes")];

	public static var list: Array<Array<TNode>> = [
		[ // Category 0
			TEX_IMAGE.def,
			InputNode.def,
			MathNode.def,
			RandomNode.def,
			SeparateVectorNode.def,
			TimeNode.def,
			FloatNode.def,
			VectorNode.def,
			VectorMathNode.def
		]
	];

	public static function createNode(nodeType: String): TNode {
		for (c in list) {
			for (n in c) {
				if (n.type == nodeType) {
					var canvas = Context.raw.brush.canvas;
					var nodes = Context.raw.brush.nodes;
					var node = arm.ui.UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}
}
