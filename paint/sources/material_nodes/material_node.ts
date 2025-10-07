
function material_node_init() {
    array_push(nodes_material_input, material_node_def);
    map_set(parser_material_node_vectors, "MATERIAL", material_node_vector);
	map_set(parser_material_node_values, "MATERIAL", material_node_value);
}

function material_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    let result: string = "float3(0.0, 0.0, 0.0)";
    let mi: i32 = node.buttons[0].default_value[0];
    if (mi >= project_materials.length) {
        return result;
    }
    let m: slot_material_t = project_materials[mi];
    let _nodes: ui_node_t[] = parser_material_nodes;
    let _links: ui_node_link_t[] = parser_material_links;
    parser_material_nodes = m.canvas.nodes;
    parser_material_links = m.canvas.links;
    array_push(parser_material_parents, node);
    let output_node: ui_node_t = parser_material_node_by_type(parser_material_nodes, "OUTPUT_MATERIAL_PBR");
    if (socket == node.outputs[0]) { // Base
        result = parser_material_parse_vector_input(output_node.inputs[0]);
    }
    else if (socket == node.outputs[5]) { // Normal
        result = parser_material_parse_vector_input(output_node.inputs[5]);
    }
    parser_material_nodes = _nodes;
    parser_material_links = _links;
    array_pop(parser_material_parents);
    return result;
}

function material_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let result: string = "0.0";
    let mi: i32 = node.buttons[0].default_value[0];
    if (mi >= project_materials.length) return result;
    let m: slot_material_t = project_materials[mi];
    let _nodes: ui_node_t[] = parser_material_nodes;
    let _links: ui_node_link_t[] = parser_material_links;
    parser_material_nodes = m.canvas.nodes;
    parser_material_links = m.canvas.links;
    array_push(parser_material_parents, node);
    let output_node: ui_node_t = parser_material_node_by_type(parser_material_nodes, "OUTPUT_MATERIAL_PBR");
    if (socket == node.outputs[1]) { // Opac
        result = parser_material_parse_value_input(output_node.inputs[1]);
    }
    else if (socket == node.outputs[2]) { // Occ
        result = parser_material_parse_value_input(output_node.inputs[2]);
    }
    else if (socket == node.outputs[3]) { // Rough
        result = parser_material_parse_value_input(output_node.inputs[3]);
    }
    else if (socket == node.outputs[4]) { // Metal
        result = parser_material_parse_value_input(output_node.inputs[4]);
    }
    else if (socket == node.outputs[7]) { // Height
        result = parser_material_parse_value_input(output_node.inputs[7]);
    }
    parser_material_nodes = _nodes;
    parser_material_links = _links;
    array_pop(parser_material_parents);
    return result;
}

let material_node_def: ui_node_t = {
    id: 0,
    name: _tr("Material"),
    type: "MATERIAL", // extension
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
            name: _tr("Material"),
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
