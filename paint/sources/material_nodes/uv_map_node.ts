
function uv_map_node_init() {
	array_push(nodes_material_input, uv_map_node_def);
	map_set(parser_material_node_vectors, "UVMAP", uv_map_node_vector);
}

function uv_map_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	node_shader_context_add_elem(parser_material_kong.context, "tex", "short2norm");
	let uv_map: i32 = node.buttons[0].default_value[0];
	if (uv_map == 1 && mesh_data_get_vertex_array(context_raw.paint_object.data, "tex1") != null) {
		node_shader_context_add_elem(parser_material_kong.context, "tex1", "short2norm");
		node_shader_add_out(parser_material_kong, "tex_coord1: float2");
		node_shader_write_vert(parser_material_kong, "output.tex_coord1 = input.tex1;");
		return "float3(input.tex_coord1.x, input.tex_coord1.y, 0.0)";
	}
	else {
		return "float3(tex_coord.x, tex_coord.y, 0.0)";
	}
}

let uv_map_node_def: ui_node_t = {
	id : 0,
	name : _tr("UV Map"),
	type : "UVMAP",
	x : 0,
	y : 0,
	color : 0xffb34f5a,
	inputs : [],
	outputs : [ {
		id : 0,
		node_id : 0,
		name : _tr("UV"),
		type : "VECTOR",
		color : 0xff6363c7,
		default_value : f32_array_create_xyz(0.0, 0.0, 0.0),
		min : 0.0,
		max : 1.0,
		precision : 100,
		display : 0
	} ],
	buttons : [
		{
			name : _tr("UV Map"),
			type : "ENUM",
			output : -1,
			default_value : f32_array_create_x(0),
			data : u8_array_create_from_string("uv0" + "\n" + "uv1"),
			min : 0.0,
			max : 1.0,
			precision : 100,
			height : 0
		}
	],
	width : 0,
	flags : 0
};
