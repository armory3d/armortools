
function fresnel_node_init() {
	array_push(nodes_material_input, fresnel_node_def);
	map_set(parser_material_node_values, "FRESNEL", fresnel_node_value);
}

function fresnel_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	let ior: string                 = parser_material_parse_value_input(node.inputs[0]);
	parser_material_kong.frag_dotnv = true;
	return "pow(1.0 - dotnv, 7.25 / " + ior + ")";
}

let fresnel_node_def: ui_node_t = {
	id : 0,
	name : _tr("Fresnel"),
	type : "FRESNEL",
	x : 0,
	y : 0,
	color : 0xffb34f5a,
	inputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("IOR"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.45),
			min : 0.0,
			max : 3.0,
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
		name : _tr("Fac"),
		type : "VALUE",
		color : 0xffa1a1a1,
		default_value : f32_array_create_x(0.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [],
	width : 0,
	flags : 0
};
