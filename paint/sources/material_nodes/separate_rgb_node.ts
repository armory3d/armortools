
function separate_rgb_node_init() {
    array_push(nodes_material_converter, separate_rgb_node_def);
    map_set(parser_material_node_values, "SEPRGB", separate_rgb_node_value);
}

function separate_rgb_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let col: string = parser_material_parse_vector_input(node.inputs[0]);
    if (socket == node.outputs[0]) {
        return col + ".r";
    }
    else if (socket == node.outputs[1]) {
        return col + ".g";
    }
    else {
        return col + ".b";
    }
}

let separate_rgb_node_def: ui_node_t = {
    id: 0,
    name: _tr("Separate RGB"),
    type: "SEPRGB",
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
            default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
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
            name: _tr("R"),
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
            name: _tr("G"),
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
            name: _tr("B"),
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
