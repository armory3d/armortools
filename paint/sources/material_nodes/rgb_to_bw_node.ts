
function rgb_to_bw_node_init() {
    array_push(nodes_material_converter, rgb_to_bw_node_def);
    map_set(parser_material_node_values, "RGBTOBW", rgb_to_bw_node_value);
}

function rgb_to_bw_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let col: string = parser_material_parse_vector_input(node.inputs[0]);
    return "(((" + col + ".r * 0.3 + " + col + ".g * 0.59 + " + col + ".b * 0.11) / 3.0) * 2.5)";
}

let rgb_to_bw_node_def: ui_node_t = {
    id: 0,
    name: _tr("RGB to BW"),
    type: "RGBTOBW",
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
            default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 0.0),
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
            name: _tr("Val"),
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
