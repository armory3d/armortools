
function uv_map_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    node_shader_context_add_elem(parser_material_kong.context, "tex", "short2norm");
	return "float3(tex_coord.x, tex_coord.y, 0.0)";
}

let uv_map_node_def: ui_node_t = {
    id: 0,
    name: _tr("UV Map"),
    type: "UVMAP",
    x: 0,
    y: 0,
    color: 0xffb34f5a,
    inputs: [],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("UV"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
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
