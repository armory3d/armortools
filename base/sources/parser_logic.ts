
let parser_logic_custom_nodes: map_t<any, any> = map_create();
let parser_logic_nodes: ui_node_t[];
let parser_logic_links: ui_node_link_t[];

let parser_logic_parsed_nodes: string[] = null;
let parser_logic_node_map: map_t<string, logic_node_ext_t>;

function parser_logic_get_logic_node(node: ui_node_t): logic_node_ext_t {
	return map_get(parser_logic_node_map, parser_logic_node_name(node));
}

function parser_logic_get_node(id: i32): ui_node_t {
	for (let i: i32 = 0; i < parser_logic_nodes.length; ++i) {
		let n: ui_node_t = parser_logic_nodes[i];
		if (n.id == id) {
			return n;
		}
	}
	return null;
}

function parser_logic_get_link(id: i32): ui_node_link_t {
	for (let i: i32 = 0; i < parser_logic_links.length; ++i) {
		let l: ui_node_link_t = parser_logic_links[i];
		if (l.id == id) {
			return l;
		}
	}
	return null;
}

function parser_logic_get_input_link(inp: ui_node_socket_t): ui_node_link_t {
	for (let i: i32 = 0; i < parser_logic_links.length; ++i) {
		let l: ui_node_link_t = parser_logic_links[i];
		if (l.to_id == inp.node_id) {
			let node: ui_node_t = parser_logic_get_node(inp.node_id);
			if (node.inputs.length <= l.to_socket) {
				return null;
			}
			if (node.inputs[l.to_socket] == inp) {
				return l;
			}
		}
	}
	return null;
}

function parser_logic_get_output_links(out: ui_node_socket_t): ui_node_link_t[] {
	let res: ui_node_link_t[] = [];
	for (let i: i32 = 0; i < parser_logic_links.length; ++i) {
		let l: ui_node_link_t = parser_logic_links[i];
		if (l.from_id == out.node_id) {
			let node: ui_node_t = parser_logic_get_node(out.node_id);
			if (node.outputs.length <= l.from_socket) {
				continue;
			}
			if (node.outputs[l.from_socket] == out) {
				array_push(res, l);
			}
		}
	}
	return res;
}

function parser_logic_safe_src(s: string): string {
	return string_replace_all(s, " ", "");
}

function parser_logic_node_name(node: ui_node_t): string {
	let safe: string = parser_logic_safe_src(node.name);
	let nid: i32 = node.id;
	let s: string = safe + nid;
	return s;
}

function parser_logic_parse(canvas: ui_node_canvas_t) {
	parser_logic_nodes = canvas.nodes;
	parser_logic_links = canvas.links;

	parser_logic_parsed_nodes = [];
	parser_logic_node_map = map_create();
	let root_nodes: ui_node_t[] = parser_logic_get_root_nodes(canvas);

	for (let i: i32 = 0; i < root_nodes.length; ++i) {
		let node: ui_node_t = root_nodes[i];
		parser_logic_build_node(node);
	}
}

function parser_logic_build_node(node: ui_node_t): string {
	// Get node name
	let name: string = parser_logic_node_name(node);

	// Check if node already exists
	if (array_index_of(parser_logic_parsed_nodes, name) != -1) {
		return name;
	}

	array_push(parser_logic_parsed_nodes, name);

	// Create node
	let v: logic_node_ext_t = parser_logic_create_node_instance(node.type, node, null);
	map_set(parser_logic_node_map, name, v);

	// Create inputs
	let inp_node: logic_node_ext_t = null;
	let inp_from: i32 = 0;
	for (let i: i32 = 0; i < node.inputs.length; ++i) {
		let inp: ui_node_socket_t = node.inputs[i];
		// Is linked - find node
		let l: ui_node_link_t = parser_logic_get_input_link(inp);
		if (l != null) {
			let n: ui_node_t = parser_logic_get_node(l.from_id);
			let s: string = parser_logic_build_node(n);
			inp_node = map_get(parser_logic_node_map, s);
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
	for (let i: i32 = 0; i < node.outputs.length; ++i) {
		let out: ui_node_socket_t = node.outputs[i];
		let out_nodes: logic_node_t[] = [];
		let ls: ui_node_link_t[] = parser_logic_get_output_links(out);
		if (ls != null && ls.length > 0) {
			for (let i: i32 = 0; i < ls.length; ++i) {
				let l: ui_node_link_t = ls[i];
				let n: ui_node_t = parser_logic_get_node(l.to_id);
				let out_name: string = parser_logic_build_node(n);
				array_push(out_nodes, map_get(parser_logic_node_map, out_name));
			}
		}
		// Not linked - create node with default values
		else {
			array_push(out_nodes, parser_logic_build_default_node(out));
		}
		// Add outputs
		logic_node_add_outputs(v.base, out_nodes);
	}

	return name;
}

function parser_logic_get_root_nodes(node_group: ui_node_canvas_t): ui_node_t[] {
	let roots: ui_node_t[] = [];
	for (let i: i32 = 0; i < node_group.nodes.length; ++i) {
		let node: ui_node_t = node_group.nodes[i];
		let linked: bool = false;
		for (let i: i32 = 0; i < node.outputs.length; ++i) {
			let out: ui_node_socket_t = node.outputs[i];
			let ls: ui_node_link_t[] = parser_logic_get_output_links(out);
			if (ls != null && ls.length > 0) {
				linked = true;
				break;
			}
		}
		if (!linked) { // Assume node with no connected outputs as roots
			array_push(roots, node);
		}
	}
	return roots;
}

function parser_logic_build_default_node(inp: ui_node_socket_t): logic_node_ext_t {
	let v: logic_node_ext_t = null;

	if (inp.type == "VECTOR") {
		if (inp.default_value == null) {
			inp.default_value = f32_array_create_xyz(0, 0, 0);
		}
		v = parser_logic_create_node_instance("vector_node", null, inp.default_value);
	}
	else if (inp.type == "RGBA") {
		if (inp.default_value == null) {
			inp.default_value = f32_array_create_xyzw(0, 0, 0, 0);
		}
		v = parser_logic_create_node_instance("color_node", null, inp.default_value);
	}
	else if (inp.type == "RGB") {
		if (inp.default_value == null) {
			inp.default_value = f32_array_create_xyzw(0, 0, 0, 0);
		}
		v = parser_logic_create_node_instance("color_node", null, inp.default_value);
	}
	else if (inp.type == "VALUE") {
		v = parser_logic_create_node_instance("float_node", null, inp.default_value);
	}
	else if (inp.type == "INT") {
		v = parser_logic_create_node_instance("integer_node", null, inp.default_value);
	}
	else if (inp.type == "BOOLEAN") {
		v = parser_logic_create_node_instance("boolean_node", null, inp.default_value);
	}
	else if (inp.type == "STRING") {
		v = parser_logic_create_node_instance("string_node", null, inp.default_value);
	}
	else {
		v = parser_logic_create_node_instance("null_node", null, null);
	}
	return v;
}

function parser_logic_create_node_instance(node_type: string, raw: ui_node_t, args: f32_array_t): logic_node_ext_t {
	if (map_get(parser_logic_custom_nodes, node_type) != null) {
		let node: logic_node_t = logic_node_create(null);
		node.get = map_get(parser_logic_custom_nodes, node_type);
		node.ext = {
			base: node
		};
		return node.ext;
	}

	if (nodes_brush_creates == null) {
		nodes_brush_init();
	}

	let create: (raw: ui_node_t, args: f32_array_t)=>logic_node_ext_t = map_get(nodes_brush_creates, node_type);
	return create(raw, args);
}
