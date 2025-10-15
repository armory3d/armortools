
function combine_rgb_node_init() {
	array_push(nodes_material_converter, combine_rgb_node_def);
	map_set(parser_material_node_vectors, "COMBRGB", combine_rgb_node_vector);
}

function combine_rgb_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let r: string = parser_material_parse_value_input(node.inputs[0]);
	let g: string = parser_material_parse_value_input(node.inputs[1]);
	let b: string = parser_material_parse_value_input(node.inputs[2]);
	return "float3(" + r + ", " + g + ", " + b + ")";
}

let combine_rgb_node_def: ui_node_t = {
	id : 0,
	name : _tr("Combine RGB"),
	type : "COMBRGB",
	x : 0,
	y : 0,
	color : 0xff62676d,
	inputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("R"),
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
			name : _tr("G"),
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
			name : _tr("B"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.0),
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
