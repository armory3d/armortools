package arm;

import zui.Zui.Nodes;
import zui.Zui.TNode;
import arm.Translator._tr;

class NodesBrush {

	public static var categories = [_tr("Nodes")];

	public static var list: Array<Array<TNode>> = [
		[ // Category 0
			arm.nodes.TEX_IMAGE.def,
			arm.nodes.InputNode.def,
			arm.nodes.MathNode.def,
			arm.nodes.RandomNode.def,
			arm.nodes.SeparateVectorNode.def,
			arm.nodes.TimeNode.def,
			arm.nodes.FloatNode.def,
			arm.nodes.VectorNode.def,
			arm.nodes.VectorMathNode.def
		]
	];

	public static function createNode(nodeType: String): TNode {
		for (c in list) {
			for (n in c) {
				if (n.type == nodeType) {
					var canvas = Context.raw.brush.canvas;
					var nodes = Context.raw.brush.nodes;
					var node = arm.UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}
}
