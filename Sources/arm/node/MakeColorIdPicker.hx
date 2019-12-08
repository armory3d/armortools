package arm.node;

import arm.node.MaterialShader;
import arm.Tool;

class MakeColorIdPicker {

	public static function run(vert: MaterialShader, frag: MaterialShader) {
		// Mangle vertices to form full screen triangle
		vert.write('gl_Position = vec4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);');

		frag.add_uniform('sampler2D gbuffer2');
		frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
		frag.add_uniform('vec4 inp', '_inputBrush');

		#if (kha_opengl || kha_webgl)
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
		#else
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
		#end

		if (Context.tool == ToolColorId) {
			frag.add_out('vec4 fragColor');
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.write('vec3 idcol = textureLod(texcolorid, texCoordInp, 0.0).rgb;');
			frag.write('fragColor = vec4(idcol, 1.0);');
		}
		else if (Context.tool == ToolPicker) {
			frag.add_out('vec4 fragColor[3]');
			frag.add_uniform('sampler2D texpaint');
			frag.add_uniform('sampler2D texpaint_nor');
			frag.add_uniform('sampler2D texpaint_pack');
			frag.write('fragColor[0] = textureLod(texpaint, texCoordInp, 0.0);');
			frag.write('fragColor[1] = textureLod(texpaint_nor, texCoordInp, 0.0);');
			frag.write('fragColor[2] = textureLod(texpaint_pack, texCoordInp, 0.0);');
			frag.write('fragColor[0].a = texCoordInp.x;');
			frag.write('fragColor[2].a = texCoordInp.y;');
		}
	}
}
