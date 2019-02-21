package arm;

import zui.*;
import zui.Nodes;

@:access(arm.UINodes)
class NodeCreator {

	public static var numNodes = [12, 1, 9, 5, 4, 10];

	public static function createImageTexture():TNode {
		var getNodeX = UINodes.inst.getNodeX;
		var getNodeY = UINodes.inst.getNodeY;
		var nodes = UINodes.inst.nodes;
		var canvas = UINodes.inst.canvas;

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
					default_value: [0.0, 0.0, 0.0, 1.0]
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
					name: "File",
					type: "ENUM",
					default_value: 0,
					data: ""
				},
				{
					name: "Color Space",
					type: "ENUM",
					default_value: 0,
					data: ["linear", "srgb"]
				}
			]
		};
		canvas.nodes.push(n);
		return n;
	}

	public static function draw(cat:Int) {
		var ui = UINodes.inst.ui;
		var getNodeX = UINodes.inst.getNodeX;
		var getNodeY = UINodes.inst.getNodeY;
		var nodes = UINodes.inst.nodes;
		var canvas = UINodes.inst.canvas;
		
		if (cat == 0) { // Input
			if (ui.button("Attribute", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Attribute",
					type: "ATTRIBUTE",
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
							default_value: [0.8, 0.8, 0.8, 1.0]
						},
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
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: [
						{
							name: "Name",
							type: "STRING"
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Camera Data", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Camera Data",
					type: "CAMERA",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Fresnel", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Fresnel",
					type: "FRESNEL",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "IOR",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.45,
							min: 0,
							max: 3
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Geometry", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Geometry",
					type: "NEW_GEOMETRY",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
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
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Tangent",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "True Normal",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Incoming",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Parametric",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Backfacing",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Pointiness",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Layer Weight", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Layer Weight",
					type: "LAYER_WEIGHT",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Blend",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Object Info", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Object Info",
					type: "OBJECT_INFO",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Location",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Object Index",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Material Index",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Random",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("RGB", Left)) {
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Tangent", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Tangent",
					type: "TANGENT",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Tangent",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Texture Coord", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Texture Coord",
					type: "TEX_COORD",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Generated",
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
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "UV",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Object",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Camera",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Window",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Reflection",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("UV Map", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "UV Map",
					type: "UVMAP",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "UV",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Value", Left)) {
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
		}
		if (cat == 1) { // Output
			if (ui.button("Material Output", Left)) {

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
							color: -10238109,
							default_value: [0.5, 0.5, 1.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Emission",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Height",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Subsurface",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					outputs: [],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
		}
		if (cat == 2) { // Texture
			if (ui.button("Brick", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Brick Texture",
					type: "TEX_BRICK",
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
							name: "Color 3",
							type: "RGB",
							color: 0xffc7c729,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Scale",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 5.0,
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
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Checker", Left)) {
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
							default_value: 5.0,
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
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Gradient", Left)) {
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
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Image", Left)) {
				var n = createImageTexture();
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Magic", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Magic Texture",
					type: "TEX_MAGIC",
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
							default_value: 5.0,
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
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Musgrave", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Musgrave Texture",
					type: "TEX_MUSGRAVE",
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
							default_value: 5.0,
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
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Noise", Left)) {
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
							default_value: 5.0,
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
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Voronoi", Left)) {
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
							default_value: 5.0,
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
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Wave", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Wave Texture",
					type: "TEX_WAVE",
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
							default_value: 5.0,
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
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
		}
		if (cat == 3) { // Color
			if (ui.button("BrightContrast", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "BrightContrast",
					type: "BRIGHTCONTRAST",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff448c6d,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8, 1.0]
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
							default_value: [0.8, 0.8, 0.8, 1.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Gamma", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Gamma",
					type: "GAMMA",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff448c6d,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8, 1.0]
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
							default_value: [0.8, 0.8, 0.8, 1.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("HueSatVal", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "HueSatVal",
					type: "HUE_SAT",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff448c6d,
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
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8, 1.0]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8, 1.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Invert", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Invert",
					type: "INVERT",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff448c6d,
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
							default_value: [0.8, 0.8, 0.8, 1.0]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8, 1.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("MixRGB", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "MixRGB",
					type: "MIX_RGB",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff448c6d,
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
							default_value: [0.5, 0.5, 0.5, 1.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color2",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.5, 0.5, 0.5, 1.0]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			// CURVE_RGB
		}
		if (cat == 4) { // Vector
			if (ui.button("Bump", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Bump",
					type: "BUMP",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff522c99,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Strength",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Distance",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Height",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
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
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							// name: "Normal",
							name: "Normal Map",
							type: "VECTOR",
							color: -10238109,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Mapping", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Mapping",
					type: "MAPPING",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff522c99,
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
							name: "Vector",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					buttons: [
						{
							name: "Location",
							type: "VECTOR",
							default_value: [0.0, 0.0, 0.0],
							output: 0
						},
						{
							name: "Rotation",
							type: "VECTOR",
							default_value: [0.0, 0.0, 0.0],
							output: 0
						},
						{
							name: "Scale",
							type: "VECTOR",
							default_value: [1.0, 1.0, 1.0],
							output: 0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Normal", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Normal",
					type: "NORMAL",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff522c99,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Normal",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Normal",
							type: "VECTOR",
							color: 0xff6363c7,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Dot",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						}
					],
					buttons: [
						{
							name: "Vector",
							type: "VECTOR",
							default_value: [0.0, 0.0, 0.0],
							output: 0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Vector Curves", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Vector Curves",
					type: "CURVE_VEC",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff522c99,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
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
						}
					],
					buttons: [
						{
							name: "Vector",
							type: "CURVES",
							default_value: [[0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0]],
							output: 0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			// VECT_TRANSFORM
		}
		if (cat == 5) { // Converter
			if (ui.button("Color Ramp", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Color Ramp",
					type: "VALTORGB",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Fac",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.0, 0.0, 0.0, 1.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Alpha",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: [
						{
							name: "Ramp",
							type: "RAMP",
							default_value: [[1.0, 1.0, 1.0, 1.0, 0.0]],
							data: 0,
							output: 0
						}
					]
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Combine HSV", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Combine HSV",
					type: "COMBHSV",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "H",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "S",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "V",
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
							default_value: [0.8, 0.8, 0.8, 1.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Combine RGB", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Combine RGB",
					type: "COMBRGB",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
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
							default_value: [0.8, 0.8, 0.8, 1.0]
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Combine XYZ", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Combine XYZ",
					type: "COMBXYZ",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Math", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Math",
					type: "MATH",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("RGB to BW", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "RGB to BW",
					type: "RGBTOBW",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.0, 0.0, 0.0, 0.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Separate HSV", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Separate HSV",
					type: "SEPHSV",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.5, 0.5, 0.5, 1.0]
						}
					],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "H",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "S",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "V",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Separate RGB", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Separate RGB",
					type: "SEPRGB",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Color",
							type: "RGBA",
							color: 0xffc7c729,
							default_value: [0.8, 0.8, 0.8, 1.0]
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Separate XYZ", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Separate XYZ",
					type: "SEPXYZ",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
			if (ui.button("Vector Math", Left)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Vector Math",
					type: "VECT_MATH",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff62676d,
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
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
		}
	}
}
