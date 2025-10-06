
function layer_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    let l: i32 = node.buttons[0].default_value[0];
    if (socket == node.outputs[0]) { // Base
        node_shader_add_texture(parser_material_kong, "texpaint" + l, "_texpaint" + l);
        return "sample(texpaint" + l + ", sampler_linear, tex_coord).rgb";
    }
    else { // Normal
        node_shader_add_texture(parser_material_kong, "texpaint_nor" + l, "_texpaint_nor" + l);
        return "sample(texpaint_nor" + l + ", sampler_linear, tex_coord).rgb";
    }
}

function layer_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let l: i32 = node.buttons[0].default_value[0];
    if (socket == node.outputs[1]) { // Opac
        node_shader_add_texture(parser_material_kong, "texpaint" + l, "_texpaint" + l);
        return "sample(texpaint" + l + ", sampler_linear, tex_coord).a";
    }
    else if (socket == node.outputs[2]) { // Occ
        node_shader_add_texture(parser_material_kong, "texpaint_pack" + l, "_texpaint_pack" + l);
        return "sample(texpaint_pack" + l + ", sampler_linear, tex_coord).r";
    }
    else if (socket == node.outputs[3]) { // Rough
        node_shader_add_texture(parser_material_kong, "texpaint_pack" + l, "_texpaint_pack" + l);
        return "sample(texpaint_pack" + l + ", sampler_linear, tex_coord).g";
    }
    else if (socket == node.outputs[4]) { // Metal
        node_shader_add_texture(parser_material_kong, "texpaint_pack" + l, "_texpaint_pack" + l);
        return "sample(texpaint_pack" + l + ", sampler_linear, tex_coord).b";
    }
    else if (socket == node.outputs[6]) { return "0.0"; } // Emission
    else if (socket == node.outputs[7]) { // Height
        node_shader_add_texture(parser_material_kong, "texpaint_pack" + l, "_texpaint_pack" + l);
        return "sample(texpaint_pack" + l + ", sampler_linear, tex_coord).a";
    }
    else { return "0.0"; } // Subsurface
}

let layer_node_def: ui_node_t = {
    id: 0,
    name: _tr("Layer"),
    type: "LAYER", // extension
    x: 0,
    y: 0,
    color: 0xff4982a0,
    inputs: [],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Base Color"),
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
            name: _tr("Opacity"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Occlusion"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Roughness"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Metallic"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Normal Map"),
            type: "VECTOR",
            color: -10238109,
            default_value: f32_array_create_xyz(0.5, 0.5, 1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Emission"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Height"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Subsurface"),
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
