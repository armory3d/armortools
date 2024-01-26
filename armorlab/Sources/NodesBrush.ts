/// <reference path='./nodes/ImageTextureNode.ts'/>
/// <reference path='./nodes/RGBNode.ts'/>
/// <reference path='./nodes/InpaintNode.ts'/>
/// <reference path='./nodes/PhotoToPBRNode.ts'/>
/// <reference path='./nodes/TextToPhotoNode.ts'/>
/// <reference path='./nodes/TilingNode.ts'/>
/// <reference path='./nodes/UpscaleNode.ts'/>
/// <reference path='./nodes/VarianceNode.ts'/>

class NodesBrush {

	static categories = [_tr("Input"), _tr("Model")];

	static list: TNode[][] = [
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

	static createNode = (nodeType: string): TNode => {
		for (let c of NodesBrush.list) {
			for (let n of c) {
				if (n.type == nodeType) {
					let canvas = Project.canvas;
					let nodes = Project.nodes;
					let node = UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}
}
