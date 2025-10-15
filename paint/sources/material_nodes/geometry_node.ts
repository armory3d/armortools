
function geometry_node_init() {
	array_push(nodes_material_input, geometry_node_def);
	map_set(parser_material_node_vectors, "NEW_GEOMETRY", geometry_node_vector);
	map_set(parser_material_node_values, "NEW_GEOMETRY", geometry_node_value);
}

function geometry_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	if (socket == node.outputs[0]) { // Position
		parser_material_kong.frag_wposition = true;
		return "input.wposition";
	}
	else if (socket == node.outputs[1]) { // Normal
		parser_material_kong.frag_n = true;
		return "n";
	}
	else if (socket == node.outputs[2]) { // Tangent
		parser_material_kong.frag_wtangent = true;
		return "input.wtangent";
	}
	else if (socket == node.outputs[3]) { // True Normal
		parser_material_kong.frag_n = true;
		return "n";
	}
	else if (socket == node.outputs[4]) { // Incoming
		parser_material_kong.frag_vvec = true;
		return "vvec";
	}
	else { // Parametric
		parser_material_kong.frag_mposition = true;
		return "input.mposition";
	}
}

function geometry_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	if (socket == node.outputs[6]) { // Backfacing
		return "0.0";                // SV_IsFrontFace
		// return "(1.0 - float(gl_FrontFacing))";
	}
	else if (socket == node.outputs[7]) { // Pointiness
		let strength: f32           = 1.0;
		let radius: f32             = 1.0;
		let offset: f32             = 0.0;
		let store: string           = parser_material_store_var_name(node);
		parser_material_kong.frag_n = true;
		parser_material_write(parser_material_kong, "var " + store + "_dx: float3 = ddx3(n);");
		parser_material_write(parser_material_kong, "var " + store + "_dy: float3 = ddy3(n);");
		parser_material_write(parser_material_kong,
		                      "var " + store + "_curvature: float = max(dot(" + store + "_dx, " + store + "_dx), dot(" + store + "_dy, " + store + "_dy));");
		parser_material_write(parser_material_kong, store + "_curvature = clamp(pow(" + store + "_curvature, (1.0 / " + radius + ") * 0.25) * " + strength +
		                                                " * 2.0 + " + offset + " / 10.0, 0.0, 1.0);");
		return store + "_curvature";
	}
	else { // Random Per Island
		return "0.0";
	}
}

let geometry_node_def: ui_node_t = {
	id : 0,
	name : _tr("Geometry"),
	type : "NEW_GEOMETRY",
	x : 0,
	y : 0,
	color : 0xffb34f5a,
	inputs : [],
	outputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("Position"),
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
			name : _tr("Normal"),
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
			name : _tr("Tangent"),
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
			name : _tr("True Normal"),
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
			name : _tr("Incoming"),
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
			name : _tr("Parametric"),
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
			name : _tr("Backfacing"),
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
			name : _tr("Pointiness"),
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
			name : _tr("Random Per Island"),
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
