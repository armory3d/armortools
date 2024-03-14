
let parser_logic_custom_nodes: map_t<any, any> = map_create();
let parser_logic_nodes: zui_node_t[];
let parser_logic_links: zui_node_link_t[];

let parser_logic_parsed_nodes: string[] = null;
let parser_logic_parsed_labels: map_t<string, string> = null;
let parser_logic_node_map: map_t<string, logic_node_t>;
let parser_logic_raw_map: map_t<logic_node_t, zui_node_t>;

function parser_logic_get_logic_node(node: zui_node_t): logic_node_t {
	return parser_logic_node_map.get(parser_logic_node_name(node));
}

function parser_logic_get_raw_node(node: logic_node_t): zui_node_t {
	return parser_logic_raw_map.get(node);
}

function parser_logic_get_node(id: i32): zui_node_t {
	for (let n of parser_logic_nodes) if (n.id == id) return n;
	return null;
}

function parser_logic_get_link(id: i32): zui_node_link_t {
	for (let l of parser_logic_links) if (l.id == id) return l;
	return null;
}

function parser_logic_get_input_link(inp: zui_node_socket_t): zui_node_link_t {
	for (let l of parser_logic_links) {
		if (l.to_id == inp.node_id) {
			let node: zui_node_t = parser_logic_get_node(inp.node_id);
			if (node.inputs.length <= l.to_socket) return null;
			if (node.inputs[l.to_socket] == inp) return l;
		}
	}
	return null;
}

function parser_logic_get_output_links(out: zui_node_socket_t): zui_node_link_t[] {
	let res: zui_node_link_t[] = [];
	for (let l of parser_logic_links) {
		if (l.from_id == out.node_id) {
			let node: zui_node_t = parser_logic_get_node(out.node_id);
			if (node.outputs.length <= l.from_socket) continue;
			if (node.outputs[l.from_socket] == out) res.push(l);
		}
	}
	return res;
}

function parser_logic_safe_src(s: string): string {
	return string_replace_all(s, " ", "");
}

function parser_logic_node_name(node: zui_node_t): string {
	let s: string = parser_logic_safe_src(node.name) + node.id;
	return s;
}

function parser_logic_parse(canvas: zui_node_canvas_t) {
	parser_logic_nodes = canvas.nodes;
	parser_logic_links = canvas.links;

	parser_logic_parsed_nodes = [];
	parser_logic_parsed_labels = map_create();
	parser_logic_node_map = map_create();
	parser_logic_raw_map = map_create();
	let root_nodes: zui_node_t[] = parser_logic_get_root_nodes(canvas);

	for (let node of root_nodes) parser_logic_build_node(node);
}

function parser_logic_build_node(node: zui_node_t): string {
	// Get node name
	let name: string = parser_logic_node_name(node);

	// Check if node already exists
	if (parser_logic_parsed_nodes.indexOf(name) != -1) {
		return name;
	}

	parser_logic_parsed_nodes.push(name);

	// Create node
	let v: any = parser_logic_create_node_instance(node.type, []);
	parser_logic_node_map.set(name, v);
	parser_logic_raw_map.set(v, node);

	// Expose button values in node class
	for (let b of node.buttons) {
		if (b.type == "ENUM") {
			// let array_data: bool = Array.isArray(b.data);
			let array_data: bool = b.data.length > 1;
			let texts: string[] = array_data ? b.data : zui_enum_texts_js(node.type);
			v[b.name] = texts[b.default_value];
		}
		else {
			v[b.name] = b.default_value;
		}
	}

	// Create inputs
	let inp_node: logic_node_t = null;
	let inp_from: i32 = 0;
	for (let i: i32 = 0; i < node.inputs.length; ++i) {
		let inp: zui_node_socket_t = node.inputs[i];
		// Is linked - find node
		let l: zui_node_link_t = parser_logic_get_input_link(inp);
		if (l != null) {
			inp_node = parser_logic_node_map.get(parser_logic_build_node(parser_logic_get_node(l.from_id)));
			inp_from = l.from_socket;
		}
		// Not linked - create node with default values
		else {
			inp_node = parser_logic_build_default_node(inp);
			inp_from = 0;
		}
		// Add input
		logic_node_add_input(v.base, inp_node, inp_from);
	}

	// Create outputss
	for (let out of node.outputs) {
		let out_nodes: logic_node_t[] = [];
		let ls: zui_node_link_t[] = parser_logic_get_output_links(out);
		if (ls != null && ls.length > 0) {
			for (let l of ls) {
				let n: zui_node_t = parser_logic_get_node(l.to_id);
				let out_name: string = parser_logic_build_node(n);
				out_nodes.push(parser_logic_node_map.get(out_name));
			}
		}
		// Not linked - create node with default values
		else {
			out_nodes.push(parser_logic_build_default_node(out));
		}
		// Add outputs
		logic_node_add_outputs(v.base, out_nodes);
	}

	return name;
}

function parser_logic_get_root_nodes(node_group: zui_node_canvas_t): zui_node_t[] {
	let roots: zui_node_t[] = [];
	for (let node of node_group.nodes) {
		let linked: bool = false;
		for (let out of node.outputs) {
			let ls: zui_node_link_t[] = parser_logic_get_output_links(out);
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

function parser_logic_build_default_node(inp: zui_node_socket_t): logic_node_t {
	let v: logic_node_t = null;

	if (inp.type == "VECTOR") {
		if (inp.default_value == null) inp.default_value = [0, 0, 0]; // TODO
		v = parser_logic_create_node_instance("vector_node", [inp.default_value[0], inp.default_value[1], inp.default_value[2]]);
	}
	else if (inp.type == "RGBA") {
		if (inp.default_value == null) inp.default_value = [0, 0, 0, 0]; // TODO
		v = parser_logic_create_node_instance("color_node", [inp.default_value[0], inp.default_value[1], inp.default_value[2], inp.default_value[3]]);
	}
	else if (inp.type == "RGB") {
		if (inp.default_value == null) inp.default_value = [0, 0, 0, 0]; // TODO
		v = parser_logic_create_node_instance("color_node", [inp.default_value[0], inp.default_value[1], inp.default_value[2], inp.default_value[3]]);
	}
	else if (inp.type == "VALUE") {
		v = parser_logic_create_node_instance("float_node", [inp.default_value]);
	}
	else if (inp.type == "INT") {
		v = parser_logic_create_node_instance("integer_node", [inp.default_value]);
	}
	else if (inp.type == "BOOLEAN") {
		v = parser_logic_create_node_instance("boolean_node", [inp.default_value]);
	}
	else if (inp.type == "STRING") {
		v = parser_logic_create_node_instance("string_node", [inp.default_value]);
	}
	else {
		v = parser_logic_create_node_instance("null_node", []);
	}
	return v;
}

function parser_logic_create_node_instance(node_type: string, args: any[]): any {
	if (parser_logic_custom_nodes.get(node_type) != null) {
		let node: logic_node_t = logic_node_create();
		node.get = (from: i32) => { return parser_logic_custom_nodes.get(node_type)(node, from); }
		return node;
	}

	let eval_args: string = "";
	for (let arg of args) {
		if (eval_args != "") {
			eval_args += ",";
		}
		eval_args += arg + "";
	}

	return eval(node_type + "_create(" + eval_args + ")");
}
