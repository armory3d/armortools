
class ParserLogic {

	static customNodes = new Map();
	static nodes: zui_node_t[];
	static links: zui_node_link_t[];

	static parsed_nodes: string[] = null;
	static parsed_labels: Map<string, string> = null;
	static nodeMap: Map<string, LogicNode>;
	static rawMap: Map<LogicNode, zui_node_t>;

	static getLogicNode = (node: zui_node_t): LogicNode => {
		return ParserLogic.nodeMap.get(ParserLogic.node_name(node));
	}

	static getRawNode = (node: LogicNode): zui_node_t => {
		return ParserLogic.rawMap.get(node);
	}

	static getNode = (id: i32): zui_node_t => {
		for (let n of ParserLogic.nodes) if (n.id == id) return n;
		return null;
	}

	static getLink = (id: i32): zui_node_link_t => {
		for (let l of ParserLogic.links) if (l.id == id) return l;
		return null;
	}

	static getInputLink = (inp: zui_node_socket_t): zui_node_link_t => {
		for (let l of ParserLogic.links) {
			if (l.to_id == inp.node_id) {
				let node = ParserLogic.getNode(inp.node_id);
				if (node.inputs.length <= l.to_socket) return null;
				if (node.inputs[l.to_socket] == inp) return l;
			}
		}
		return null;
	}

	static getOutputLinks = (out: zui_node_socket_t): zui_node_link_t[] => {
		let res: zui_node_link_t[] = [];
		for (let l of ParserLogic.links) {
			if (l.from_id == out.node_id) {
				let node = ParserLogic.getNode(out.node_id);
				if (node.outputs.length <= l.from_socket) continue;
				if (node.outputs[l.from_socket] == out) res.push(l);
			}
		}
		return res;
	}

	static safe_src = (s: string): string => {
		return s.replaceAll(" ", "");
	}

	static node_name = (node: zui_node_t): string => {
		let s = ParserLogic.safe_src(node.name) + node.id;
		return s;
	}

	static parse = (canvas: zui_node_canvas_t) => {
		ParserLogic.nodes = canvas.nodes;
		ParserLogic.links = canvas.links;

		ParserLogic.parsed_nodes = [];
		ParserLogic.parsed_labels = new Map();
		ParserLogic.nodeMap = new Map();
		ParserLogic.rawMap = new Map();
		let root_nodes = ParserLogic.get_root_nodes(canvas);

		for (let node of root_nodes) ParserLogic.build_node(node);
	}

	static build_node = (node: zui_node_t): string => {
		// Get node name
		let name = ParserLogic.node_name(node);

		// Check if node already exists
		if (ParserLogic.parsed_nodes.indexOf(name) != -1) {
			return name;
		}

		ParserLogic.parsed_nodes.push(name);

		// Create node
		let v = ParserLogic.createClassInstance(node.type, []);
		ParserLogic.nodeMap.set(name, v);
		ParserLogic.rawMap.set(v, node);

		// Expose button values in node class
		for (let b of node.buttons) {
			if (b.type == "ENUM") {
				// let arrayData = Array.isArray(b.data);
				let arrayData = b.data.length > 1;
				let texts = arrayData ? b.data : zui_enum_texts_js(node.type);
				v[b.name] = texts[b.default_value];
			}
			else {
				v[b.name] = b.default_value;
			}
		}

		// Create inputs
		let inp_node: LogicNode = null;
		let inp_from = 0;
		for (let i = 0; i < node.inputs.length; ++i) {
			let inp = node.inputs[i];
			// Is linked - find node
			let l = ParserLogic.getInputLink(inp);
			if (l != null) {
				inp_node = ParserLogic.nodeMap.get(ParserLogic.build_node(ParserLogic.getNode(l.from_id)));
				inp_from = l.from_socket;
			}
			// Not linked - create node with default values
			else {
				inp_node = ParserLogic.build_default_node(inp);
				inp_from = 0;
			}
			// Add input
			v.addInput(inp_node, inp_from);
		}

		// Create outputss
		for (let out of node.outputs) {
			let outNodes:LogicNode[] = [];
			let ls = ParserLogic.getOutputLinks(out);
			if (ls != null && ls.length > 0) {
				for (let l of ls) {
					let n = ParserLogic.getNode(l.to_id);
					let out_name = ParserLogic.build_node(n);
					outNodes.push(ParserLogic.nodeMap.get(out_name));
				}
			}
			// Not linked - create node with default values
			else {
				outNodes.push(ParserLogic.build_default_node(out));
			}
			// Add outputs
			v.addOutputs(outNodes);
		}

		return name;
	}

	static get_root_nodes = (node_group: zui_node_canvas_t): zui_node_t[] => {
		let roots: zui_node_t[] = [];
		for (let node of node_group.nodes) {
			let linked = false;
			for (let out of node.outputs) {
				let ls = ParserLogic.getOutputLinks(out);
				if (ls != null && ls.length > 0) {
					linked = true;
					break;
				}
			}
			if (!linked) { // Assume node with no connected outputs as roots
				roots.push(node);
			}
		}
		return roots;
	}

	static build_default_node = (inp: zui_node_socket_t): LogicNode => {

		let v: LogicNode = null;

		if (inp.type == "VECTOR") {
			if (inp.default_value == null) inp.default_value = [0, 0, 0]; // TODO
			v = ParserLogic.createClassInstance("VectorNode", [inp.default_value[0], inp.default_value[1], inp.default_value[2]]);
		}
		else if (inp.type == "RGBA") {
			if (inp.default_value == null) inp.default_value = [0, 0, 0, 0]; // TODO
			v = ParserLogic.createClassInstance("ColorNode", [inp.default_value[0], inp.default_value[1], inp.default_value[2], inp.default_value[3]]);
		}
		else if (inp.type == "RGB") {
			if (inp.default_value == null) inp.default_value = [0, 0, 0, 0]; // TODO
			v = ParserLogic.createClassInstance("ColorNode", [inp.default_value[0], inp.default_value[1], inp.default_value[2], inp.default_value[3]]);
		}
		else if (inp.type == "VALUE") {
			v = ParserLogic.createClassInstance("FloatNode", [inp.default_value]);
		}
		else if (inp.type == "INT") {
			v = ParserLogic.createClassInstance("IntegerNode", [inp.default_value]);
		}
		else if (inp.type == "BOOLEAN") {
			v = ParserLogic.createClassInstance("BooleanNode", [inp.default_value]);
		}
		else if (inp.type == "STRING") {
			v = ParserLogic.createClassInstance("StringNode", [inp.default_value]);
		}
		else {
			v = ParserLogic.createClassInstance("NullNode", []);
		}
		return v;
	}

	static createClassInstance = (className: string, args: any[]): any => {
		if (ParserLogic.customNodes.get(className) != null) {
			let node = new LogicNode();
			node.get = (from: i32) => { return ParserLogic.customNodes.get(className)(node, from); }
			return node;
		}
		let dynamic_class = eval(`${className}`);
		return new dynamic_class(args);
	}
}
