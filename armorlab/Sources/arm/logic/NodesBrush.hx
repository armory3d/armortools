package arm.logic;

import zui.Nodes;
import arm.Translator._tr;

class NodesBrush {

	public static var categories = [_tr("Nodes")];

	public static var list: Array<Array<TNode>> = [
		[ // Category 0
			ImageTextureNode.def,
			InpaintNode.def,
			PhotoToPBRNode.def,
			TextToPhotoNode.def,
			TilingNode.def,
			UpscaleNode.def,
			VarianceNode.def
		]
	];

	public static function createNode(nodeType: String): TNode {
		for (c in list) {
			for (n in c) {
				if (n.type == nodeType) {
					var canvas = Project.canvas;
					var nodes = Project.nodes;
					var node = arm.ui.UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}
}
