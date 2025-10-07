
function color_mask_node_init() {
    array_push(nodes_material_converter, color_mask_node_def);
    map_set(parser_material_node_values, "COLMASK", color_mask_node_value);
}

function color_mask_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let input_color: string = parser_material_parse_vector_input(node.inputs[0]);
    let mask_color: string = parser_material_parse_vector_input(node.inputs[1]);
    let radius: string = parser_material_parse_value_input(node.inputs[2]);
    let fuzziness: string = parser_material_parse_value_input(node.inputs[3]);
    return "clamp(1.0 - (distance(" + input_color + ", " + mask_color + ") - " + radius + ") / max(" + fuzziness + ", " + parser_material_eps + "), 0.0, 1.0)";
}

let color_mask_node_def: ui_node_t = {
    id: 0,
    name: _tr("Color Mask"),
    type: "COLMASK", // extension
    x: 0,
    y: 0,
    color: 0xff62676d,
    inputs: [
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
            name: _tr("Mask Color"),
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
            name: _tr("Radius"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.1),
            min: 0.0,
            max: 1.74,
            precision: 100,
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Fuzziness"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(0.0),
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
            name: _tr("Mask"),
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
