
function map_range_node_init() {
	array_push(nodes_material_utilities, map_range_node_def);
	map_set(parser_material_node_values, "MAPRANGE", map_range_node_value);
}

function map_range_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	let val: string  = parser_material_parse_value_input(node.inputs[0]);
	let fmin: string = parser_material_parse_value_input(node.inputs[1]);
	let fmax: string = parser_material_parse_value_input(node.inputs[2]);
	let tmin: string = parser_material_parse_value_input(node.inputs[3]);
	let tmax: string = parser_material_parse_value_input(node.inputs[4]);

	let use_clamp: bool = node.buttons[0].default_value[0] > 0;

	let a: string       = "((" + tmin + " - " + tmax + ") / (" + fmin + " - " + fmax + "))";
	let out_val: string = "(" + a + " * " + val + " + " + tmin + " - " + a + " * " + fmin + ")";
	if (use_clamp) {
		return "(clamp(" + out_val + ", " + tmin + ", " + tmax + "))";
	}
	else {
		return out_val;
	}
}

let map_range_node_def: ui_node_t = {
	id : 0,
	name : _tr("Map Range"),
	type : "MAPRANGE",
	x : 0,
	y : 0,
	color : 0xff62676d,
	inputs : [
		{
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
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("From Min"),
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
			name : _tr("From Max"),
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
			name : _tr("To Min"),
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
			name : _tr("To Max"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	outputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("Value"),
		type : "VALUE",
		color : 0xffa1a1a1,
		default_value : f32_array_create_x(0.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [ {
		name : _tr("Clamp"),
		type : "BOOL",
		output : 0,
		default_value : f32_array_create_x(0),
		data : null,
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 0
	} ],
	width : 0,
	flags : 0
};
