
function gradient_texture_node_init() {
	array_push(nodes_material_texture, gradient_texture_node_def);
	map_set(parser_material_node_vectors, "TEX_GRADIENT", gradient_texture_node_vector);
	map_set(parser_material_node_values, "TEX_GRADIENT", gradient_texture_node_value);
}

function parser_material_get_gradient(grad: string, co: string): string {
	if (grad == "LINEAR") {
		return co + ".x";
	}
	else if (grad == "QUADRATIC") {
		return "0.0";
	}
	else if (grad == "EASING") {
		return "0.0";
	}
	else if (grad == "DIAGONAL") {
		return "(" + co + ".x + " + co + ".y) * 0.5";
	}
	else if (grad == "RADIAL") {
		return "atan2(" + co + ".x, " + co + ".y) / (3.141592 * 2.0) + 0.5";
	}
	else if (grad == "QUADRATIC_SPHERE") {
		return "0.0";
	}
	else { // "SPHERICAL"
		return "max(1.0 - sqrt(" + co + ".x * " + co + ".x + " + co + ".y * " + co + ".y + " + co + ".z * " + co + ".z), 0.0)";
	}
}

function gradient_texture_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let co: string            = parser_material_get_coord(node);
	let but: ui_node_button_t = node.buttons[0]; // gradient_type;
	let grad: string          = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
	grad                      = string_replace_all(grad, " ", "_");
	let f: string             = parser_material_get_gradient(grad, co);
	let res: string           = parser_material_to_vec3("clamp(" + f + ", 0.0, 1.0)");
	return res;
}

function gradient_texture_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	let co: string            = parser_material_get_coord(node);
	let but: ui_node_button_t = node.buttons[0]; // gradient_type;
	let grad: string          = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
	grad                      = string_replace_all(grad, " ", "_");
	let f: string             = parser_material_get_gradient(grad, co);
	let res: string           = "(clamp(" + f + ", 0.0, 1.0))";
	return res;
}

let gradient_texture_node_def: ui_node_t = {
	id : 0,
	name : _tr("Gradient Texture"),
	type : "TEX_GRADIENT",
	x : 0,
	y : 0,
	color : 0xff4982a0,
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
			name : _tr("Color"),
			type : "RGBA",
			color : 0xffc7c729,
			default_value : f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Fac"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	buttons : [ {
		name : _tr("gradient_type"),
		type : "ENUM",
		output : 0,
		default_value : f32_array_create_x(0),
		data : u8_array_create_from_string(_tr("Linear") + "\n" + _tr("Diagonal") + "\n" + _tr("Radial") + "\n" + _tr("Spherical")),
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 0
	} ],
	width : 0,
	flags : 0
};
