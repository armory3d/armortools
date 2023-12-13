package arm;

import zui.Zui.Nodes;
import arm.Translator._tr;

class NodesBrush {

	public static var categories = [_tr("Input"), _tr("Model")];

	public static var list: Array<Array<zui.Zui.TNode>> = [
		[ // Input
			arm.nodes.ImageTextureNode.def,
			arm.nodes.RGBNode.def,
		],
		[ // Model
			arm.nodes.InpaintNode.def,
			arm.nodes.PhotoToPBRNode.def,
			arm.nodes.TextToPhotoNode.def,
			arm.nodes.TilingNode.def,
			arm.nodes.UpscaleNode.def,
			arm.nodes.VarianceNode.def,
		]
	];

	public static function createNode(nodeType: String): zui.Zui.TNode {
		for (c in list) {
			for (n in c) {
				if (n.type == nodeType) {
					var canvas = Project.canvas;
					var nodes = Project.nodes;
					var node = arm.UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}
}
