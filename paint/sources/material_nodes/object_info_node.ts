
function object_info_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    if (socket == node.outputs[0]) { // Location
        parser_material_kong.frag_wposition = true;
        return "input.wposition";
    }
    else if (socket == node.outputs[1]) { // Color
        return "float3(0.0, 0.0, 0.0)";
    }
}

function object_info_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    if (socket == node.outputs[1]) { // Object Index
        node_shader_add_constant(parser_material_kong, "object_info_index: float", "_object_info_index");
        return "constants.object_info_index";
    }
    else if (socket == node.outputs[2]) { // Material Index
        node_shader_add_constant(parser_material_kong, "object_info_material_index: float", "_object_info_material_index");
        return "constants.object_info_material_index";
    }
    else if (socket == node.outputs[3]) { // Random
        node_shader_add_constant(parser_material_kong, "object_info_random: float", "_object_info_random");
        return "constants.object_info_random";
    }
}

let object_info_node_def: ui_node_t = {
    id: 0,
    name: _tr("Object Info"),
    type: "OBJECT_INFO",
    x: 0,
    y: 0,
    color: 0xffb34f5a,
    inputs: [],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Location"),
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
            name: _tr("Color"),
            type: "RGBA",
            color: 0xffc7c729,
            default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Object Index"),
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
            name: _tr("Material Index"),
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
            name: _tr("Random"),
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
