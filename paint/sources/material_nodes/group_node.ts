
let _nodes_material_nodes: ui_nodes_t;
let _nodes_material_node: ui_node_t;
let _nodes_material_sockets: ui_node_socket_t[];

function group_node_init() {
	array_push(nodes_material_group, group_node_def);
	map_set(parser_material_node_vectors, "GROUP", group_node_vector);
	map_set(parser_material_node_values, "GROUP", group_node_value);

	map_set(ui_nodes_custom_buttons, "nodes_material_new_group_button", nodes_material_new_group_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_group_input_button", nodes_material_group_input_button);
	map_set(ui_nodes_custom_buttons, "nodes_material_group_output_button", nodes_material_group_output_button);
}

function group_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	return parser_material_parse_group(node, socket);
}

function group_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	return parser_material_parse_group(node, socket);
}

function nodes_material_new_group_button(node_id: i32) {
	let node: ui_node_t = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);

	if (node.name == "New Group") {
		for (let i: i32 = 1; i < 999; ++i) {
			node.name = tr("Group") + " " + i;

			let found: bool = false;
			for (let i: i32 = 0; i < project_material_groups.length; ++i) {
				let g: node_group_t = project_material_groups[i];
				let cname: string   = g.canvas.name;
				if (cname == node.name) {
					found = true;
					break;
				}
			}
			if (!found) {
				break;
			}
		}

		let canvas: ui_node_canvas_t = {
			name : node.name,
			nodes : [
				{
					id : 0,
					name : _tr("Group Input"),
					type : "GROUP_INPUT",
					x : 50,
					y : 200,
					color : 0xff448c6d,
					inputs : [],
					outputs : [],
					buttons : [ {name : "nodes_material_group_input_button", type : "CUSTOM", height : 1} ],
					width : 0,
					flags : 0
				},
				{
					id : 1,
					name : _tr("Group Output"),
					type : "GROUP_OUTPUT",
					x : 450,
					y : 200,
					color : 0xff448c6d,
					inputs : [],
					outputs : [],
					buttons : [ {name : "nodes_material_group_output_button", type : "CUSTOM", height : 1} ],
					width : 0,
					flags : 0
				}
			],
			links : []
		};
		let ng: node_group_t = {canvas : canvas, nodes : ui_nodes_create()};
		array_push(project_material_groups, ng);
	}

	let group: node_group_t = null;
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		let cname: string   = g.canvas.name;
		if (cname == node.name) {
			group = g;
			break;
		}
	}

	if (ui_button(tr("Nodes"))) {
		array_push(ui_nodes_group_stack, group);
	}
}

function nodes_material_group_input_button(node_id: i32) {
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t   = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);
	nodes_material_add_socket_button(nodes, node, node.outputs);
}

function nodes_material_group_output_button(node_id: i32) {
	let nodes: ui_nodes_t = ui_nodes_get_nodes();
	let node: ui_node_t   = ui_get_node(ui_nodes_get_canvas(true).nodes, node_id);
	nodes_material_add_socket_button(nodes, node, node.inputs);
}

function nodes_material_add_socket_button(nodes: ui_nodes_t, node: ui_node_t, sockets: ui_node_socket_t[]) {
	if (ui_button(tr("Add"))) {
		_nodes_material_nodes   = nodes;
		_nodes_material_node    = node;
		_nodes_material_sockets = sockets;
		ui_menu_draw(function() {
			let nodes: ui_nodes_t           = _nodes_material_nodes;
			let node: ui_node_t             = _nodes_material_node;
			let sockets: ui_node_socket_t[] = _nodes_material_sockets;

			let group_stack: node_group_t[] = ui_nodes_group_stack;
			let c: ui_node_canvas_t         = group_stack[group_stack.length - 1].canvas;
			if (ui_menu_button(tr("RGBA"))) {
				array_push(sockets, nodes_material_create_socket(nodes, node, null, "RGBA", c));
				nodes_material_sync_sockets(node);
			}
			if (ui_menu_button(tr("Vector"))) {
				array_push(sockets, nodes_material_create_socket(nodes, node, null, "VECTOR", c));
				nodes_material_sync_sockets(node);
			}
			if (ui_menu_button(tr("Value"))) {
				array_push(sockets, nodes_material_create_socket(nodes, node, null, "VALUE", c));
				nodes_material_sync_sockets(node);
			}
		});
	}
}

function nodes_material_sync_sockets(node: ui_node_t) {
	let group_stack: node_group_t[] = ui_nodes_group_stack;
	let c: ui_node_canvas_t         = group_stack[group_stack.length - 1].canvas;
	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		nodes_material_sync_group_sockets(m.canvas, c.name, node);
	}
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		nodes_material_sync_group_sockets(g.canvas, c.name, node);
	}
}

function nodes_material_sync_group_sockets(canvas: ui_node_canvas_t, group_name: string, node: ui_node_t) {
	for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
		let n: ui_node_t = canvas.nodes[i];
		if (n.type == "GROUP" && n.name == group_name) {
			let is_inputs: bool                 = node.name == "Group Input";
			let old_sockets: ui_node_socket_t[] = is_inputs ? n.inputs : n.outputs;
			let sockets: ui_node_socket_t[]     = util_clone_canvas_sockets(is_inputs ? node.outputs : node.inputs);
			if (is_inputs) {
				n.inputs = sockets;
			}
			else {
				n.outputs = sockets;
			}
			for (let i: i32 = 0; i < sockets.length; ++i) {
				let s: ui_node_socket_t = sockets[i];
				s.node_id               = n.id;
			}
			let num_sockets: i32 = sockets.length < old_sockets.length ? sockets.length : old_sockets.length;
			for (let i: i32 = 0; i < num_sockets; ++i) {
				if (sockets[i].type == old_sockets[i].type) {
					sockets[i].default_value = old_sockets[i].default_value;
				}
			}
		}
	}
}

function nodes_material_get_socket_color(type: string): i32 {
	return type == "RGBA" ? 0xffc7c729 : type == "VECTOR" ? 0xff6363c7 : 0xffa1a1a1;
}

function nodes_material_get_socket_default_value(type: string): f32_array_t {
	return type == "RGBA" ? f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0) : type == "VECTOR" ? f32_array_create_xyz(0.0, 0.0, 0.0) : f32_array_create_x(0.0);
}

function nodes_material_get_socket_name(type: string): string {
	return type == "RGBA" ? _tr("Color") : type == "VECTOR" ? _tr("Vector") : _tr("Value");
}

function nodes_material_create_socket(nodes: ui_nodes_t, node: ui_node_t, name: string, type: string, canvas: ui_node_canvas_t, min: f32 = 0.0, max: f32 = 1.0,
                                      default_value: any = null): ui_node_socket_t {
	let soc: ui_node_socket_t = {
		id : ui_get_socket_id(canvas.nodes),
		node_id : node.id,
		name : name == null ? nodes_material_get_socket_name(type) : name,
		type : type,
		color : nodes_material_get_socket_color(type),
		default_value : default_value == null ? nodes_material_get_socket_default_value(type) : default_value,
		min : min,
		max : max,
		precision : 100
	};
	return soc;
}

let group_node_def: ui_node_t = {
	id : 0,
	name : _tr("New Group"),
	type : "GROUP",
	x : 0,
	y : 0,
	color : 0xffb34f5a,
	inputs : [],
	outputs : [],
	buttons : [ {
		name : "nodes_material_new_group_button",
		type : "CUSTOM",
		output : -1,
		default_value : f32_array_create_x(0),
		data : null,
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 1
	} ],
	width : 0,
	flags : 0
};
