
function script_node_init() {
	array_push(nodes_material_input, script_node_def);
	map_set(parser_material_node_values, "SCRIPT_CPU", script_node_value);
}

function script_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	if (parser_material_script_links == null) {
		parser_material_script_links = map_create();
	}
	let script: buffer_t = node.buttons[0].default_value;
	let str: string      = sys_buffer_to_string(script);
	let link: string     = parser_material_node_name(node);
	map_set(parser_material_script_links, link, str);
	node_shader_add_constant(parser_material_kong, "" + link + ": float", "_" + link);
	return "constants." + link;
}

let script_node_def: ui_node_t = {
	id : 0,
	name : _tr("Script"),
	type : "SCRIPT_CPU", // extension
	x : 0,
	y : 0,
	color : 0xffb34f5a,
	inputs : [],
	outputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Value"),
		type : "VALUE",
		color : 0xffa1a1a1,
		default_value : f32_array_create_x(0.5),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [ {
		name : " ",
		type : "STRING",
		output : -1,
		default_value : f32_array_create_x(0), // "",
		data : null,
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 0
	} ],
	width : 0,
	flags : 0
};
