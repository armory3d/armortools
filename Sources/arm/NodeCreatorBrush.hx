package arm;

import armory.object.Object;
import armory.system.Cycles;
import zui.*;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.MaterialData;

class NodeCreatorBrush {

	public static var numNodes = [8];

	public static function draw(cat:Int) {
		var ui = UINodes.inst.ui;
		var getNodeX = UINodes.inst.getNodeX;
		var getNodeY = UINodes.inst.getNodeY;
		var nodes = UINodes.inst.nodes;
		var canvas = UINodes.inst.canvasBrush;
		
		if (cat == 0) { // Input
			if (ui.button("Input")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Input",
					type: "InputNode",
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
							color: 0xff63c763,
							default_value: null
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Brush Output")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Brush Output",
					type: "BrushOutputNode",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Position",
							type: "VECTOR",
							color: 0xff63c763,
							default_value: [0.0, 0.0, 0.0]
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Radius",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
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
							name: "Strength",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "UV Scale",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 1.0
						}
					],
					outputs: [],
					buttons: []
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
					type: "FloatNode",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xffb34f5a,
					inputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
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
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Value",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.5
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				nodes.nodeDrag = n;
				nodes.nodeSelected = n;
			}
			if (ui.button("Vector")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Vector",
					type: "VectorNode",
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
			if (ui.button("Separate Vector")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Separate Vector",
					type: "SeparateVectorNode",
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
			if (ui.button("Vector Math")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Vector Math",
					type: "VectorMathNode",
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
					type: "MathNode",
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
			if (ui.button("Time")) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Time",
					type: "TimeNode",
					x: getNodeX(),
					y: getNodeY(),
					color: 0xff4982a0,
					inputs: [],
					outputs: [
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Time",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Delta",
							type: "VALUE",
							color: 0xffa1a1a1,
							default_value: 0.0
						},
						{
							id: nodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Brush",
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
