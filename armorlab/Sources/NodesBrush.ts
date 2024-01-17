
class NodesBrush {

	static categories = [_tr("Input"), _tr("Model")];

	static list: zui.Zui.TNode[][] = [
		[ // Input
			ImageTextureNode.def,
			RGBNode.def,
		],
		[ // Model
			InpaintNode.def,
			PhotoToPBRNode.def,
			TextToPhotoNode.def,
			TilingNode.def,
			UpscaleNode.def,
			VarianceNode.def,
		]
	];

	static createNode = (nodeType: string): zui.Zui.TNode => {
		for (let c of list) {
			for (let n of c) {
				if (n.type == nodeType) {
					let canvas = Project.canvas;
					let nodes = Project.nodes;
					let node = arm.UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}
}
