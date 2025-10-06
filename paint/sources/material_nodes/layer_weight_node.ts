
function layer_weight_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let blend: string = parser_material_parse_value_input(node.inputs[0]);
    if (socket == node.outputs[0]) { // Fresnel
        parser_material_kong.frag_dotnv = true;
        return "clamp(pow(1.0 - dotnv, (1.0 - " + blend + ") * 10.0), 0.0, 1.0)";
    }
    else if (socket == node.outputs[1]) { // Facing
        parser_material_kong.frag_dotnv = true;
        return "((1.0 - dotnv) * " + blend + ")";
    }
}

let layer_weight_node_def: ui_node_t = {
    id: 0,
    name: _tr("Layer Weight"),
    type: "LAYER_WEIGHT",
    x: 0,
    y: 0,
    color: 0xffb34f5a,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Blend"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.5),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
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
            name: _tr("Fresnel"),
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
            name: _tr("Facing"),
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
