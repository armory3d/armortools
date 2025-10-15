
function wave_texture_node_init() {
	array_push(nodes_material_texture, wave_texture_node_def);
	map_set(parser_material_node_vectors, "TEX_WAVE", wave_texture_node_vector);
	map_set(parser_material_node_values, "TEX_WAVE", wave_texture_node_value);
}

let str_tex_wave: string = "\
fun tex_wave_f(p: float3): float { \
	return 1.0 - sin((p.x + p.y) * 10.0); \
} \
";

function wave_texture_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	node_shader_add_function(parser_material_kong, str_tex_wave);
	let co: string    = parser_material_get_coord(node);
	let scale: string = parser_material_parse_value_input(node.inputs[1]);
	let res: string   = parser_material_to_vec3("tex_wave_f(" + co + " * " + scale + ")");
	return res;
}

function wave_texture_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	node_shader_add_function(parser_material_kong, str_tex_wave);
	let co: string    = parser_material_get_coord(node);
	let scale: string = parser_material_parse_value_input(node.inputs[1]);
	let res: string   = "tex_wave_f(" + co + " * " + scale + ")";
	return res;
}

let wave_texture_node_def: ui_node_t = {
	id : 0,
	name : _tr("Wave Texture"),
	type : "TEX_WAVE",
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
			name : _tr("Fac"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(1.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		}
	],
	buttons : [],
	width : 0,
	flags : 0
};
