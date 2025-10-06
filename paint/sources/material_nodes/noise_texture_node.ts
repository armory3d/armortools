
let str_tex_noise: string = "\
fun hash(n: float): float { return frac(sin(n) * 10000.0); } \
fun tex_noise_f(x: float3): float { \
    var step: float3 = float3(110.0, 241.0, 171.0); \
    var i: float3 = floor3(x); \
    var f: float3 = frac3(x); \
    var n: float = dot(i, step); \
    var u: float3 = f * f * (3.0 - 2.0 * f); \
    return lerp(lerp(lerp(hash(n + dot(step, float3(0.0, 0.0, 0.0))), hash(n + dot(step, float3(1.0, 0.0, 0.0))), u.x), \
                     lerp(hash(n + dot(step, float3(0.0, 1.0, 0.0))), hash(n + dot(step, float3(1.0, 1.0, 0.0))), u.x), u.y), \
                lerp(lerp(hash(n + dot(step, float3(0.0, 0.0, 1.0))), hash(n + dot(step, float3(1.0, 0.0, 1.0))), u.x), \
                     lerp(hash(n + dot(step, float3(0.0, 1.0, 1.0))), hash(n + dot(step, float3(1.0, 1.0, 1.0))), u.x), u.y), u.z); \
} \
fun tex_noise(p: float3): float { \
	p = p * 1.25; \
	var f: float = 0.5 * tex_noise_f(p); p = p * 2.01; \
	f += 0.25 * tex_noise_f(p); p = p * 2.02; \
	f += 0.125 * tex_noise_f(p); p = p * 2.03; \
	f += 0.0625 * tex_noise_f(p); \
	return 1.0 - f; \
} \
";

function noise_texture_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    node_shader_add_function(parser_material_kong, str_tex_noise);
    let co: string = parser_material_get_coord(node);
    let scale: string = parser_material_parse_value_input(node.inputs[1]);
    let res: string = "float3(tex_noise(" + co + " * " + scale + "), tex_noise(" + co + " * " + scale + " + 0.33), tex_noise(" + co + " * " + scale + " + 0.66))";
    return res;
}

function noise_texture_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    node_shader_add_function(parser_material_kong, str_tex_noise);
    let co: string = parser_material_get_coord(node);
    let scale: string = parser_material_parse_value_input(node.inputs[1]);
    let res: string = "tex_noise(" + co + " * " + scale + ")";
    return res;
}

let noise_texture_node_def: ui_node_t = {
    id: 0,
    name: _tr("Noise Texture"),
    type: "TEX_NOISE",
    x: 0,
    y: 0,
    color: 0xff4982a0,
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
            display: 0
        },
        {
            id: 0,
            node_id: 0,
            name: _tr("Scale"),
            type: "VALUE",
            color: 0xffa1a1a1,
            default_value: f32_array_create_x(5.0),
            min: 0.0,
            max: 10.0,
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
        }
    ],
    buttons: [],
    width: 0,
    flags: 0
};
