
function gamma_node_init() {
    array_push(nodes_material_color, gamma_node_def);
    map_set(parser_material_node_vectors, "GAMMA", gamma_node_vector);
}

function gamma_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    let out_col: string = parser_material_parse_vector_input(node.inputs[0]);
    let gamma: string = parser_material_parse_value_input(node.inputs[1]);
    return "pow3(" + out_col + ", " + parser_material_to_vec3(gamma) + ")";
}

let gamma_node_def: ui_node_t = {
    id: 0,
    name: _tr("Gamma"),
    type: "GAMMA",
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
            name: _tr("Gamma"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
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
