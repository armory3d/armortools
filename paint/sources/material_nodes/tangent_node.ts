
function tangent_node_init() {
	array_push(nodes_material_input, tangent_node_def);
	map_set(parser_material_node_vectors, "TANGENT", tangent_node_vector);
}

function tangent_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	parser_material_kong.frag_wtangent = true;
	return "input.wtangent";
}

let tangent_node_def: ui_node_t = {
	id : 0,
	name : _tr("Tangent"),
	type : "TANGENT",
	x : 0,
	y : 0,
	color : 0xffb34f5a,
	inputs : [],
	outputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Tangent"),
		type : "VECTOR",
		color : 0xff6363c7,
		default_value : f32_array_create_xyz(0.0, 0.0, 0.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [],
	width : 0,
	flags : 0
};
