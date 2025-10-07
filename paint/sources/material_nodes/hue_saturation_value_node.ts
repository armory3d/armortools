
function hue_saturation_value_node_init() {
    array_push(nodes_material_color, hue_saturation_value_node_def);
    map_set(parser_material_node_vectors, "HUE_SAT", hue_saturation_value_node_vector);
}

let str_hue_sat: string = "\
fun hsv_to_rgb(c: float3): float3 { \
	var K: float4 = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0); \
	var p: float3 = abs3(frac3(c.xxx + K.xyz) * 6.0 - K.www); \
	return lerp3(K.xxx, clamp3(p - K.xxx, float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0)), c.y) * c.z; \
} \
fun rgb_to_hsv(c: float3): float3 { \
	var K: float4 = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0); \
	var p: float4 = lerp4(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g)); \
	var q: float4 = lerp4(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r)); \
	var d: float = q.x - min(q.w, q.y); \
	var e: float = 0.0000000001; \
	return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x); \
} \
fun hue_sat(col: float3, shift: float4): float3 { \
	var hsv: float3 = rgb_to_hsv(col); \
	hsv.x += shift.x; \
	hsv.y *= shift.y; \
	hsv.z *= shift.z; \
	return lerp3(hsv_to_rgb(hsv), col, shift.w); \
} \
";

function hue_saturation_value_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    node_shader_add_function(parser_material_kong, str_hue_sat);
    let hue: string = parser_material_parse_value_input(node.inputs[0]);
    let sat: string = parser_material_parse_value_input(node.inputs[1]);
    let val: string = parser_material_parse_value_input(node.inputs[2]);
    let fac: string = parser_material_parse_value_input(node.inputs[3]);
    let col: string = parser_material_parse_vector_input(node.inputs[4]);
    return "hue_sat(" + col + ", float4(" + hue + " - 0.5, " + sat + ", " + val + ", 1.0 - " + fac + "))";
}

let hue_saturation_value_node_def: ui_node_t = {
    id: 0,
    name: _tr("Hue/Saturation/Value"),
    type: "HUE_SAT",
    x: 0,
    y: 0,
    color: 0xff448c6d,
    inputs: [
        {
            id: 0,
            node_id: 0,
            name: _tr("Hue"),
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
            name: _tr("Saturation"),
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
            name: _tr("Value"),
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
            name: _tr("Fac"),
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
