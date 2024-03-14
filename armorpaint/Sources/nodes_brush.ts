/// <reference path='./nodes/tex_image_node.ts'/>
/// <reference path='./nodes/input_node.ts'/>

let nodes_brush_categories = [_tr("Nodes")];

let nodes_brush_list: zui_node_t[][] = [
	[ // Category 0
		tex_image_node_def,
		input_node_def,
		math_node_def,
		random_node_def,
		separate_vector_node_def,
		time_node_def,
		float_node_def,
		vector_node_def,
		vector_math_node_def
	]
];

function nodes_brush_create_node(node_type: string): zui_node_t {
	for (let c of nodes_brush_list) {
		for (let n of c) {
			if (n.type == node_type) {
				let canvas: zui_node_canvas_t = context_raw.brush.canvas;
				let nodes: zui_nodes_t = context_raw.brush.nodes;
				let node: zui_node_t = ui_nodes_make_node(n, nodes, canvas);
				canvas.nodes.push(node);
				return node;
			}
		}
	}
	return null;
}
