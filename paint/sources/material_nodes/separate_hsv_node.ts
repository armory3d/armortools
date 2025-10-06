
function separate_hsv_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    node_shader_add_function(parser_material_kong, str_hue_sat);
    let col: string = parser_material_parse_vector_input(node.inputs[0]);
    if (socket == node.outputs[0]) {
        return "rgb_to_hsv(" + col + ").r";
    }
    else if (socket == node.outputs[1]) {
        return "rgb_to_hsv(" + col + ").g";
    }
    else if (socket == node.outputs[2]) {
        return "rgb_to_hsv(" + col + ").b";
    }
}

let separate_hsv_node_def: ui_node_t = {
    id: 0,
    name: _tr("Separate HSV"),
    type: "SEPHSV",
    x: 0,
    y: 0,
    color: 0xff62676d,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Color"),
            type: "RGBA",
            color: 0xffc7c729,
            default_value: f32_array_create_xyzw(0.5, 0.5, 0.5, 1.0),
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
            name: _tr("H"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("S"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("V"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.0),
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
