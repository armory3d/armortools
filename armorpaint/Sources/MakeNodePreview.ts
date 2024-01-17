
class MakeNodePreview {

	static run = (data: NodeShaderData, matcon: TMaterialContext, node: TNode, group: TNodeCanvas, parents: TNode[]): NodeShaderContext => {
		let context_id = "mesh";
		let con_mesh: NodeShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: "always",
			cull_mode: "clockwise",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}, {name: "col", data: "short4norm"}],
			color_attachments: ["RGBA32"]
		});

		con_mesh.allow_vcols = true;
		let vert = con_mesh.make_vert();
		let frag = con_mesh.make_frag();
		frag.ins = vert.outs;

		vert.write_attrib('gl_Position = vec4(pos.xy * 3.0, 0.0, 1.0);'); // Pos unpack
		vert.write_attrib('const vec2 madd = vec2(0.5, 0.5);');
		vert.add_out('vec2 texCoord');
		vert.write_attrib('texCoord = gl_Position.xy * madd + madd;');
		///if (!krom_opengl)
		vert.write_attrib('texCoord.y = 1.0 - texCoord.y;');
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

		let link: TNodeLink = { id: nodes.getLinkId(links), from_id: node.id, from_socket: Context.raw.nodePreviewSocket, to_id: -1, to_socket: -1 };
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

		frag.add_out('vec4 fragColor');
		frag.write(`vec3 basecol = ${res};`);
		frag.write('fragColor = vec4(basecol.rgb, 1.0);');

		// frag.ndcpos = true;
		// vert.add_out('vec4 ndc');
		// vert.write_attrib('ndc = vec4(gl_Position.xyz * vec3(0.5, 0.5, 0.0) + vec3(0.5, 0.5, 0.0), 1.0);');

		ParserMaterial.finalize(con_mesh);

		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}
}
