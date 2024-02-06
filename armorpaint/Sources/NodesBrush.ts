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

	static categories = [_tr("Nodes")];

	static list: zui_node_t[][] = [
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

	static createNode = (nodeType: string): zui_node_t => {
		for (let c of NodesBrush.list) {
			for (let n of c) {
				if (n.type == nodeType) {
					let canvas = Context.raw.brush.canvas;
					let nodes = Context.raw.brush.nodes;
					let node = UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}
}
