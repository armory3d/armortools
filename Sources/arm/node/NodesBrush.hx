package arm.node;

import zui.Nodes;

class NodesBrush {

	public static var categories = ["Nodes"];

	public static var list: Array<Array<TNode>> = [
		[ // Category 0
			{
				id: 0,
				name: "Input",
				type: "InputNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Lazy Radius",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Lazy Step",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Position",
						type: "VECTOR",
						color: 0xff63c763,
						default_value: null
					}
				],
				buttons: []
			},
			// {
			// 	id: 0,
			// 	name: "Brush Output",
			// 	type: "BrushOutputNode",
			// 	x: 0,
			// 	y: 0,
			// 	color: 0xff4982a0,
			// 	inputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: "Position",
			// 			type: "VECTOR",
			// 			color: 0xff63c763,
			// 			default_value: f32([0.0, 0.0, 0.0])
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: "Radius",
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 1.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: "Scale",
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 1.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: "Angle",
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: "Opacity",
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 1.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: "Hardness",
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 1.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: "Stencil",
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 1.0
			// 		}
			// 	],
			// 	outputs: [],
			// 	buttons: [
			//		{
			//			name: "Directional",
			//			type: "BOOL",
			//			default_value: false,
			//			output: 0
			//		}
			// ]
			// },
			{
				id: 0,
				name: "Value",
				type: "FloatNode",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5,
						min: 0.0,
						max: 10.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Random",
				type: "RandomNode",
				x: 0,
				y: 0,
				color: 0xffb34f5a,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Min",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Max",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Vector",
				type: "VectorNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "X",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Y",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Z",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Separate Vector",
				type: "SeparateVectorNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "X",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Y",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Z",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Vector Math",
				type: "VectorMathNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					},
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: "operation",
						type: "ENUM",
						data: ["Add", "Subtract", "Average", "Dot Product", "Cross Product", "Normalize"],
						default_value: 0,
						output: 0
					}
				]
			},
			{
				id: 0,
				name: "Math",
				type: "MathNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					},
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Value",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: [
					{
						name: "operation",
						type: "ENUM",
						data: ["Add", "Subtract", "Multiply", "Divide", "Sine", "Cosine", "Tangent", "Arcsine", "Arccosine", "Arctangent", "Power", "Logarithm", "Minimum", "Maximum", "Round", "Less Than", "Greater Than", "Module", "Absolute"],
						default_value: 0,
						output: 0
					},
					{
						name: "use_clamp",
						type: "BOOL",
						default_value: false,
						output: 0
					}
				]
			},
			{
				id: 0,
				name: "Time",
				type: "TimeNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Time",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Delta",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: "Brush",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: "Image Texture",
				type: "TEX_IMAGE",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: "Vector",
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: "Color",
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: "Alpha",
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: "file",
						type: "ENUM",
						default_value: 0,
						data: ""
					},
					{
						name: "color_space",
						type: "ENUM",
						default_value: 0,
						data: ["linear", "srgb"]
					}
				]
			}
		]
	];

	public static function createNode(nodeType: String): TNode {
		for (c in list) {
			for (n in c) {
				if (n.type == nodeType) {
					var canvas = Context.brush.canvas;
					var nodes = Context.brush.nodes;
					var node = arm.ui.UINodes.makeNode(n, nodes, canvas);
					canvas.nodes.push(node);
					return node;
				}
			}
		}
		return null;
	}

	static function f32(ar: Array<kha.FastFloat>): kha.arrays.Float32Array {
		var res = new kha.arrays.Float32Array(ar.length);
		for (i in 0...ar.length) res[i] = ar[i];
		return res;
	}
}
