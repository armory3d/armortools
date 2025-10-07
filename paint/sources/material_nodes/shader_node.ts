
function shader_node_init() {
    array_push(nodes_material_input, shader_node_def);
    map_set(parser_material_node_values, "SHADER_GPU", shader_node_value);
}

function shader_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let shader: buffer_t = node.buttons[0].default_value;
    let str: string = sys_buffer_to_string(shader);
    return str == "" ? "0.0" : str;
}

let shader_node_def: ui_node_t = {
    id: 0,
    name: _tr("Shader"),
    type: "SHADER_GPU", // extension
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
            name: " ",
            type: "STRING",
            output: -1,
            default_value: f32_array_create_x(0), // "",
            data: null,
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
