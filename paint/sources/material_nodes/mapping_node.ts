
function mapping_node_init() {
	array_push(nodes_material_vector, mapping_node_def);
	map_set(parser_material_node_vectors, "MAPPING", mapping_node_vector);
}

function mapping_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let out: string              = parser_material_parse_vector_input(node.inputs[0]);
	let node_translation: string = parser_material_parse_vector_input(node.inputs[1]);
	let node_rotation: string    = parser_material_parse_vector_input(node.inputs[2]);
	let node_scale: string       = parser_material_parse_vector_input(node.inputs[3]);
	if (node_scale != "float3(1, 1, 1)") {
		out = "(" + out + " * " + node_scale + ")";
	}
	if (node_rotation != "float3(0, 0, 0)") {
		// ZYX rotation, Z axis for now..
		let a: string = node_rotation + ".z * (3.1415926535 / 180)";
		// x * cos(theta) - y * sin(theta)
		// x * sin(theta) + y * cos(theta)
		out = "float3(" + out + ".x * cos(" + a + ") - " + out + ".y * sin(" + a + "), " + out + ".x * sin(" + a + ") + " + out + ".y * cos(" + a + "), 0.0)";
	}
	// if node.rotation[1] != 0.0:
	//     a = node.rotation[1]
	//     out = "float3({0}.x * {1} - {0}.z * {2}, {0}.x * {2} + {0}.z * {1}, 0.0)".format(out, math_cos(a), math_sin(a))
	// if node.rotation[0] != 0.0:
	//     a = node.rotation[0]
	//     out = "float3({0}.y * {1} - {0}.z * {2}, {0}.y * {2} + {0}.z * {1}, 0.0)".format(out, math_cos(a), math_sin(a))
	if (node_translation != "float3(0, 0, 0)") {
		out = "(" + out + " + " + node_translation + ")";
	}
	// if node.use_min:
	// out = "max({0}, float3({1}, {2}, {3}))".format(out, node.min[0], node.min[1])
	// if node.use_max:
	// out = "min({0}, float3({1}, {2}, {3}))".format(out, node.max[0], node.max[1])
	return out;
}

let mapping_node_def: ui_node_t = {
	id : 0,
	name : _tr("Mapping"),
	type : "MAPPING",
	x : 0,
	y : 0,
	color : 0xff522c99,
	inputs : [
		{
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
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Location"),
			type : "VECTOR",
			color : 0xff6363c7,
			default_value : f32_array_create_xyz(0.0, 0.0, 0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 1
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Rotation"),
			type : "VECTOR",
			color : 0xff6363c7,
			default_value : f32_array_create_xyz(0.0, 0.0, 0.0),
			min : 0.0,
			max : 360.0,
			precision : 100,
			display : 1
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Scale"),
			type : "VECTOR",
			color : 0xff6363c7,
			default_value : f32_array_create_xyz(1.0, 1.0, 1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 1
		}
	],
	outputs : [ {
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
	buttons : [],
	width : 0,
	flags : 0
};
