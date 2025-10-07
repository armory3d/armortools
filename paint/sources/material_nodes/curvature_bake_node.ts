
let parser_material_bake_passthrough_strength: string = "0.0";
let parser_material_bake_passthrough_radius: string = "0.0";
let parser_material_bake_passthrough_offset: string = "0.0";

function curvature_bake_node_init() {
    array_push(nodes_material_texture, curvature_bake_node_def);
    map_set(parser_material_node_values, "BAKE_CURVATURE", curvature_bake_node_value);
}

function curvature_bake_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    if (parser_material_bake_passthrough) {
        parser_material_bake_passthrough_strength = parser_material_parse_value_input(node.inputs[0]);
        parser_material_bake_passthrough_radius = parser_material_parse_value_input(node.inputs[1]);
        parser_material_bake_passthrough_offset = parser_material_parse_value_input(node.inputs[2]);
        return "0.0";
    }
    let tex_name: string = "texbake_" + parser_material_node_name(node);
    node_shader_add_texture(parser_material_kong, "" + tex_name, "_" + tex_name);
    let store: string = parser_material_store_var_name(node);
    parser_material_write(parser_material_kong, "var " + store + "_res: float = sample(" + tex_name + ", sampler_linear, tex_coord).r;");
    return store + "_res";
}

let curvature_bake_node_def: ui_node_t = {
    id: 0,
    name: _tr("Curvature Bake"),
    type: "BAKE_CURVATURE",
    x: 0,
    y: 0,
    color: 0xff4982a0,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Strength"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 2.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Radius"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
            min: 0.0,
            max: 2.0,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Offset"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.0),
            min: -2.0,
            max: 2.0,
            precision: 100,
            display: 0
        }
    ],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Value"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
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
