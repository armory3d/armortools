
function mix_color_node_init() {
	array_push(nodes_material_color, mix_color_node_def);
	map_set(parser_material_node_vectors, "MIX_RGB", mix_color_node_vector);
}

function mix_color_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let fac: string     = parser_material_parse_value_input(node.inputs[0]);
	let fac_var: string = parser_material_node_name(node) + "_fac";
	parser_material_write(parser_material_kong, "var " + fac_var + ": float = " + fac + ";");
	let col1: string          = parser_material_parse_vector_input(node.inputs[1]);
	let col2: string          = parser_material_parse_vector_input(node.inputs[2]);
	let but: ui_node_button_t = node.buttons[0]; // blend_type
	let blend: string         = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
	blend                     = string_replace_all(blend, " ", "_");
	let use_clamp: bool       = node.buttons[1].default_value[0] > 0;
	let out_col: string       = "";
	if (blend == "MIX") {
		out_col = "lerp3(" + col1 + ", " + col2 + ", " + fac_var + ")";
	}
	else if (blend == "DARKEN") {
		out_col = "min3(" + col1 + ", " + col2 + " * " + fac_var + ")";
	}
	else if (blend == "MULTIPLY") {
		out_col = "lerp3(" + col1 + ", " + col1 + " * " + col2 + ", " + fac_var + ")";
	}
	else if (blend == "BURN") {
		out_col = "lerp3(" + col1 + ", float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - " + col1 + ") / " + col2 + ", " + fac_var + ")";
	}
	else if (blend == "LIGHTEN") {
		out_col = "max3(" + col1 + ", " + col2 + " * " + fac_var + ")";
	}
	else if (blend == "SCREEN") {
		let v3: string = parser_material_to_vec3("1.0 - " + fac_var);
		out_col = "(float3(1.0, 1.0, 1.0) - (" + v3 + " + " + fac_var + " * (float3(1.0, 1.0, 1.0) - " + col2 + ")) * (float3(1.0, 1.0, 1.0) - " + col1 + "))";
	}
	else if (blend == "DODGE") {
		out_col = "lerp3(" + col1 + ", " + col1 + " / (float3(1.0, 1.0, 1.0) - " + col2 + "), " + fac_var + ")";
	}
	else if (blend == "ADD") {
		out_col = "lerp3(" + col1 + ", " + col1 + " + " + col2 + ", " + fac_var + ")";
	}
	else if (blend == "OVERLAY") {
		// out_col = "lerp3(" + col1 + ", float3( \
		// 	" + col1 + ".r < 0.5 ? 2.0 * " + col1 + ".r * " + col2 + ".r : 1.0 - 2.0 * (1.0 - " + col1 + ".r) * (1.0 - " + col2 + ".r), \
		// 	" + col1 + ".g < 0.5 ? 2.0 * " + col1 + ".g * " + col2 + ".g : 1.0 - 2.0 * (1.0 - " + col1 + ".g) * (1.0 - " + col2 + ".g), \
		// 	" + col1 + ".b < 0.5 ? 2.0 * " + col1 + ".b * " + col2 + ".b : 1.0 - 2.0 * (1.0 - " + col1 + ".b) * (1.0 - " + col2 + ".b) \
		// ), " + fac_var + ")";
		let res_r: string = parser_material_node_name(node) + "_res_r";
		let res_g: string = parser_material_node_name(node) + "_res_g";
		let res_b: string = parser_material_node_name(node) + "_res_b";
		parser_material_write(parser_material_kong, "var " + res_r + ": float;");
		parser_material_write(parser_material_kong, "var " + res_g + ": float;");
		parser_material_write(parser_material_kong, "var " + res_b + ": float;");
		parser_material_write(parser_material_kong, "if (" + col1 + ".r < 0.5) { " + res_r + " = 2.0 * " + col1 + ".r * " + col2 + ".r; } else { " + res_r +
		                                                " = 1.0 - 2.0 * (1.0 - " + col1 + ".r) * (1.0 - " + col2 + ".r); }");
		parser_material_write(parser_material_kong, "if (" + col1 + ".g < 0.5) { " + res_g + " = 2.0 * " + col1 + ".g * " + col2 + ".g; } else { " + res_g +
		                                                " = 1.0 - 2.0 * (1.0 - " + col1 + ".g) * (1.0 - " + col2 + ".g); }");
		parser_material_write(parser_material_kong, "if (" + col1 + ".b < 0.5) { " + res_b + " = 2.0 * " + col1 + ".b * " + col2 + ".b; } else { " + res_b +
		                                                " = 1.0 - 2.0 * (1.0 - " + col1 + ".b) * (1.0 - " + col2 + ".b); }");
		out_col = "lerp3(" + col1 + ", float3(" + res_r + ", " + res_g + ", " + res_b + "), " + fac_var + ")";
	}
	else if (blend == "SOFT_LIGHT") {
		out_col = "((1.0 - " + fac_var + ") * " + col1 + " + " + fac_var + " * ((float3(1.0, 1.0, 1.0) - " + col1 + ") * " + col2 + " * " + col1 + " + " +
		          col1 + " * (float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - " + col2 + ") * (float3(1.0, 1.0, 1.0) - " + col1 + "))))";
	}
	else if (blend == "LINEAR_LIGHT") {
		out_col = "(" + col1 + " + " + fac_var + " * (float3(2.0, 2.0, 2.0) * (" + col2 + " - float3(0.5, 0.5, 0.5))))";
	}
	else if (blend == "DIFFERENCE") {
		out_col = "lerp3(" + col1 + ", abs3(" + col1 + " - " + col2 + "), " + fac_var + ")";
	}
	else if (blend == "SUBTRACT") {
		out_col = "lerp3(" + col1 + ", " + col1 + " - " + col2 + ", " + fac_var + ")";
	}
	else if (blend == "DIVIDE") {
		let eps: f32   = 0.000001;
		col2           = "max3(" + col2 + ", float3(" + eps + ", " + eps + ", " + eps + "))";
		let v3: string = "(float3(1.0, 1.0, 1.0) - float3(" + fac_var + ", " + fac_var + ", " + fac_var + ")) * " + col1 + " + float3(" + fac_var + ", " +
		                 fac_var + ", " + fac_var + ") * " + col1 + " / " + col2;
		out_col = "(" + v3 + ")";
	}
	else if (blend == "HUE") {
		node_shader_add_function(parser_material_kong, str_hue_sat);
		out_col =
		    "lerp3(" + col1 + ", hsv_to_rgb(float3(rgb_to_hsv(" + col2 + ").r, rgb_to_hsv(" + col1 + ").g, rgb_to_hsv(" + col1 + ").b)), " + fac_var + ")";
	}
	else if (blend == "SATURATION") {
		node_shader_add_function(parser_material_kong, str_hue_sat);
		out_col =
		    "lerp3(" + col1 + ", hsv_to_rgb(float3(rgb_to_hsv(" + col1 + ").r, rgb_to_hsv(" + col2 + ").g, rgb_to_hsv(" + col1 + ").b)), " + fac_var + ")";
	}
	else if (blend == "COLOR") {
		node_shader_add_function(parser_material_kong, str_hue_sat);
		out_col =
		    "lerp3(" + col1 + ", hsv_to_rgb(float3(rgb_to_hsv(" + col2 + ").r, rgb_to_hsv(" + col2 + ").g, rgb_to_hsv(" + col1 + ").b)), " + fac_var + ")";
	}
	else if (blend == "VALUE") {
		node_shader_add_function(parser_material_kong, str_hue_sat);
		out_col =
		    "lerp3(" + col1 + ", hsv_to_rgb(float3(rgb_to_hsv(" + col1 + ").r, rgb_to_hsv(" + col1 + ").g, rgb_to_hsv(" + col2 + ").b)), " + fac_var + ")";
	}
	if (use_clamp) {
		return "clamp3(" + out_col + ", float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0))";
	}
	else {
		return out_col;
	}
}

let mix_color_node_def: ui_node_t = {
	id : 0,
	name : _tr("Mix Color"),
	type : "MIX_RGB",
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
			default_value : f32_array_create_x(0.5),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Color 1"),
			type : "RGBA",
			color : 0xffc7c729,
			default_value : f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Color 2"),
			type : "RGBA",
			color : 0xffc7c729,
			default_value : f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
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
	buttons : [
		{
			name : _tr("blend_type"),
			type : "ENUM",
			output : 0,
			default_value : f32_array_create_x(0),
			data : u8_array_create_from_string(_tr("Mix") + "\n" + _tr("Darken") + "\n" + _tr("Multiply") + "\n" + _tr("Burn") + "\n" + _tr("Lighten") + "\n" +
			                                   _tr("Screen") + "\n" + _tr("Dodge") + "\n" + _tr("Add") + "\n" + _tr("Overlay") + "\n" + _tr("Soft Light") +
			                                   "\n" + _tr("Linear Light") + "\n" + _tr("Difference") + "\n" + _tr("Subtract") + "\n" + _tr("Divide") + "\n" +
			                                   _tr("Hue") + "\n" + _tr("Saturation") + "\n" + _tr("Color") + "\n" + _tr("Value")),
			min : 0.0,
			max : 1.0,
			precision : 100,
			height : 0
		},
		{
			name : _tr("Clamp"),
			type : "BOOL",
			output : 0,
			default_value : f32_array_create_x(0),
			data : null,
			min : 0.0,
			max : 1.0,
			precision : 100,
			height : 0
		}
	],
	width : 0,
	flags : 0
};
