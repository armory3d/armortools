
function normal_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    if (socket == node.outputs[0]) {
        return parser_material_vec3(node.outputs[0].default_value);
    }
    else if (socket == node.outputs[1]) {
        let nor: string = parser_material_parse_vector_input(node.inputs[0]);
        let norout: string = parser_material_vec3(node.outputs[0].default_value);
        return parser_material_to_vec3("dot(" + norout + ", " + nor + ")");
    }
}

function normal_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let nor: string = parser_material_parse_vector_input(node.inputs[0]);
    let norout: string = parser_material_vec3(node.outputs[0].default_value);
    return "dot(" + norout + ", " + nor + ")";
}

let normal_node_def: ui_node_t = {
    id: 0,
    name: _tr("Normal"),
    type: "NORMAL",
    x: 0,
    y: 0,
    color: 0xff522c99,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Normal"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
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
            name: _tr("Normal"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Dot"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    buttons: [
        {
            name: _tr("Vector"),
            type: "VECTOR",
            output: 0,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
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
