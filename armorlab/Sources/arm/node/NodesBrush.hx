package arm.node;

import zui.Nodes;

class NodesBrush {

	// Mark strings as localizable
	public static inline function _tr(s: String) {
		return s;
	}

	public static var categories = [_tr("Nodes")];

	public static var list: Array<Array<TNode>> = [
		[ // Category 0
			{
				id: 0,
				name: _tr("Image Texture"),
				type: "ImageTextureNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Vector"),
						type: "VECTOR",
						color: 0xff6363c7,
						default_value: f32([0.0, 0.0, 0.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Alpha"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: [
					{
						name: _tr("file"),
						type: "ENUM",
						default_value: 0,
						data: ""
					},
					{
						name: _tr("color_space"),
						type: "ENUM",
						default_value: 0,
						data: ["linear", "srgb"]
					}
				]
			},
			{
				id: 0,
				name: _tr("Inpaint"),
				type: "InpaintNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([1.0, 1.0, 1.0, 1.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					}
				],
				buttons: [
					{
						name: _tr("auto"),
						type: "BOOL",
						default_value: true,
						output: 0
					},
					{
						name: "arm.node.brush.InpaintNode.buttons",
						type: "CUSTOM",
						height: 0
					}
				]
			},
			{
				id: 0,
				name: _tr("Photo to PBR"),
				type: "PhotoToPBRNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Base Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Occlusion"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Roughness"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Metallic"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.0
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Normal Map"),
						type: "VECTOR",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Height"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 1.0
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: _tr("Text to Photo"),
				type: "TextToPhotoNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					}
				],
				buttons: [
					{
						name: _tr("tiling"),
						type: "BOOL",
						default_value: false,
						output: 0
					},
					{
						name: "arm.node.brush.TextToPhotoNode.buttons",
						type: "CUSTOM",
						height: 1
					}
				]
			},
			{
				id: 0,
				name: _tr("Tiling"),
				type: "TilingNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					}
				],
				buttons: [
					{
						name: _tr("auto"),
						type: "BOOL",
						default_value: true,
						output: 0
					},
					{
						name: "arm.node.brush.TilingNode.buttons",
						type: "CUSTOM",
						height: 0
					}
				]
			},
			{
				id: 0,
				name: _tr("Upscale"),
				type: "UpscaleNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					}
				],
				buttons: []
			},
			{
				id: 0,
				name: _tr("Variance"),
				type: "VarianceNode",
				x: 0,
				y: 0,
				color: 0xff4982a0,
				inputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					},
					{
						id: 0,
						node_id: 0,
						name: _tr("Strength"),
						type: "VALUE",
						color: 0xffa1a1a1,
						default_value: 0.5
					}
				],
				outputs: [
					{
						id: 0,
						node_id: 0,
						name: _tr("Color"),
						type: "RGBA",
						color: 0xffc7c729,
						default_value: f32([0.0, 0.0, 0.0, 1.0])
					}
				],
				buttons: [
					{
						name: "arm.node.brush.VarianceNode.buttons",
						type: "CUSTOM",
						height: 1
					}
				]
			},

			//,
			// {
			// 	id: 0,
			// 	name: _tr("Math"),
			// 	type: "MathNode",
			// 	x: 0,
			// 	y: 0,
			// 	color: 0xff4982a0,
			// 	inputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Value"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.5
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Value"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.5
			// 		}
			// 	],
			// 	outputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Value"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		}
			// 	],
			// 	buttons: [
			// 		{
			// 			name: _tr("operation"),
			// 			type: "ENUM",
			// 			data: ["Add", "Subtract", "Multiply", "Divide", "Power", "Logarithm", "Square Root", "Inverse Square Root", "Absolute", "Exponent", "Minimum", "Maximum", "Less Than", "Greater Than", "Sign", "Round", "Floor", "Ceil", "Truncate", "Fraction", "Modulo", "Snap", "Ping-Pong", "Sine", "Cosine", "Tangent", "Arcsine", "Arccosine", "Arctangent", "Arctan2", "Hyperbolic Sine", "Hyperbolic Cosine", "Hyperbolic Tangent", "To Radians", "To Degrees"],
			// 			default_value: 0,
			// 			output: 0
			// 		},
			// 		{
			// 			name: _tr("use_clamp"),
			// 			type: "BOOL",
			// 			default_value: false,
			// 			output: 0
			// 		}
			// 	]
			// },
			// {
			// 	id: 0,
			// 	name: _tr("Random"),
			// 	type: "RandomNode",
			// 	x: 0,
			// 	y: 0,
			// 	color: 0xffb34f5a,
			// 	inputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Min"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Max"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 1.0
			// 		}
			// 	],
			// 	outputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Value"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.5
			// 		}
			// 	],
			// 	buttons: []
			// },
			// {
			// 	id: 0,
			// 	name: _tr("Separate Vector"),
			// 	type: "SeparateVectorNode",
			// 	x: 0,
			// 	y: 0,
			// 	color: 0xff4982a0,
			// 	inputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Vector"),
			// 			type: "VECTOR",
			// 			color: 0xff6363c7,
			// 			default_value: f32([0.0, 0.0, 0.0])
			// 		}
			// 	],
			// 	outputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("X"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Y"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Z"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		}
			// 	],
			// 	buttons: []
			// },
			// {
			// 	id: 0,
			// 	name: _tr("Value"),
			// 	type: "FloatNode",
			// 	x: 0,
			// 	y: 0,
			// 	color: 0xffb34f5a,
			// 	inputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Value"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.5,
			// 			min: 0.0,
			// 			max: 10.0
			// 		}
			// 	],
			// 	outputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Value"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.5
			// 		}
			// 	],
			// 	buttons: []
			// },
			// {
			// 	id: 0,
			// 	name: _tr("Vector"),
			// 	type: "VectorNode",
			// 	x: 0,
			// 	y: 0,
			// 	color: 0xff4982a0,
			// 	inputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("X"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Y"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Z"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		}
			// 	],
			// 	outputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Vector"),
			// 			type: "VECTOR",
			// 			color: 0xff6363c7,
			// 			default_value: f32([0.0, 0.0, 0.0])
			// 		}
			// 	],
			// 	buttons: []
			// },
			// {
			// 	id: 0,
			// 	name: _tr("Vector Math"),
			// 	type: "VectorMathNode",
			// 	x: 0,
			// 	y: 0,
			// 	color: 0xff4982a0,
			// 	inputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Vector"),
			// 			type: "VECTOR",
			// 			color: 0xff6363c7,
			// 			default_value: f32([0.0, 0.0, 0.0])
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Vector"),
			// 			type: "VECTOR",
			// 			color: 0xff6363c7,
			// 			default_value: f32([0.0, 0.0, 0.0])
			// 		}
			// 	],
			// 	outputs: [
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Vector"),
			// 			type: "VECTOR",
			// 			color: 0xff6363c7,
			// 			default_value: f32([0.0, 0.0, 0.0])
			// 		},
			// 		{
			// 			id: 0,
			// 			node_id: 0,
			// 			name: _tr("Value"),
			// 			type: "VALUE",
			// 			color: 0xffa1a1a1,
			// 			default_value: 0.0
			// 		}
			// 	],
			// 	buttons: [
			// 		{
			// 			name: _tr("operation"),
			// 			type: "ENUM",
			// 			data: ["Add", "Subtract", "Multiply", "Divide", "Average", "Cross Product", "Project", "Reflect", "Dot Product", "Distance", "Length", "Scale", "Normalize", "Absolute", "Minimum", "Maximum", "Floor", "Ceil", "Fraction", "Modulo", "Snap", "Sine", "Cosine", "Tangent"],
			// 			default_value: 0,
			// 			output: 0
			// 		}
			// 	]
			// }
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

	static function f32(ar: Array<kha.FastFloat>): kha.arrays.Float32Array {
		var res = new kha.arrays.Float32Array(ar.length);
		for (i in 0...ar.length) res[i] = ar[i];
		return res;
	}
}
