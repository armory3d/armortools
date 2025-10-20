
function voronoi_texture_node_init() {
	array_push(nodes_material_texture, voronoi_texture_node_def);
	map_set(parser_material_node_vectors, "TEX_VORONOI", voronoi_texture_node_vector);
	map_set(parser_material_node_values, "TEX_VORONOI", voronoi_texture_node_value);
}

let str_tex_voronoi: string = "\
fun tex_voronoi(x: float3): float4 { \
	var p: float3 = floor3(x); \
	var f: float3 = frac3(x); \
	var id: float = 0.0; \
	var res: float = 100.0; \
	for (var k: int = 0; k <= 2; k += 1) \
	for (var j: int = 0; j <= 2; j += 1) \
	for (var i: int = 0; i <= 2; i += 1) { \
		var b: float3 = float3(float(i - 1), float(j - 1), float(k - 1)); \
		var pb: float3 = p + b; \
		var snoise_sample: float3 = sample(snoise256, sampler_linear, (pb.xy + float2(3.0, 1.0) * pb.z + 0.5) / 256.0).xyz; \
		var r: float3 = b - f + snoise_sample; \
		var d: float = dot(r, r); \
		if (d < res) { \
			id = dot(p + b, float3(1.0, 57.0, 113.0)); \
			res = d; \
		} \
	} \
	/*var col: float3 = 0.5 + 0.5 * cos(id * 0.35 + float3(0.0, 1.0, 2.0));*/ \
	var col: float3; \
	col.x = 0.5 + 0.5 * cos(id * 0.35 + 0.0); \
	col.y = 0.5 + 0.5 * cos(id * 0.35 + 1.0); \
	col.z = 0.5 + 0.5 * cos(id * 0.35 + 2.0); \
	return float4(col, sqrt(res)); \
} \
";

function voronoi_texture_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	node_shader_add_function(parser_material_kong, str_tex_voronoi);
	node_shader_add_texture(parser_material_kong, "snoise256", "$noise256.k");
	let co: string            = parser_material_get_coord(node);
	let scale: string         = parser_material_parse_value_input(node.inputs[1]);
	let but: ui_node_button_t = node.buttons[0]; // coloring;
	let coloring: string      = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
	coloring                  = string_replace_all(coloring, " ", "_");
	let res: string           = "";
	if (coloring == "INTENSITY") {
		let voronoi: string = "tex_voronoi(" + co + " * " + scale + ").a";
		res                 = parser_material_to_vec3(voronoi);
	}
	else { // Cells
		res = "tex_voronoi(" + co + " * " + scale + ").rgb";
	}
	return res;
}

function voronoi_texture_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	node_shader_add_function(parser_material_kong, str_tex_voronoi);
	node_shader_add_texture(parser_material_kong, "snoise256", "$noise256.k");
	let co: string            = parser_material_get_coord(node);
	let scale: string         = parser_material_parse_value_input(node.inputs[1]);
	let but: ui_node_button_t = node.buttons[0]; // coloring
	let coloring: string      = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
	coloring                  = string_replace_all(coloring, " ", "_");
	let res: string           = "";
	if (coloring == "INTENSITY") {
		res = "tex_voronoi(" + co + " * " + scale + ").a";
	}
	else { // Cells
		res = "tex_voronoi(" + co + " * " + scale + ").r";
	}
	return res;
}

let voronoi_texture_node_def: ui_node_t = {
	id : 0,
	name : _tr("Voronoi Texture"),
	type : "TEX_VORONOI",
	x : 0,
	y : 0,
	color : 0xff4982a0,
	inputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("Vector"),
			type : "VECTOR",
			color : 0xff6363c7,
			default_value : f32_array_create_xyz(0.0, 0.0, 0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Scale"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(5.0),
			min : 0.0,
			max : 10.0,
			precision : 100,
			display : 0
		}
	],
	outputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("Color"),
			type : "RGBA",
			color : 0xffc7c729,
			default_value : f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("Factor"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	buttons : [ {
		name : _tr("coloring"),
		type : "ENUM",
		output : 0,
		default_value : f32_array_create_x(0),
		data : u8_array_create_from_string(_tr("Intensity") + "\n" + _tr("Cells")),
		min : 0.0,
		max : 1.0,
		precision : 100,
		height : 0
	} ],
	width : 0,
	flags : 0
};
