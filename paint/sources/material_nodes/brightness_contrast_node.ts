
function brightness_contrast_node_init() {
    array_push(nodes_material_color, brightness_contrast_node_def);
    map_set(parser_material_node_vectors, "BRIGHTCONTRAST", brightness_contrast_node_vector);
}

let str_brightcontrast: string = "\
fun brightcontrast(col: float3, bright: float, contr: float): float3 { \
	var a: float = 1.0 + contr; \
	var b: float = bright - contr * 0.5; \
	return max3(a * col + b, float3(0.0, 0.0, 0.0)); \
} \
";

function brightness_contrast_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    let out_col: string = parser_material_parse_vector_input(node.inputs[0]);
    let bright: string = parser_material_parse_value_input(node.inputs[1]);
    let contr: string = parser_material_parse_value_input(node.inputs[2]);
    node_shader_add_function(parser_material_kong, str_brightcontrast);
	return "brightcontrast(" + out_col + ", " + bright + ", " + contr + ")";
}

let brightness_contrast_node_def: ui_node_t = {
    id: 0,
    name: _tr("Brightness/Contrast"),
    type: "BRIGHTCONTRAST",
    x: 0,
    y: 0,
    color: 0xff448c6d,
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
            name: _tr("Bright"),
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
            name: _tr("Contrast"),
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
            name: _tr("Color"),
            type: "RGBA",
            color: 0xffc7c729,
            default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
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
