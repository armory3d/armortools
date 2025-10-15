
function make_node_preview_run(data: material_t, matcon: material_context_t, node: ui_node_t, group: ui_node_canvas_t,
                               parents: ui_node_t[]): node_shader_context_t {
	let context_id: string      = "mesh";
	let props: shader_context_t = {
		name : context_id,
		depth_write : false,
		compare_mode : "always",
		cull_mode : "clockwise",
		vertex_elements : [
			{name : "pos", data : "short4norm"}, {name : "nor", data : "short2norm"}, {name : "tex", data : "short2norm"}, {name : "col", data : "short4norm"}
		],
		color_attachments : [ "RGBA32" ]
	};
	let con_mesh: node_shader_context_t = node_shader_context_create(data, props);

	con_mesh.allow_vcols = true;

	let kong: node_shader_t = node_shader_context_make_kong(con_mesh);

	node_shader_write_attrib_vert(kong, "output.pos = float4(input.pos.xy * 3.0, 0.0, 1.0);"); // Pos unpack
	node_shader_write_attrib_vert(kong, "var madd: float2 = float2(0.5, 0.5);");
	node_shader_add_out(kong, "tex_coord: float2");
	node_shader_write_attrib_vert(kong, "output.tex_coord = output.pos.xy * madd + madd;");
	node_shader_write_attrib_vert(kong, "output.tex_coord.y = 1.0 - output.tex_coord.y;");
	node_shader_write_attrib_frag(kong, "var tex_coord: float2 = input.tex_coord;");

	parser_material_init();
	parser_material_canvases = [ context_raw.material.canvas ];
	parser_material_nodes    = context_raw.material.canvas.nodes;
	parser_material_links    = context_raw.material.canvas.links;
	if (group != null) {
		parser_material_push_group(group);
		parser_material_parents = parents;
	}
	let links: ui_node_link_t[] = parser_material_links;
	let socket_preview: i32     = i32_imap_get(context_raw.node_preview_socket_map, node.id);
	let link:
	    ui_node_link_t = {id : ui_next_link_id(links), from_id : node.id, from_socket : socket_preview == -1 ? 0 : socket_preview, to_id : -1, to_socket : -1};
	array_push(links, link);

	parser_material_con    = con_mesh;
	parser_material_kong   = kong;
	parser_material_matcon = matcon;

	parser_material_transform_color_space = false;
	let res: string                       = parser_material_write_result(link);
	parser_material_transform_color_space = true;
	let st: string                        = node.outputs[link.from_socket].type;
	if (st != "RGB" && st != "RGBA" && st != "VECTOR") {
		res = parser_material_to_vec3(res);
	}
	array_remove(links, link);

	kong.frag_out = "float4";
	node_shader_write_frag(kong, "var basecol: float3 = " + res + ";");
	node_shader_write_frag(kong, "output = float4(basecol.rgb, 1.0);");

	parser_material_finalize(con_mesh);

	con_mesh.data.shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_mesh.data.vertex_shader), ADDRESS(con_mesh.data.fragment_shader),
	                             ADDRESS(con_mesh.data._.vertex_shader_size), ADDRESS(con_mesh.data._.fragment_shader_size));

	return con_mesh;
}
