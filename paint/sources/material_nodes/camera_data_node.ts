
function camera_data_node_init() {
	array_push(nodes_material_input, camera_data_node_def);
	map_set(parser_material_node_vectors, "CAMERA", camera_data_node_vector);
	map_set(parser_material_node_values, "CAMERA", camera_data_node_value);
}

function camera_data_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	parser_material_kong.frag_vvec_cam = true;
	return "vvec_cam";
}

function camera_data_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	if (socket == node.outputs[1]) { // View Z Depth
		node_shader_add_constant(parser_material_kong, "camera_proj: float2", "_camera_plane_proj");
		parser_material_kong.frag_wvpposition = true;
		return "(constants.camera_proj.y / ((input.wvpposition.z / input.wvpposition.w) - constants.camera_proj.x))";
	}
	else { // View Distance
		node_shader_add_constant(parser_material_kong, "eye: float3", "_camera_pos");
		parser_material_kong.frag_wposition = true;
		return "distance(constants.eye, input.wposition)";
	}
}

let camera_data_node_def: ui_node_t = {
	id : 0,
	name : _tr("Camera Data"),
	type : "CAMERA",
	x : 0,
	y : 0,
	color : 0xffb34f5a,
	inputs : [],
	outputs : [
		{
			id : 0,
			node_id : 0,
			name : _tr("View Vector"),
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
			name : _tr("View Z Depth"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.0),
			min : 0.0,
			max : 1.0,
			precision : 100,
			display : 0
		},
		{
			id : 0,
			node_id : 0,
			name : _tr("View Distance"),
			type : "VALUE",
			color : 0xffa1a1a1,
			default_value : f32_array_create_x(0.0),
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
