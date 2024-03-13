/// <reference path='./nodes/TEX_IMAGE.ts'/>
/// <reference path='./nodes/InputNode.ts'/>
/// <reference path='./../../base/Sources/nodes/MathNode.ts'/>
/// <reference path='./../../base/Sources/nodes/RandomNode.ts'/>
/// <reference path='./../../base/Sources/nodes/SeparateVectorNode.ts'/>
/// <reference path='./../../base/Sources/nodes/TimeNode.ts'/>
/// <reference path='./../../base/Sources/nodes/FloatNode.ts'/>
/// <reference path='./../../base/Sources/nodes/VectorNode.ts'/>
/// <reference path='./../../base/Sources/nodes/VectorMathNode.ts'/>

class NodesBrush {

	static nodes_brush_categories = [_tr("Nodes")];

	static nodes_brush_list: zui_node_t[][] = [
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

	static nodes_brush_create_node = (nodeType: string): zui_node_t => {
		for (let c of NodesBrush.nodes_brush_list) {
			for (let n of c) {
				if (n.type == nodeType) {
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
}
