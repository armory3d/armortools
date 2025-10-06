
function blur_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    if (parser_material_blur_passthrough) {
			return parser_material_parse_vector_input(node.inputs[0]);
		}
		let strength: string = parser_material_parse_value_input(node.inputs[1]);
		if (strength == "0.0") {
			return "float3(0.0, 0.0, 0.0)";
		}
		let steps: string = "(" + strength + " * 10.0 + 1.0)";
		let tex_name: string = "texblur_" + parser_material_node_name(node);
		node_shader_add_texture(parser_material_kong, "" + tex_name, "_" + tex_name);
		node_shader_add_constant(parser_material_kong, "" + tex_name + "_size: float2", "_size(_" + tex_name + ")");
		let store: string = parser_material_store_var_name(node);
		parser_material_write(parser_material_kong, "var " + store + "_res: float3 = float3(0.0, 0.0, 0.0);");
		parser_material_write(parser_material_kong, "for (var i: int = 0; i <= int(" + steps + " * 2.0); i += 1) {");
		parser_material_write(parser_material_kong, "for (var j: int = 0; j <= int(" + steps + " * 2.0); j += 1) {");
		parser_material_write(parser_material_kong, store + "_res += sample(" + tex_name + ", sampler_linear, tex_coord + float2(float(i) - " + steps + ", float(j) - " + steps + ") / constants." + tex_name + "_size).rgb;");
		parser_material_write(parser_material_kong, "}");
		parser_material_write(parser_material_kong, "}");
		parser_material_write(parser_material_kong, store + "_res = " + store + "_res / (" + steps + " * 2.0 + 1.0) * (" + steps + " * 2.0 + 1.0);");
		return store + "_res";
}

let blur_node_def: ui_node_t = {
    id: 0,
    name: _tr("Blur"),
    type: "BLUR", // extension
    x: 0,
    y: 0,
    color: 0xff448c6d,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Color"),
            type: "RGBA",
            color: 0xffc7c729,
            default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Strength"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.5),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Color"),
            type: "RGBA",
            color: 0xffc7c729,
            default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    buttons: [],
    width: 0,
    flags: 0
};
