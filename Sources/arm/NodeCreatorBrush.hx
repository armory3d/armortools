package arm;

import armory.object.Object;
import armory.system.Cycles;
import zui.*;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.MaterialData;

@:access(arm.UINodes)
class NodeCreator {

	public static var numNodes = [2];

	public static function draw(uinodes:UINodes, cat:Int) {
		var ui = uinodes.ui;
		var getNodeX = uinodes.getNodeX;
		var getNodeY = uinodes.getNodeY;
		var nodes = UINodes.nodes;
		var canvas = UINodes.canvas;
		
		if (cat == 0) { // Input
			if (ui.button("On Brush")) {
				var node_id = uinodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "On Brush",
					type: "OnBrushNode",
					x: mouse.x - wx,
					y: mouse.y - wy,
					color: 0xff4982a0,
					inputs: [],
					outputs: [
						{
							id: uinodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Out",
							type: "ACTION",
							color: 0xff63c763,
							default_value: null
						},
						{
							id: uinodes.getSocketId(canvas.nodes),
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
				uinodes.nodeDrag = n;
				uinodes.nodeSelected = n;
			}
			if (ui.button("Brush Output")) {
				var node_id = uinodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Brush Output",
					type: "PaintNode",
					x: mouse.x - wx,
					y: mouse.y - wy,
					color: 0xff4982a0,
					inputs: [
						{
							id: uinodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "In",
							type: "ACTION",
							color: 0xff63c763,
							default_value: null
						},
						{
							id: uinodes.getSocketId(canvas.nodes),
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
				uinodes.nodeDrag = n;
				uinodes.nodeSelected = n;
			}
		}
	}
}
