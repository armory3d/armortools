
function bump_node_init() {
	array_push(nodes_material_vector, bump_node_def);
	map_set(parser_material_node_vectors, "BUMP", bump_node_vector);
}

function bump_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let strength: string = parser_material_parse_value_input(node.inputs[0]);
	// let distance: string = parse_value_input(node.inputs[1]);
	let height: string          = parser_material_parse_value_input(node.inputs[2]);
	let nor: string             = parser_material_parse_vector_input(node.inputs[3]);
	let sample_bump_res: string = parser_material_store_var_name(node) + "_bump";
	parser_material_write(parser_material_kong, "var " + sample_bump_res + "_x: float = ddx(float(" + height + ")) * (" + strength + ") * 16.0;");
	parser_material_write(parser_material_kong, "var " + sample_bump_res + "_y: float = ddy(float(" + height + ")) * (" + strength + ") * 16.0;");
	return "(normalize(float3(" + sample_bump_res + "_x, " + sample_bump_res + "_y, 1.0) + " + nor + ") * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5))";
}

let bump_node_def: ui_node_t = {
	id : 0,
	name : _tr("Bump"),
	type : "BUMP",
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
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Distance"),
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
			name : _tr("Height"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Normal"),
			type : "VECTOR",
			color : 0xff6363c7,
			default_value : f32_array_create_xyz(0.0, 0.0, 0.0),
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
