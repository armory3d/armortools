package arm.node;

import zui.Nodes;
import iron.data.SceneFormat;
import arm.shader.MaterialParser;
import arm.shader.NodeShader;
import arm.shader.NodeShaderData;
import arm.shader.NodeShaderContext;

class MakeNodePreview {

	@:access(arm.shader.MaterialParser)
	public static function run(data: NodeShaderData, matcon: TMaterialContext, node: TNode): NodeShaderContext {
		var context_id = "mesh";
		var con_mesh: NodeShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: "less",
			cull_mode: "clockwise",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}, {name: "col", data: "short4norm"}],
			color_attachments: ["RGBA32"]
		});

		con_mesh.allow_vcols = true;
		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();
		frag.ins = vert.outs;

		vert.write_attrib('gl_Position = vec4(pos.xy * 3.0, 0.0, 1.0);'); // Pos unpack
		vert.write_attrib('const vec2 madd = vec2(0.5, 0.5);');
		vert.add_out('vec2 texCoord');
		vert.write_attrib('texCoord = gl_Position.xy * madd + madd;');
		#if (!kha_opengl)
		vert.write_attrib('texCoord.y = 1.0 - texCoord.y;');
		#end

		var canvas_nodes = Context.material.canvas.nodes;
		var canvas_links = Context.material.canvas.links;
		var nodes = Context.material.nodes;

		var link: TNodeLink = { id: nodes.getLinkId(canvas_links), from_id: node.id, from_socket: Context.nodePreviewSocket, to_id: 0, to_socket: 0 };
		canvas_links.push(link);

		MaterialParser.init();
		MaterialParser.nodes = canvas_nodes;
		MaterialParser.links = canvas_links;
		MaterialParser.con = con_mesh;
		MaterialParser.vert = vert;
		MaterialParser.frag = frag;
		MaterialParser.curshader = frag;
		MaterialParser.matcon = matcon;

		var res = MaterialParser.write_result(link);
		var st = node.outputs[link.from_socket].type;
		if (st != "RGB" && st != "RGBA" && st != "VECTOR") {
			res = MaterialParser.to_vec3(res);
		}
		canvas_links.remove(link);

		frag.add_out('vec4 fragColor');
		frag.write('vec3 basecol = $res;');
		frag.write('fragColor = vec4(basecol.rgb, 1.0);');

		// frag.ndcpos = true;
		// vert.add_out('vec4 ndc');
		// vert.write_attrib('ndc = vec4(gl_Position.xyz * vec3(0.5, 0.5, 0.0) + vec3(0.5, 0.5, 0.0), 1.0);');

		MaterialParser.finalize(con_mesh);

		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}
}
