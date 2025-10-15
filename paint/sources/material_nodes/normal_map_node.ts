
function normal_map_node_init() {
	array_push(nodes_material_vector, normal_map_node_def);
	map_set(parser_material_node_vectors, "NORMAL_MAP", normal_map_node_vector);
}

function normal_map_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let strength: string = parser_material_parse_value_input(node.inputs[0]);
	let norm: string     = parser_material_parse_vector_input(node.inputs[1]);

	let store: string = parser_material_store_var_name(node);
	parser_material_write(parser_material_kong, "var " + store + "_texn: float3 = " + norm + " * 2.0 - 1.0;");
	parser_material_write(parser_material_kong, "" + store + "_texn.xy = " + strength + " * " + store + "_texn.xy;");
	parser_material_write(parser_material_kong, "" + store + "_texn = normalize(" + store + "_texn);");

	return "(0.5 * " + store + "_texn + 0.5)";
}

let normal_map_node_def: ui_node_t = {
	id : 0,
	name : _tr("Normal Map"),
	type : "NORMAL_MAP",
	x : 0,
	y : 0,
	color : 0xff522c99,
	inputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("Strength"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 2.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Normal Map"),
			type : "VECTOR",
			color : -10238109,
			default_value : f32_array_create_xyz(0.5, 0.5, 1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	outputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Normal Map"),
		type : "VECTOR",
		color : -10238109,
		default_value : f32_array_create_xyz(0.5, 0.5, 1.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [],
	width : 0,
	flags : 0
};
