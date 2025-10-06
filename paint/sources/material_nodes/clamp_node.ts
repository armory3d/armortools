
function clamp_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let val: string = parser_material_parse_value_input(node.inputs[0]);
    let min: string = parser_material_parse_value_input(node.inputs[1]);
    let max: string = parser_material_parse_value_input(node.inputs[2]);
    let but: ui_node_button_t = node.buttons[0]; //operation;
    let op: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
    op = string_replace_all(op, " ", "_");
    if (op == "MIN_MAX") {
        return "(clamp(" + val + ", " + min + ", " + max + "))";
    }
    else { // RANGE
        return "(clamp(" + val + ", min(" + min + ", " + max + "), max(" + min + ", " + max + ")))";
    }
}

let clamp_node_def: ui_node_t = {
    id: 0,
    name: _tr("Clamp"),
    type: "CLAMP",
    x: 0,
    y: 0,
    color: 0xff62676d,
    inputs: [
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
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Min"),
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
            name: _tr("Max"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(1.0),
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
            name: _tr("operation"),
            type: "ENUM",
            output: 0,
            default_value: f32_array_create_x(0),
            data: u8_array_create_from_string(_tr("Min Max") + "\n" + _tr("Range")),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
