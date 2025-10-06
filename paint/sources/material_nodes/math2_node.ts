
function math2_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let val1: string = parser_material_parse_value_input(node.inputs[0]);
    let val2: string = parser_material_parse_value_input(node.inputs[1]);
    let but: ui_node_button_t = node.buttons[0]; // operation
    let op: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
    op = string_replace_all(op, " ", "_");
    let use_clamp: bool = node.buttons[1].default_value[0] > 0;
    let out_val: string = "";
    if (op == "ADD") {
        out_val = "(" + val1 + " + " + val2 + ")";
    }
    else if (op == "SUBTRACT") {
        out_val = "(" + val1 + " - " + val2 + ")";
    }
    else if (op == "MULTIPLY") {
        out_val = "(" + val1 + " * " + val2 + ")";
    }
    else if (op == "DIVIDE") {
        let store: string = parser_material_store_var_name(node) + "_divide";
        parser_material_write(parser_material_kong, "var " + store + ": float = " + val2 + ";");
        parser_material_write(parser_material_kong, "if (" + store + " == 0.0) { " + store + " = " + parser_material_eps + "; }");
        out_val = "(" + val1 + " / " + store + ")";
    }
    else if (op == "POWER") {
        out_val = "pow(" + val1 + ", " + val2 + ")";
    }
    else if (op == "LOGARITHM") {
        out_val = "log(" + val1 + ")";
    }
    else if (op == "SQUARE_ROOT") {
        out_val = "sqrt(" + val1 + ")";
    }
    else if(op == "INVERSE_SQUARE_ROOT") {
        out_val = "rsqrt(" + val1 + ")";
    }
    else if (op == "EXPONENT") {
        out_val = "exp(" + val1 + ")";
    }
    else if (op == "ABSOLUTE") {
        out_val = "abs(" + val1 + ")";
    }
    else if (op == "MINIMUM") {
        out_val = "min(" + val1 + ", " + val2 + ")";
    }
    else if (op == "MAXIMUM") {
        out_val = "max(" + val1 + ", " + val2 + ")";
    }
    else if (op == "LESS_THAN") {
        // out_val = "float(" + val1 + " < " + val2 + ")";
        let store: string = parser_material_store_var_name(node) + "_lessthan";
        parser_material_write(parser_material_kong, "var " + store + ": float = 0.0;");
        parser_material_write(parser_material_kong, "if (" + val1 + " < " + val2 + ") { " + store + " = 1.0; }");
        out_val = store;
    }
    else if (op == "GREATER_THAN") {
        // out_val = "float(" + val1 + " > " + val2 + ")";
        let store: string = parser_material_store_var_name(node) + "_greaterthan";
        parser_material_write(parser_material_kong, "var " + store + ": float = 0.0;");
        parser_material_write(parser_material_kong, "if (" + val1 + " > " + val2 + ") { " + store + " = 1.0; }");
        out_val = store;
    }
    else if (op == "SIGN") {
        out_val = "sign(" + val1 + ")";
    }
    else if (op == "ROUND") {
        out_val = "floor(" + val1 + " + 0.5)";
    }
    else if (op == "FLOOR") {
        out_val = "floor(" + val1 + ")";
    }
    else if (op == "CEIL") {
        out_val = "ceil(" + val1 + ")";
    }
    else if(op == "SNAP") {
        out_val = "(floor(" + val1 + " / " + val2 + ") * " + val2 + ")";
    }
    else if (op == "TRUNCATE") {
        out_val = "trunc(" + val1 + ")";
    }
    else if (op == "FRACTION") {
        out_val = "frac(" + val1 + ")";
    }
    else if (op == "MODULO") {
        out_val = "(" + val1 + " % " + val2 + ")";
    }
    else if (op == "PING-PONG") {
        let store: string = parser_material_store_var_name(node) + "_pingpong";
        parser_material_write(parser_material_kong, "var " + store + ": float = 0.0;");
        parser_material_write(parser_material_kong, "if (" + val2 + " != 0.0) { " + store + " = abs(frac((" + val1 + " - " + val2 + ") / (" + val2 + " * 2.0)) * " + val2 + " * 2.0 - " + val2 + "); }");
        out_val = store;
    }
    else if (op == "SINE") {
        out_val = "sin(" + val1 + ")";
    }
    else if (op == "COSINE") {
        out_val = "cos(" + val1 + ")";
    }
    else if (op == "TANGENT") {
        out_val = "tan(" + val1 + ")";
    }
    else if (op == "ARCSINE") {
        out_val = "asin(" + val1 + ")";
    }
    else if (op == "ARCCOSINE") {
        out_val = "acos(" + val1 + ")";
    }
    else if (op == "ARCTANGENT") {
        out_val = "atan(" + val1 + ")";
    }
    else if (op == "ARCTAN2") {
        out_val = "atan2(" + val1 + ", " + val2 + ")";
    }
    else if (op == "HYPERBOLIC_SINE") {
        out_val = "sinh(" + val1 + ")";
    }
    else if (op == "HYPERBOLIC_COSINE") {
        out_val = "cosh(" + val1 + ")";
    }
    else if (op == "HYPERBOLIC_TANGENT") {
        out_val = "tanh(" + val1 + ")";
    }
    else if (op == "TO_RADIANS") {
        out_val = "radians(" + val1 + ")";
    }
    else if (op == "TO_DEGREES") {
        out_val = "degrees(" + val1 + ")";
    }
    if (use_clamp) {
        return "clamp(" + out_val + ", 0.0, 1.0)";
    }
    else {
        return out_val;
    }
}

let math2_node_def: ui_node_t = {
    id: 0,
    name: _tr("Math"),
    type: "MATH",
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
            data: u8_array_create_from_string(_tr("Add") + "\n" + _tr("Subtract") + "\n" + _tr("Multiply") + "\n" + _tr("Divide") + "\n" + _tr("Power") + "\n" + _tr("Logarithm") + "\n" + _tr("Square Root") + "\n" + _tr("Inverse Square Root") + "\n" + _tr("Absolute") + "\n" + _tr("Exponent") + "\n" + _tr("Minimum") + "\n" + _tr("Maximum") + "\n" + _tr("Less Than") + "\n" + _tr("Greater Than") + "\n" + _tr("Sign") + "\n" + _tr("Round") + "\n" + _tr("Floor") + "\n" + _tr("Ceil") + "\n" + _tr("Truncate") + "\n" + _tr("Fraction") + "\n" + _tr("Modulo") + "\n" + _tr("Snap") + "\n" + _tr("Ping-Pong") + "\n" + _tr("Sine") + "\n" + _tr("Cosine") + "\n" + _tr("Tangent") + "\n" + _tr("Arcsine") + "\n" + _tr("Arccosine") + "\n" + _tr("Arctangent") + "\n" + _tr("Arctan2") + "\n" + _tr("Hyperbolic Sine") + "\n" + _tr("Hyperbolic Cosine") + "\n" + _tr("Hyperbolic Tangent") + "\n" + _tr("To Radians") + "\n" + _tr("To Degrees")),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        },
        {
            name: _tr("use_clamp"),
            type: "BOOL",
            output: 0,
            default_value: f32_array_create_x(0),
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
