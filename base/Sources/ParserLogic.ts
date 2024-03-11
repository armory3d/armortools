
class ParserLogic {

	static custom_nodes: map_t<any, any> = map_create();
	static nodes: zui_node_t[];
	static links: zui_node_link_t[];

	static parsed_nodes: string[] = null;
	static parsed_labels: map_t<string, string> = null;
	static node_map: map_t<string, LogicNode>;
	static raw_map: map_t<LogicNode, zui_node_t>;

	static get_logic_node = (node: zui_node_t): LogicNode => {
		return ParserLogic.node_map.get(ParserLogic.node_name(node));
	}

	static get_raw_node = (node: LogicNode): zui_node_t => {
		return ParserLogic.raw_map.get(node);
	}

	static get_node = (id: i32): zui_node_t => {
		for (let n of ParserLogic.nodes) if (n.id == id) return n;
		return null;
	}

	static get_link = (id: i32): zui_node_link_t => {
		for (let l of ParserLogic.links) if (l.id == id) return l;
		return null;
	}

	static get_input_link = (inp: zui_node_socket_t): zui_node_link_t => {
		for (let l of ParserLogic.links) {
			if (l.to_id == inp.node_id) {
				let node: zui_node_t = ParserLogic.get_node(inp.node_id);
				if (node.inputs.length <= l.to_socket) return null;
				if (node.inputs[l.to_socket] == inp) return l;
			}
		}
		return null;
	}

	static get_output_links = (out: zui_node_socket_t): zui_node_link_t[] => {
		let res: zui_node_link_t[] = [];
		for (let l of ParserLogic.links) {
			if (l.from_id == out.node_id) {
				let node: zui_node_t = ParserLogic.get_node(out.node_id);
				if (node.outputs.length <= l.from_socket) continue;
				if (node.outputs[l.from_socket] == out) res.push(l);
			}
		}
		return res;
	}

	static safe_src = (s: string): string => {
		return string_replace_all(s, " ", "");
	}

	static node_name = (node: zui_node_t): string => {
		let s: string = ParserLogic.safe_src(node.name) + node.id;
		return s;
	}

	static parse = (canvas: zui_node_canvas_t) => {
		ParserLogic.nodes = canvas.nodes;
		ParserLogic.links = canvas.links;

		ParserLogic.parsed_nodes = [];
		ParserLogic.parsed_labels = map_create();
		ParserLogic.node_map = map_create();
		ParserLogic.raw_map = map_create();
		let root_nodes: zui_node_t[] = ParserLogic.get_root_nodes(canvas);

		for (let node of root_nodes) ParserLogic.build_node(node);
	}

	static build_node = (node: zui_node_t): string => {
		// Get node name
		let name: string = ParserLogic.node_name(node);

		// Check if node already exists
		if (ParserLogic.parsed_nodes.indexOf(name) != -1) {
			return name;
		}

		ParserLogic.parsed_nodes.push(name);

		// Create node
		let v: any = ParserLogic.create_class_instance(node.type, []);
		ParserLogic.node_map.set(name, v);
		ParserLogic.raw_map.set(v, node);

		// Expose button values in node class
		for (let b of node.buttons) {
			if (b.type == "ENUM") {
				// let arrayData: bool = Array.isArray(b.data);
				let array_data: bool = b.data.length > 1;
				let texts: string[] = array_data ? b.data : zui_enum_texts_js(node.type);
				v[b.name] = texts[b.default_value];
			}
			else {
				v[b.name] = b.default_value;
			}
		}

		// Create inputs
		let inp_node: LogicNode = null;
		let inp_from: i32 = 0;
		for (let i: i32 = 0; i < node.inputs.length; ++i) {
			let inp: zui_node_socket_t = node.inputs[i];
			// Is linked - find node
			let l: zui_node_link_t = ParserLogic.get_input_link(inp);
			if (l != null) {
				inp_node = ParserLogic.node_map.get(ParserLogic.build_node(ParserLogic.get_node(l.from_id)));
				inp_from = l.from_socket;
			}
			// Not linked - create node with default values
			else {
				inp_node = ParserLogic.build_default_node(inp);
				inp_from = 0;
			}
			// Add input
			v.add_input(inp_node, inp_from);
		}

		// Create outputss
		for (let out of node.outputs) {
			let out_nodes: LogicNode[] = [];
			let ls: zui_node_link_t[] = ParserLogic.get_output_links(out);
			if (ls != null && ls.length > 0) {
				for (let l of ls) {
					let n: zui_node_t = ParserLogic.get_node(l.to_id);
					let out_name: string = ParserLogic.build_node(n);
					out_nodes.push(ParserLogic.node_map.get(out_name));
				}
			}
			// Not linked - create node with default values
			else {
				out_nodes.push(ParserLogic.build_default_node(out));
			}
			// Add outputs
			v.add_outputs(out_nodes);
		}

		return name;
	}

	static get_root_nodes = (node_group: zui_node_canvas_t): zui_node_t[] => {
		let roots: zui_node_t[] = [];
		for (let node of node_group.nodes) {
			let linked: bool = false;
			for (let out of node.outputs) {
				let ls: zui_node_link_t[] = ParserLogic.get_output_links(out);
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
			v = ParserLogic.create_class_instance("VectorNode", [inp.default_value[0], inp.default_value[1], inp.default_value[2]]);
		}
		else if (inp.type == "RGBA") {
			if (inp.default_value == null) inp.default_value = [0, 0, 0, 0]; // TODO
			v = ParserLogic.create_class_instance("ColorNode", [inp.default_value[0], inp.default_value[1], inp.default_value[2], inp.default_value[3]]);
		}
		else if (inp.type == "RGB") {
			if (inp.default_value == null) inp.default_value = [0, 0, 0, 0]; // TODO
			v = ParserLogic.create_class_instance("ColorNode", [inp.default_value[0], inp.default_value[1], inp.default_value[2], inp.default_value[3]]);
		}
		else if (inp.type == "VALUE") {
			v = ParserLogic.create_class_instance("FloatNode", [inp.default_value]);
		}
		else if (inp.type == "INT") {
			v = ParserLogic.create_class_instance("IntegerNode", [inp.default_value]);
		}
		else if (inp.type == "BOOLEAN") {
			v = ParserLogic.create_class_instance("BooleanNode", [inp.default_value]);
		}
		else if (inp.type == "STRING") {
			v = ParserLogic.create_class_instance("StringNode", [inp.default_value]);
		}
		else {
			v = ParserLogic.create_class_instance("NullNode", []);
		}
		return v;
	}

	static create_class_instance = (className: string, args: any[]): any => {
		if (ParserLogic.custom_nodes.get(className) != null) {
			let node: LogicNode = new LogicNode();
			node.get = (from: i32) => { return ParserLogic.custom_nodes.get(className)(node, from); }
			return node;
		}
		let dynamic_class: any = eval(`${className}`);
		return new dynamic_class(args);
	}
}
