
function camera_texture_node_init() {
	array_push(nodes_material_texture, camera_texture_node_def);
	map_set(parser_material_node_vectors, "TEX_CAMERA", camera_texture_node_vector);
}

function camera_texture_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let tex_name: string = "texcamera_" + parser_material_node_name(node);
	node_shader_add_texture(parser_material_kong, "" + tex_name, "_camera_texture");
	let store: string = parser_material_store_var_name(node);
	parser_material_write(parser_material_kong, "var " + store + "_res: float3 = sample(" + tex_name + ", sampler_linear, tex_coord).rgb;");
	return store + "_res";
}

let camera_texture_node_def: ui_node_t = {
	id : 0,
	name : _tr("Camera Texture"),
	type : "TEX_CAMERA", // extension
	x : 0,
	y : 0,
	color : 0xff4982a0,
	inputs : [],
	outputs : [ {
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
	} ],
	buttons : [],
	width : 0,
	flags : 0
};
