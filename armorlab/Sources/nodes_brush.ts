/// <reference path='./nodes/image_texture_node.ts'/>
/// <reference path='./nodes/rgb_node.ts'/>
/// <reference path='./nodes/inpaint_node.ts'/>
/// <reference path='./nodes/photo_to_pbr_node.ts'/>
/// <reference path='./nodes/text_to_photo_node.ts'/>
/// <reference path='./nodes/tiling_node.ts'/>
/// <reference path='./nodes/upscale_node.ts'/>
/// <reference path='./nodes/variance_node.ts'/>

let nodes_brush_categories: string[] = [_tr("Input"), _tr("Model")];

let nodes_brush_list: zui_node_t[][] = [
	[ // Input
		image_texture_node.def,
		rgb_node.def,
	],
	[ // Model
		inpaint_node.def,
		photo_to_pbr_node.def,
		text_to_photo_node.def,
		tiling_node.def,
		upscale_node.def,
		variance_node.def,
	]
];

function nodes_brush_create_node(nodeType: string): zui_node_t {
	for (let c of nodes_brush_list) {
		for (let n of c) {
			if (n.type == nodeType) {
				let canvas = project_canvas;
				let nodes = project_nodes;
				let node = ui_nodes_make_node(n, nodes, canvas);
				array_push(canvas.nodes, node);
				return node;
			}
		}
	}
	return null;
}
