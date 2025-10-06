
function mix_normal_map_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    let nm1: string = parser_material_parse_vector_input(node.inputs[0]);
    let nm2: string = parser_material_parse_vector_input(node.inputs[1]);
    let but: ui_node_button_t = node.buttons[0];
    let blend: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0])); // blend_type
    blend = string_replace_all(blend, " ", "_");
    let store: string = parser_material_store_var_name(node);

    // The blending algorithms are based on the paper "Blending in Detail" by Colin Barré-Brisebois and Stephen Hill 2012
    // https://blog.selfshadow.com/publications/blending-in-detail/
    if (blend == "PARTIAL_DERIVATIVE") { //partial derivate blending
        parser_material_write(parser_material_kong, "var " + store + "_n1: float3 = " + nm1 + " * 2.0 - 1.0;");
        parser_material_write(parser_material_kong, "var " + store + "_n2: float3 = " + nm2 + " * 2.0 - 1.0;");
        return "0.5 * normalize(float3(" + store + "_n1.xy * " + store + "_n2.z + " + store + "_n2.xy * " + store + "_n1.z, " + store + "_n1.z * " + store + "_n2.z)) + 0.5";
    }
    else if (blend == "WHITEOUT") { //whiteout blending
        parser_material_write(parser_material_kong, "var " + store + "_n1: float3 = " + nm1 + " * 2.0 - 1.0;");
        parser_material_write(parser_material_kong, "var " + store + "_n2: float3 = " + nm2 + " * 2.0 - 1.0;");
        return "0.5 * normalize(float3(" + store + "_n1.xy + " + store + "_n2.xy, " + store + "_n1.z * " + store + "_n2.z)) + 0.5";
    }
    else if (blend == "REORIENTED") { //reoriented normal mapping
        parser_material_write(parser_material_kong, "var " + store + "_n1: float3 = " + nm1 + " * 2.0 - float3(1.0, 1.0, 0.0);");
        parser_material_write(parser_material_kong, "var " + store + "_n2: float3 = " + nm2 + " * float3(-2.0, -2.0, 2.0) - float3(-1.0, -1.0, 1.0);");
        return "0.5 * normalize(" + store + "_n1 * dot(" + store + "_n1, " + store + "_n2) - " + store + "_n2 * " + store + "_n1.z) + 0.5";
    }
}

let mix_normal_map_node_def: ui_node_t = {
    id: 0,
    name: _tr("Mix Normal Map"),
    type: "MIX_NORMAL_MAP", // extension
    x: 0,
    y: 0,
    color: 0xff522c99,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Normal Map 1"),
            type: "VECTOR",
            color: -10238109,
            default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Normal Map 2"),
            type: "VECTOR",
            color: -10238109,
            default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
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
            name: _tr("Normal Map"),
            type: "VECTOR",
            color: -10238109,
            default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    buttons: [
        {
            name: _tr("blend_type"),
            type: "ENUM",
            output: 0,
            default_value: f32_array_create_x(0),
            data: u8_array_create_from_string(_tr("Partial Derivative") + "\n" + _tr("Whiteout") + "\n" + _tr("Reoriented")),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
