
function layer_mask_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    if (socket == node.outputs[0]) {
        let l: i32 = node.buttons[0].default_value[0];
        node_shader_add_texture(parser_material_kong, "texpaint" + l, "_texpaint" + l);
        return "sample(texpaint" + l + ", sampler_linear, tex_coord).r";
    }
}

let layer_mask_node_def: ui_node_t = {
    id: 0,
    name: _tr("Layer Mask"),
    type: "LAYER_MASK", // extension
    x: 0,
    y: 0,
    color: 0xff4982a0,
    inputs: [],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Value"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        }
    ],
    buttons: [
        {
            name: _tr("Layer"),
            type: "ENUM",
            output: -1,
            default_value: f32_array_create_x(0),
            data: u8_array_create_from_string(""),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
