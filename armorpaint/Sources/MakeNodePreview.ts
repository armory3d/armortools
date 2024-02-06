
class MakeNodePreview {

	static run = (data: TMaterial, matcon: material_context_t, node: zui_node_t, group: zui_node_canvas_t, parents: zui_node_t[]): NodeShaderContextRaw => {
		let context_id = "mesh";
		let con_mesh = NodeShaderContext.create(data, {
			name: context_id,
			depth_write: false,
			compare_mode: "always",
			cull_mode: "clockwise",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}, {name: "col", data: "short4norm"}],
			color_attachments: ["RGBA32"]
		});

		con_mesh.allow_vcols = true;
		let vert = NodeShaderContext.make_vert(con_mesh);
		let frag = NodeShaderContext.make_frag(con_mesh);
		frag.ins = vert.outs;

		NodeShader.write_attrib(vert, 'gl_Position = vec4(pos.xy * 3.0, 0.0, 1.0);'); // Pos unpack
		NodeShader.write_attrib(vert, 'const vec2 madd = vec2(0.5, 0.5);');
		NodeShader.add_out(vert, 'vec2 texCoord');
		NodeShader.write_attrib(vert, 'texCoord = gl_Position.xy * madd + madd;');
		///if (!krom_opengl)
		NodeShader.write_attrib(vert, 'texCoord.y = 1.0 - texCoord.y;');
		///end

		ParserMaterial.init();
		ParserMaterial.canvases = [Context.raw.material.canvas];
		ParserMaterial.nodes = Context.raw.material.canvas.nodes;
		ParserMaterial.links = Context.raw.material.canvas.links;
		if (group != null) {
			ParserMaterial.push_group(group);
			ParserMaterial.parents = parents;
		}
		let links = ParserMaterial.links;
		let nodes = Context.raw.material.nodes;

		let link: zui_node_link_t = { id: zui_get_link_id(links), from_id: node.id, from_socket: Context.raw.nodePreviewSocket, to_id: -1, to_socket: -1 };
		links.push(link);

		ParserMaterial.con = con_mesh;
		ParserMaterial.vert = vert;
		ParserMaterial.frag = frag;
		ParserMaterial.curshader = frag;
		ParserMaterial.matcon = matcon;

		ParserMaterial.transform_color_space = false;
		let res = ParserMaterial.write_result(link);
		ParserMaterial.transform_color_space = true;
		let st = node.outputs[link.from_socket].type;
		if (st != "RGB" && st != "RGBA" && st != "VECTOR") {
			res = ParserMaterial.to_vec3(res);
		}
		array_remove(links, link);

		NodeShader.add_out(frag, 'vec4 fragColor');
		NodeShader.write(frag, `vec3 basecol = ${res};`);
		NodeShader.write(frag, 'fragColor = vec4(basecol.rgb, 1.0);');

		// frag.ndcpos = true;
		// NodeShader.add_out(vert, 'vec4 ndc');
		// NodeShader.write_attrib(vert, 'ndc = vec4(gl_Position.xyz * vec3(0.5, 0.5, 0.0) + vec3(0.5, 0.5, 0.0), 1.0);');

		ParserMaterial.finalize(con_mesh);

		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = NodeShader.get(vert);
		con_mesh.data.fragment_shader = NodeShader.get(frag);

		return con_mesh;
	}
}
