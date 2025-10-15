
function separate_xyz_node_init() {
	array_push(nodes_material_converter, separate_xyz_node_def);
	map_set(parser_material_node_values, "SEPXYZ", separate_xyz_node_value);
}

function separate_xyz_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	let vec: string = parser_material_parse_vector_input(node.inputs[0]);
	if (socket == node.outputs[0]) {
		return vec + ".x";
	}
	else if (socket == node.outputs[1]) {
		return vec + ".y";
	}
	else {
		return vec + ".z";
	}
}

let separate_xyz_node_def: ui_node_t = {
	id : 0,
	name : _tr("Separate XYZ"),
	type : "SEPXYZ",
	x : 0,
	y : 0,
	color : 0xff62676d,
	inputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Vector"),
		type : "VECTOR",
		color : 0xff6363c7,
		default_value : f32_array_create_xyz(0.0, 0.0, 0.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	outputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("X"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Y"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Z"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	buttons : [],
	width : 0,
	flags : 0
};
