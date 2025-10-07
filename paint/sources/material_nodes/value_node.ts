
function value_node_init() {
    array_push(nodes_material_input, value_node_def);
    map_set(parser_material_node_values, "VALUE", value_node_value);
}

function value_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    return parser_material_vec1(node.outputs[0].default_value[0]);
}

let value_node_def: ui_node_t = {
    id: 0,
    name: _tr("Value"),
    type: "VALUE",
    x: 0,
    y: 0,
    color: 0xffb34f5a,
    inputs: [],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Value"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.5),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    buttons: [
        {
            name: _tr("default_value"),
            type: "VALUE",
            output: 0,
            default_value: f32_array_create_x(0),
            data: null,
            min: 0.0,
            max: 10.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
