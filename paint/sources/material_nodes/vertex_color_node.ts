
function vertex_color_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    if (parser_material_kong.context.allow_vcols) {
        node_shader_context_add_elem(parser_material_kong.context, "col", "short4norm");
        return "input.vcolor";
    }
    else {
        return("float3(0.0, 0.0, 0.0)");
    }
}

function vertex_color_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    return "1.0";
}

let vertex_color_node_def: ui_node_t = {
    id: 0,
    name: _tr("Vertex Color"),
    type: "VERTEX_COLOR",
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
            default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Alpha"),
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
