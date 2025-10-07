
function color_ramp_node_init() {
    array_push(nodes_material_converter, color_ramp_node_def);
    map_set(parser_material_node_vectors, "VALTORGB", color_ramp_node_vector);
}

function color_ramp_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    let fac: string = parser_material_parse_value_input(node.inputs[0]);
    let data0: i32 = node.buttons[0].data[0];
    let interp: string = data0 == 0 ? "LINEAR" : "CONSTANT";
    let elems: f32[] = node.buttons[0].default_value;
    let len: i32 = elems.length / 5;
    if (len == 1) {
        return parser_material_vec3(elems);
    }
    // Write cols array
    let cols_var: string = parser_material_node_name(node) + "_cols";
    parser_material_write(parser_material_kong, "var " + cols_var + ": float3[" + len + "];"); // TODO: Make const
    for (let i: i32 = 0; i < len; ++i) {
        let tmp: f32[] = [];
        array_push(tmp, elems[i * 5]);
        array_push(tmp, elems[i * 5 + 1]);
        array_push(tmp, elems[i * 5 + 2]);
        parser_material_write(parser_material_kong, cols_var + "[" + i + "] = " + parser_material_vec3(tmp) + ";");
    }
    // Get index
    let fac_var: string = parser_material_node_name(node) + "_fac";
    parser_material_write(parser_material_kong, "var " + fac_var + ": float = " + fac + ";");
    let index: string = "0";
    for (let i: i32 = 1; i < len; ++i) {
        let e: f32 = elems[i * 5 + 4];
        index += " + (" + fac_var + " > " + e + " ? 1 : 0)";
    }
    // Write index
    let index_var: string = parser_material_node_name(node) + "_i";
    parser_material_write(parser_material_kong, "var " + index_var + ": int = " + index + ";");
    if (interp == "CONSTANT") {
        return cols_var + "[" + index_var + "]";
    }
    else { // Linear
        // Write facs array
        let facs_var: string = parser_material_node_name(node) + "_facs";
        parser_material_write(parser_material_kong, "var " + facs_var + ": float[" + len + "];"); // TODO: Make const
        for (let i: i32 = 0; i < len; ++i) {
            let e: f32 = elems[i * 5 + 4];
            parser_material_write(parser_material_kong, facs_var + "[" + i + "] = " + e + ";");
        }
        // Mix color
        // float f = (pos - start) * (1.0 / (finish - start))
        // TODO: index_var + 1 out of bounds
        return "lerp3(" + cols_var + "[" + index_var + "], " + cols_var + "[" + index_var + " + 1], (" +
            fac_var + " - " + facs_var + "[" + index_var + "]) * (1.0 / (" + facs_var + "[" + index_var + " + 1] - " +
            facs_var + "[" + index_var + "]) ))";
    }
}

let color_ramp_node_def: ui_node_t = {
    id: 0,
    name: _tr("Color Ramp"),
    type: "VALTORGB",
    x: 0,
    y: 0,
    color: 0xff62676d,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Fac"),
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
            name: _tr("Color"),
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
            name: _tr("Alpha"),
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
            name: "nodes_material_color_ramp_button",
            type: "CUSTOM",
            output: 0,
            default_value: f32_array_create_xyzwv(1.0, 1.0, 1.0, 1.0, 0.0),
            data: u8_array_create(1),
            min: 0.0,
            max: 1.0,
            precision: 100,
            height: 4.5
        }
    ],
    width: 0,
    flags: 0
};
