package arm;

import armory.object.Object;
import armory.system.Cycles;
import zui.*;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.MaterialData;

@:access(arm.UINodes)
class NodeCreator {

	public static function createImageTexture(uinodes:UINodes) {
		var getNodeX = uinodes.getNodeX;
		var getNodeY = uinodes.getNodeY;
		var nodes = UINodes.nodes;
		var canvas = UINodes.canvas;

		var node_id = nodes.getNodeId(canvas.nodes);
		var n:TNode = {
			id: node_id,
			name: "Image Texture",
			type: "TEX_IMAGE",
			x: getNodeX(),
			y: getNodeY(),
			color: 0xff4982a0,
			inputs: [
				{
					id: nodes.getSocketId(canvas.nodes),
					node_id: node_id,
					name: "Vector",
					type: "VECTOR",
					color: 0xff6363c7,
					default_value: [0.0, 0.0, 0.0]
				}
			],
			outputs: [
				{
					id: nodes.getSocketId(canvas.nodes),
					node_id: node_id,
					name: "Color",
					type: "RGBA",
					color: 0xffc7c729,
					default_value: ""
				},
				{
					id: nodes.getSocketId(canvas.nodes),
					node_id: node_id,
					name: "Alpha",
					type: "VALUE",
					color: 0xffa1a1a1,
					default_value: 1.0
				}
			],
			buttons: [
				{
					name: "default_value",
					type: "ENUM",
					default_value: 0,
					output: 0
				}
			]
		};
		canvas.nodes.push(n);
		nodes.nodeDrag = n;
		nodes.nodeSelected = n;
	}

	public static var numNodes = [6, 1, 5, 5, 9];

	public static function draw(uinodes:UINodes, cat:Int) {
		var ui = uinodes.ui;
		var getNodeX = uinodes.getNodeX;
		var getNodeY = uinodes.getNodeY;
		var nodes = UINodes.nodes;
		var canvas = UINodes.canvas;
		
		if (cat == 0) { // Input
			if (ui.button("RGB")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "RGB",
					type: "RGB",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.5, 0.5, 0.5, 1.0]
						}
					],
					buttons: [
						{
							name: "default_value",
							type: "RGBA",
							output: 0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Value")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Value",
					type: "VALUE",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Value",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
						}
					],
					buttons: [
						{
							name: "default_value",
							type: "VALUE",
							output: 0,
							min: 0.0,
							max: 10.0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Camera Data")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Camera Data",
					type: "CAMERA",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "View Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "View Z Depth",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "View Distance",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Geometry")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Geometry",
					type: "NEW_GEOMETRY",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Position",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Normal",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Fresnel")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Fresnel",
					type: "FRESNEL",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "IOR",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.450,
							min: 0.0,
							max: 2.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Layer Weight")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Layer Weight",
					type: "LAYER_WEIGHT",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Blend",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fresnel",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Facing",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
		}
		if (cat == 1) { // Output
			if (ui.button("Material Output")) {

				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Material Output",
					type: "OUTPUT_MATERIAL_PBR",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Base Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8, 1.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Opacity",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Occlusion",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Roughness",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.1
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Metallic",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Normal Map",
							type: "VECTOR",
							color: 0xff63c763,
							default_value: [0.5, 0.5, 1.0]
						}
						// ,
						// {
						// 	id: nodes.getSocketId(canvas.nodes),
						// 	node_id: node_id,
						// 	name: "Emission",
						// 	type: "VALUE",
						// 	color: 0xffa1a1a1,
						// 	default_value: 0.0
						// },
						// {
						// 	id: nodes.getSocketId(canvas.nodes),
						// 	node_id: node_id,
						// 	name: "Height",
						// 	type: "VALUE",
						// 	color: 0xffa1a1a1,
						// 	default_value: 0.0
						// },
						// {
						// 	id: nodes.getSocketId(canvas.nodes),
						// 	node_id: node_id,
						// 	name: "Subsurface",
						// 	type: "VALUE",
						// 	color: 0xffa1a1a1,
						// 	default_value: 0.0
						// }
					],
					outputs: [],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
		}
		if (cat == 2) { // Texture
			if (ui.button("Image")) {
				createImageTexture(uinodes);
			}
			if (ui.button("Checker")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Checker Texture",
					type: "TEX_CHECKER",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color 1",
							type: "RGB",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color 2",
							type: "RGB",
							color: 0xffc7c729,
							default_value: [0.2, 0.2, 0.2]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Scale",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0,
							min: 0.0,
							max: 10.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Gradient")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Gradient Texture",
					type: "TEX_GRADIENT",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						}
					],
					buttons: [
						{
							name: "gradient_type",
							type: "ENUM",
							// data: ["Linear", "Quadratic", "Easing", "Diagonal", "Radial", "Quadratic Sphere", "Spherical"],
							data: ["Linear", "Diagonal", "Radial", "Spherical"],
							default_value: 0,
							output: 0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Noise")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Noise Texture",
					type: "TEX_NOISE",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Scale",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0,
							min: 0.0,
							max: 10.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Voronoi")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Voronoi Texture",
					type: "TEX_VORONOI",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Scale",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0,
							min: 0.0,
							max: 10.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						}
					],
					buttons: [
						{
							name: "coloring",
							type: "ENUM",
							data: ["Intensity", "Cells"],
							default_value: 0,
							output: 0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
		}
		if (cat == 3) { // Color
			if (ui.button("BrightContrast")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "BrightContrast",
					type: "BRIGHTCONTRAST",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Bright",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Contrast",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Gamma")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Gamma",
					type: "GAMMA",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Gamma",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("HueSatVal")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "HueSatVal",
					type: "HUE_SAT",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Hue",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Sat",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Val",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Invert")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Invert",
					type: "INVERT",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("MixRGB")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "MixRGB",
					type: "MIX_RGB",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color1",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.5, 0.5, 0.5]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color2",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.5, 0.5, 0.5]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						}
					],
					buttons: [
						{
							name: "blend_type",
							type: "ENUM",
							data: ["Mix", "Add", "Multiply", "Subtract", "Screen", "Divide", "Difference", "Darken", "Lighten", "Soft Light"],
							default_value: 0,
							output: 0
						},
						{
							name: "use_clamp",
							type: "BOOL",
							default_value: "false",
							output: 0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
		}
		if (cat == 4) { // Converter
			if (ui.button("Combine RGB")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Combine RGB",
					type: "COMBRGB",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "R",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "G",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "B",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Combine XYZ")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Combine XYZ",
					type: "COMBXYZ",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "X",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Y",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Z",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Wavelength")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Wavelength",
					type: "WAVELENGTH",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Wavelength",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 500.0,
							min: 450.0,
							max: 600.0
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Mapping")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Mapping",
					type: "MAPPING",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Translation",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Scale",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [1.0, 1.0, 1.0]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Vector Math")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Vector Math",
					type: "VECT_MATH",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",	
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
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
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Math")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Math",
					type: "MATH",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Value",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Value",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
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
							default_value: "false",
							output: 0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("RGB to BW")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "RGB to BW",
					type: "RGBTOBW",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: ""
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Val",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Separate RGB")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Separate RGB",
					type: "SEPRGB",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "R",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "G",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "B",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Separate XYZ")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Separate XYZ",
					type: "SEPXYZ",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "X",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Y",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Z",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
		}
	}
}
