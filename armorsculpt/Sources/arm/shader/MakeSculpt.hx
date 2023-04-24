package arm.shader;

import iron.data.SceneFormat;
import zui.Nodes;
import arm.ui.UINodes;
import arm.shader.MaterialParser;
import arm.shader.NodeShaderContext;
import arm.shader.NodeShaderData;
import arm.shader.ShaderFunctions;

class MakeSculpt {

	public static function run(data: NodeShaderData, matcon: TMaterialContext): NodeShaderContext {
		var context_id = "paint";
		var con_paint:NodeShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: "always", // TODO: align texcoords winding order
			// cull_mode: "counter_clockwise",
			cull_mode: "none",
			vertex_elements: [{name: "pos", data: "float2"}],
			color_attachments: ["RGBA32", "RGBA32", "RGBA32", "R8"]
		});

		con_paint.data.color_writes_red = [true, true, true, true];
		con_paint.data.color_writes_green = [true, true, true, true];
		con_paint.data.color_writes_blue = [true, true, true, true];
		con_paint.data.color_writes_alpha = [true, true, true, true];
		con_paint.allow_vcols = Context.raw.paintObject.data.geom.cols != null;

		var vert = con_paint.make_vert();
		var frag = con_paint.make_frag();
		frag.ins = vert.outs;

		var faceFill = Context.raw.tool == ToolFill && Context.raw.fillTypeHandle.position == FillFace;
		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;

		vert.add_out('vec2 texCoord');
		vert.write('const vec2 madd = vec2(0.5, 0.5);');
		vert.write('texCoord = pos.xy * madd + madd;');
		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		vert.write('texCoord.y = 1.0 - texCoord.y;');
		#end
		vert.write('gl_Position = vec4(pos.xy, 0.0, 1.0);');

		frag.add_uniform('vec4 inp', '_inputBrush');
		frag.add_uniform('vec4 inplast', '_inputBrushLast');

		frag.add_uniform('sampler2D gbufferD');

		frag.add_out('vec4 fragColor[2]');

		frag.add_uniform('float brushRadius', '_brushRadius');
		frag.add_uniform('float brushOpacity', '_brushOpacity');
		frag.add_uniform('float brushHardness', '_brushHardness');

		if (Context.raw.tool == ToolBrush  ||
			Context.raw.tool == ToolEraser ||
			Context.raw.tool == ToolClone  ||
			Context.raw.tool == ToolBlur   ||
			Context.raw.tool == ToolSmudge   ||
			Context.raw.tool == ToolParticle ||
			decal) {

			var depthReject = !Context.raw.xray;

			MakeBrush.run(vert, frag);
		}

		frag.write('vec3 basecol = vec3(1.0, 1.0, 1.0);');
		frag.write('float opacity = 1.0;');
		frag.write('if (opacity == 0.0) discard;');

		frag.write('float str = clamp((brushRadius - dist) * brushHardness * 400.0, 0.0, 1.0) * opacity;');

		frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
		frag.write('vec4 sample_undo = textureLod(texpaint_undo, texCoord, 0.0);');

		frag.write('if (sample_undo.r == 0 && sample_undo.g == 0 && sample_undo.b == 0) discard;');

		frag.add_function(ShaderFunctions.str_octahedronWrap);
		frag.add_uniform('sampler2D gbuffer0_undo');
		frag.write('vec2 g0_undo = textureLod(gbuffer0_undo, inp.xy, 0.0).rg;');
		frag.write('vec3 wn;');
		frag.write('wn.z = 1.0 - abs(g0_undo.x) - abs(g0_undo.y);');
		frag.write('wn.xy = wn.z >= 0.0 ? g0_undo.xy : octahedronWrap(g0_undo.xy);');
		frag.write('vec3 n = normalize(wn);');

		frag.write('fragColor[0] = vec4(sample_undo.rgb + n * 0.1 * str, 1.0);');

		frag.write('fragColor[1] = vec4(str, 0.0, 0.0, 1.0);');

		MaterialParser.finalize(con_paint);
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = vert.get();
		con_paint.data.fragment_shader = frag.get();

		return con_paint;
	}
}
