/// <reference path='./nodes/ImageTextureNode.ts'/>
/// <reference path='./nodes/RGBNode.ts'/>
/// <reference path='./nodes/InpaintNode.ts'/>
/// <reference path='./nodes/PhotoToPBRNode.ts'/>
/// <reference path='./nodes/TextToPhotoNode.ts'/>
/// <reference path='./nodes/TilingNode.ts'/>
/// <reference path='./nodes/UpscaleNode.ts'/>
/// <reference path='./nodes/VarianceNode.ts'/>

let nodes_brush_categories: string[] = [_tr("Input"), _tr("Model")];

let nodes_brush_list: zui_node_t[][] = [
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

function nodes_brush_create_node(nodeType: string): zui_node_t {
	for (let c of nodes_brush_list) {
		for (let n of c) {
			if (n.type == nodeType) {
				let canvas = project_canvas;
				let nodes = project_nodes;
				let node = ui_nodes_make_node(n, nodes, canvas);
				canvas.nodes.push(node);
				return node;
			}
		}
	}
	return null;
}
