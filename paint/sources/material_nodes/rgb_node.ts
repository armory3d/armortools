
function rgb_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    return parser_material_vec3(socket.default_value);
}

let rgb_node_def: ui_node_t = {
    id: 0,
    name: _tr("RGB"),
    type: "RGB",
    x: 0,
    y: 0,
    color: 0xffb34f5a,
    inputs: [],
    outputs: [
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
    buttons: [
        {
            name: _tr("default_value"),
            type: "RGBA",
            output: 0,
            default_value: f32_array_create_x(0),
            data: null,
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
