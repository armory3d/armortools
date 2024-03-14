
function make_node_preview_run(data: material_t, matcon: material_context_t, node: zui_node_t, group: zui_node_canvas_t, parents: zui_node_t[]): NodeShaderContextRaw {
	let context_id: string = "mesh";
	let con_mesh: NodeShaderContextRaw = node_shader_context_create(data, {
		name: context_id,
		depth_write: false,
		compare_mode: "always",
		cull_mode: "clockwise",
		vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}, {name: "col", data: "short4norm"}],
		color_attachments: ["RGBA32"]
	});

	con_mesh.allow_vcols = true;
	let vert: NodeShaderRaw = node_shader_context_make_vert(con_mesh);
	let frag: NodeShaderRaw = node_shader_context_make_frag(con_mesh);
	frag.ins = vert.outs;

	node_shader_write_attrib(vert, 'gl_Position = vec4(pos.xy * 3.0, 0.0, 1.0);'); // Pos unpack
	node_shader_write_attrib(vert, 'const vec2 madd = vec2(0.5, 0.5);');
	node_shader_add_out(vert, 'vec2 texCoord');
	node_shader_write_attrib(vert, 'texCoord = gl_Position.xy * madd + madd;');
	///if (!krom_opengl)
	node_shader_write_attrib(vert, 'texCoord.y = 1.0 - texCoord.y;');
	///end

	parser_material_init();
	parser_material_canvases = [context_raw.material.canvas];
	parser_material_nodes = context_raw.material.canvas.nodes;
	parser_material_links = context_raw.material.canvas.links;
	if (group != null) {
		parser_material_push_group(group);
		parser_material_parents = parents;
	}
	let links: zui_node_link_t[] = parser_material_links;
	let link: zui_node_link_t = { id: zui_get_link_id(links), from_id: node.id, from_socket: context_raw.node_preview_socket, to_id: -1, to_socket: -1 };
	links.push(link);

	parser_material_con = con_mesh;
	parser_material_vert = vert;
	parser_material_frag = frag;
	parser_material_curshader = frag;
	parser_material_matcon = matcon;

	parser_material_transform_color_space = false;
	let res: string = parser_material_write_result(link);
	parser_material_transform_color_space = true;
	let st: string = node.outputs[link.from_socket].type;
	if (st != "RGB" && st != "RGBA" && st != "VECTOR") {
		res = parser_material_to_vec3(res);
	}
	array_remove(links, link);

	node_shader_add_out(frag, 'vec4 fragColor');
	node_shader_write(frag, `vec3 basecol = ${res};`);
	node_shader_write(frag, 'fragColor = vec4(basecol.rgb, 1.0);');

	// frag.ndcpos = true;
	// add_out(vert, 'vec4 ndc');
	// write_attrib(vert, 'ndc = vec4(gl_Position.xyz * vec3(0.5, 0.5, 0.0) + vec3(0.5, 0.5, 0.0), 1.0);');

	parser_material_finalize(con_mesh);

	con_mesh.data.shader_from_source = true;
	con_mesh.data.vertex_shader = node_shader_get(vert);
	con_mesh.data.fragment_shader = node_shader_get(frag);

	return con_mesh;
}
