
function warp_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    if (parser_material_warp_passthrough) {
        return parser_material_parse_vector_input(node.inputs[0]);
    }
    let angle: string = parser_material_parse_value_input(node.inputs[1], true);
    let mask: string = parser_material_parse_value_input(node.inputs[2], true);
    let tex_name: string = "texwarp_" + parser_material_node_name(node);
    node_shader_add_texture(parser_material_kong, "" + tex_name, "_" + tex_name);
    let store: string = parser_material_store_var_name(node);
    let pi: f32 = math_pi();
    parser_material_write(parser_material_kong, "var " + store + "_rad: float = " + angle + " * (" + pi + " / 180.0);");
    parser_material_write(parser_material_kong, "var " + store + "_x: float = cos(" + store + "_rad);");
    parser_material_write(parser_material_kong, "var " + store + "_y: float = sin(" + store + "_rad);");
    return "sample(" + tex_name + ", sampler_linear, tex_coord + float2(" + store + "_x, " + store + "_y) * " + mask + ").rgb";
}

let warp_node_def: ui_node_t = {
    id: 0,
    name: _tr("Warp"),
    type: "DIRECT_WARP", // extension
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
            name: _tr("Angle"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.0),
            min: 0.0,
            max: 360.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Mask"),
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
