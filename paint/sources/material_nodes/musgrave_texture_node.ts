
let str_tex_musgrave: string = "\
fun random3(c: float3): float3 { \
	var j: float = 4096.0 * sin(dot(c, float3(17.0, 59.4, 15.0))); \
	var r: float3; \
	r.z = frac(512.0 * j); \
	j *= 0.125; \
	r.x = frac(512.0 * j); \
	j *= 0.125; \
	r.y = frac(512.0 * j); \
	return r - 0.5; \
} \
fun tex_musgrave_f(p: float3): float { \
	var F3: float = 0.3333333; \
	var G3: float = 0.1666667; \
	var s: float3 = floor3(p + dot(p, float3(F3, F3, F3))); \
	var x: float3 = p - s + dot(s, float3(G3, G3, G3)); \
	var e: float3 = step3(float3(0.0, 0.0, 0.0), x - x.yzx); \
	var i1: float3 = e * (1.0 - e.zxy); \
	var i2: float3 = 1.0 - e.zxy * (1.0 - e); \
	var x1: float3 = x - i1 + G3; \
	var x2: float3 = x - i2 + 2.0 * G3; \
	var x3: float3 = x - 1.0 + 3.0 * G3; \
	var w: float4; \
	var d: float4; \
	w.x = dot(x, x); \
	w.y = dot(x1, x1); \
	w.z = dot(x2, x2); \
	w.w = dot(x3, x3); \
	w = max4(float4(0.6, 0.6, 0.6, 0.6) - w, float4(0.0, 0.0, 0.0, 0.0)); \
	d.x = dot(random3(s), x); \
	d.y = dot(random3(s + i1), x1); \
	d.z = dot(random3(s + i2), x2); \
	d.w = dot(random3(s + 1.0), x3); \
	w *= w; \
	w *= w; \
	d *= w; \
	return clamp(dot(d, float4(52.0, 52.0, 52.0, 52.0)), 0.0, 1.0); \
} \
";

function musgrave_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
    node_shader_add_function(parser_material_kong, str_tex_musgrave);
    let co: string = parser_material_get_coord(node);
    let scale: string = parser_material_parse_value_input(node.inputs[1]);
    let res: string = "tex_musgrave_f(" + co + " * " + scale + " * 0.5)";
    return res;
}

let musgrave_texture_node_def: ui_node_t = {
    id: 0,
    name: _tr("Musgrave Texture"),
    type: "TEX_MUSGRAVE",
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
            name: _tr("Height"),
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
