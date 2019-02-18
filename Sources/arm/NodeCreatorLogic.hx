package arm;

import armory.system.CyclesFormat;

typedef TNodeList = {
	var categories: Array<TCat>;
}

typedef TCat = {
	var name: String;
	var nodes: Array<TCatNode>;
}

typedef TCatNode = {
	var name: String;
	var type: String;
	var inputs: Array<TCatSocket>;
	var outputs: Array<TCatSocket>;
	var buttons: Array<TCatButton>;
}

typedef TCatSocket = {
	var name: String;
	var type: String;
}

typedef TCatButton = {
	var name: String;
	var type: String;
	var default_value: Dynamic;
	var data: Array<String>;
}

@:access(arm.UINodes)
class NodeCreatorLogic {
	
	public static var list:TNodeList;

	static function nodeColor(type:String):Int {
		switch(type) {
		case 'Logic':
			return 0xffb34f5a;
		case 'Event':
			return 0xffb34f5a;
		case 'Action':
			return 0xffb34f5a;
		case 'Value':
			return 0xffb34f5a;
		case 'Variable':
			return 0xffb34f5a;
		case 'Input':
			return 0xffb34f5a;
		case 'Animation':
			return 0xffb34f5a;
		case 'Physics':
			return 0xffb34f5a;
		case 'Sound':
			return 0xffb34f5a;
		case 'Navmesh':
			return 0xffb34f5a;
		case 'Native':
			return 0xffb34f5a;
		}
		return 0xffb34f5a;
	}

	static function socketColor(type:String):Int {
		switch(type) {
		case 'ACTION':
			return 0xffcc4c4c;
		case 'OBJECT':
			return 0xff268cbf;
		// case 'ARRAY':
		// case 'ANIMACTION':
		case 'SHADER':
			return 0xff4982a0;
		case 'INTEGER':
			return 0xff63c763;
		case 'VALUE':
			return 0xff63c763;
		case 'VECTOR':
			return 0xff63c763;
		case 'RGBA':
			return 0xff63c763;
		case 'STRING':
			return 0xffffffff;
		case 'BOOL':
			return 0xff63c763;
		}
		return 0xffb34f5a;
	}

	static function socketDefault(type:String):Dynamic {
		switch(type) {
		case 'ACTION':
			return 0;
		case 'OBJECT':
			return 0;
		// case 'ARRAY':
		// case 'ANIMACTION':
		case 'SHADER':
			return 0;
		case 'INTEGER':
			return 0;
		case 'VALUE':
			return 0.0;
		case 'VECTOR':
			return [0.0, 0.0, 0.0];
		case 'RGBA':
			return [0.0, 0.0, 0.0, 0.0];
		case 'STRING':
			return '';
		case 'BOOL':
			return false;
		}
		return 0;
	}
	
	public static function draw(cat:Int) {
		var ui = UINodes.inst.ui;
		var getNodeX = UINodes.inst.getNodeX;
		var getNodeY = UINodes.inst.getNodeY;
		var nodes = UINodes.inst.nodes;
		var canvas = UINodes.inst.canvasLogic;
		
		var c:TCat = list.categories[cat];
		for (cnode in c.nodes) {
			if (ui.button(cnode.name)) {
				var node_id = nodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: cnode.name,
					type: cnode.type,
					x: getNodeX(),
					y: getNodeY(),
					color: nodeColor(cnode.type),
					inputs: [],
					outputs: [],
					buttons: []
				};
				for (cinp in cnode.inputs) {
					var soc:TNodeSocket = {
						id: nodes.getSocketId(canvas.nodes),
						node_id: node_id,
						name: cinp.name,
						type: cinp.type,
						color: socketColor(cinp.type),
						default_value: socketDefault(cinp.type)
					};
					n.inputs.push(soc);
				}
				for (cout in cnode.outputs) {
					var soc:TNodeSocket = {
						id: nodes.getSocketId(canvas.nodes),
						node_id: node_id,
						name: cout.name,
						type: cout.type,
						color: socketColor(cout.type),
						default_value: socketDefault(cout.type)
					};
					n.outputs.push(soc);
				}
				for (cbut in cnode.buttons) {
					var but:TNodeButton = {
						name: cbut.name,
						type: cbut.type,
						output: 0,
						default_value: cbut.default_value,
						data: cbut.data
					};
					n.buttons.push(but);
				}
				canvas.nodes.push(n);
				nodes.nodesDrag = true;
				nodes.nodesSelected = [n];
			}
		}
	}
}
