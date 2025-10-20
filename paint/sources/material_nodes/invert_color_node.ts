
function invert_color_node_init() {
	array_push(nodes_material_color, invert_color_node_def);
	map_set(parser_material_node_vectors, "INVERT_COLOR", invert_color_node_vector);
}

function invert_color_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let fac: string     = parser_material_parse_value_input(node.inputs[0]);
	let out_col: string = parser_material_parse_vector_input(node.inputs[1]);
	return "lerp3(" + out_col + ", float3(1.0, 1.0, 1.0) - (" + out_col + "), " + fac + ")";
}

let invert_color_node_def: ui_node_t = {
	id : 0,
	name : _tr("Invert Color"),
	type : "INVERT_COLOR",
	x : 0,
	y : 0,
	color : 0xff448c6d,
	inputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("Factor"),
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
			name : _tr("Color"),
			type : "RGBA",
			color : 0xffc7c729,
			default_value : f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	outputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Color"),
		type : "RGBA",
		color : 0xffc7c729,
		default_value : f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [],
	width : 0,
	flags : 0
};
