
function replace_color_node_init() {
	array_push(nodes_material_color, replace_color_node_def);
	map_set(parser_material_node_vectors, "REPLACECOL", replace_color_node_vector);
}

function replace_color_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let input_color: string = parser_material_parse_vector_input(node.inputs[0]);
	let old_color: string   = parser_material_parse_vector_input(node.inputs[1]);
	let new_color: string   = parser_material_parse_vector_input(node.inputs[2]);
	let radius: string      = parser_material_parse_value_input(node.inputs[3]);
	let fuzziness: string   = parser_material_parse_value_input(node.inputs[4]);
	return "lerp3(" + new_color + ", " + input_color + ", clamp((distance(" + old_color + ", " + input_color + ") - " + radius + ") / max(" + fuzziness + ", " +
	       parser_material_eps + "), 0.0, 1.0))";
}

let replace_color_node_def: ui_node_t = {
	id : 0,
	name : _tr("Replace Color"),
	type : "REPLACECOL", // extension
	x : 0,
	y : 0,
	color : 0xff448c6d,
	inputs : [
		{id : 0, node_id : 0, name : _tr("Color"), type : "RGBA", color : 0xffc7c729, default_value : f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)},
		{id : 0, node_id : 0, name : _tr("Old Color"), type : "RGBA", color : 0xffc7c729, default_value : f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)},
		{id : 0, node_id : 0, name : _tr("New Color"), type : "RGBA", color : 0xffc7c729, default_value : f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)}, {
			id : 0,
			node_id : 0,
			name : _tr("Radius"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.1),
			min : 0.0,
			max : 1.74,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Fuzziness"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	outputs : [ {id : 0, node_id : 0, name : _tr("Color"), type : "RGBA", color : 0xffc7c729, default_value : f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0)} ],
	buttons : []
};
