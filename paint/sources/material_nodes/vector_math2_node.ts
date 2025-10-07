
function vector_math2_node_init() {
    array_push(nodes_material_converter, vector_math2_node_def);
    map_set(parser_material_node_vectors, "VECT_MATH", vector_math2_node_vector);
	map_set(parser_material_node_values, "VECT_MATH", vector_math2_node_value);
}

function vector_math2_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    let vec1: string = parser_material_parse_vector_input(node.inputs[0]);
    let vec2: string = parser_material_parse_vector_input(node.inputs[1]);
    let but: ui_node_button_t = node.buttons[0]; //operation;
    let op: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
    op = string_replace_all(op, " ", "_");
    if (op == "ADD") {
        return "(" + vec1 + " + " + vec2 + ")";
    }
    else if (op == "SUBTRACT") {
        return "(" + vec1 + " - " + vec2 + ")";
    }
    else if (op == "AVERAGE") {
        return "((" + vec1 + " + " + vec2 + ") / 2.0)";
    }
    else if (op == "DOT_PRODUCT") {
        return parser_material_to_vec3("dot(" + vec1 + ", " + vec2 + ")");
    }
    else if (op == "LENGTH") {
        return parser_material_to_vec3("length(" + vec1 + ")");
    }
    else if (op == "DISTANCE") {
        return parser_material_to_vec3("distance(" + vec1 + ", " + vec2 + ")");
    }
    else if (op == "CROSS_PRODUCT") {
        return "cross(" + vec1 + ", " + vec2 + ")";
    }
    else if (op == "NORMALIZE") {
        return "normalize(" + vec1 + ")";
    }
    else if (op == "MULTIPLY") {
        return "(" + vec1 + " * " + vec2 + ")";
    }
    else if (op == "DIVIDE") {
        let store: string = parser_material_store_var_name(node) + "_vec2";
        parser_material_write(parser_material_kong, "var " + store + ": float3 = " + vec2 + ";");
        parser_material_write(parser_material_kong, "if (" + store + ".x == 0.0) { " + store + ".x = 0.000001; }");
        parser_material_write(parser_material_kong, "if (" + store + ".y == 0.0) { " + store + ".y = 0.000001; }");
        parser_material_write(parser_material_kong, "if (" + store + ".z == 0.0) { " + store + ".z = 0.000001; }");
        return "(" + vec1 + " / " + vec2 + ")";
    }
    else if (op == "PROJECT") {
        return "(dot(" + vec1 + ", " + vec2 + ") / dot(" + vec2 + ", " + vec2 + ") * " + vec2 + ")";
    }
    else if (op == "REFLECT") {
        return "reflect(" + vec1 + ", normalize(" + vec2 + "))";
    }
    else if (op == "SCALE") {
        return "(" + vec2 + ".x * " + vec1 + ")";
    }
    else if (op == "ABSOLUTE") {
        return "abs3(" + vec1 + ")";
    }
    else if (op == "MINIMUM") {
        return "min3(" + vec1 + ", " + vec2 + ")";
    }
    else if (op == "MAXIMUM") {
        return "max3(" + vec1 + ", " + vec2 + ")";
    }
    else if (op == "FLOOR") {
        return "floor3(" + vec1 + ")";
    }
    else if (op == "CEIL") {
        return "ceil3(" + vec1 + ")";
    }
    else if (op == "FRACTION") {
        return "frac3(" + vec1 + ")";
    }
    else if (op == "MODULO") {
        return "(" + vec1 + " % " + vec2 + ")";
    }
    else if(op == "SNAP") {
        return "(floor3(" + vec1 + " / " + vec2 + ") * " + vec2 + ")";
    }
    else if (op == "SINE") {
        return "float3(sin(" + vec1 + ".x), sin(" + vec1 + ".y), sin(" + vec1 + ".z))";
    }
    else if (op == "COSINE") {
        return "float3(cos(" + vec1 + ".x), cos(" + vec1 + ".y), cos(" + vec1 + ".z))";
    }
    else { // TANGENT
        return "float3(tan(" + vec1 + ".x), tan(" + vec1 + ".y), tan(" + vec1 + ".z))";
    }
}

function vector_math2_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    let vec1: string = parser_material_parse_vector_input(node.inputs[0]);
    let vec2: string = parser_material_parse_vector_input(node.inputs[1]);
    let but: ui_node_button_t = node.buttons[0]; //operation;
    let op: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
    op = string_replace_all(op, " ", "_");
    if (op == "DOT_PRODUCT") {
        return "dot(" + vec1 + ", " + vec2 + ")";
    }
    else if (op == "LENGTH") {
        return "length(" + vec1 + ")";
    }
    else if (op == "DISTANCE") {
        return "distance(" + vec1 + ", " + vec2 + ")";
    }
    else {
        return "0.0";
    }
}

let vector_math2_node_def: ui_node_t = {
    id: 0,
    name: _tr("Vector Math"),
    type: "VECT_MATH",
    x: 0,
    y: 0,
    color: 0xff62676d,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Vector"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 1
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Vector"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
            min: 0.0,
            max: 1.0,
            precision: 100,
            display: 1
        }
    ],
    outputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Vector"),
            type: "VECTOR",
            color: 0xff6363c7,
            default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
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
            data: u8_array_create_from_string(_tr("Add") + "\n" + _tr("Subtract") + "\n" + _tr("Multiply") + "\n" + _tr("Divide") + "\n" + _tr("Average") + "\n" + _tr("Cross Product") + "\n" + _tr("Project") + "\n" + _tr("Reflect") + "\n" + _tr("Dot Product") + "\n" + _tr("Distance") + "\n" + _tr("Length") + "\n" + _tr("Scale") + "\n" + _tr("Normalize") + "\n" + _tr("Absolute") + "\n" + _tr("Minimum") + "\n" + _tr("Maximum") + "\n" + _tr("Floor") + "\n" + _tr("Ceil") + "\n" + _tr("Fraction") + "\n" + _tr("Modulo") + "\n" + _tr("Snap") + "\n" + _tr("Sine") + "\n" + _tr("Cosine") + "\n" + _tr("Tangent")),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 0
        }
    ],
    width: 0,
    flags: 0
};
