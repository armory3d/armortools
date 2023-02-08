package arm.node;

import arm.shader.ShaderFunctions;
import arm.shader.NodeShader;
import arm.Enums;

class MakeColorIdPicker {

	public static function run(vert: NodeShader, frag: NodeShader) {
		// Mangle vertices to form full screen triangle
		vert.write('gl_Position = vec4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);');

		frag.add_uniform('sampler2D gbuffer2');
		frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
		frag.add_uniform('vec4 inp', '_inputBrush');
		frag.write('vec4 inpLocal = inp;'); // TODO: spirv workaround
		frag.write('vec2 gbufferSizeLocal = gbufferSize;'); // TODO: spirv workaround

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inpLocal.x * gbufferSizeLocal.x, inpLocal.y * gbufferSizeLocal.y), 0).ba;');
		#else
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inpLocal.x * gbufferSizeLocal.x, (1.0 - inpLocal.y) * gbufferSizeLocal.y), 0).ba;');
		#end

		if (Context.tool == ToolColorId) {
			frag.add_out('vec4 fragColor');
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.write('vec3 idcol = textureLod(texcolorid, texCoordInp, 0.0).rgb;');
			frag.write('fragColor = vec4(idcol, 1.0);');
		}
		else if (Context.tool == ToolPicker) {
			if (Context.pickPosNorTex) {
				frag.add_out('vec4 fragColor[2]');
				frag.add_uniform('sampler2D gbufferD');
				frag.add_uniform('mat4 invVP', '_inverseViewProjectionMatrix');
				frag.add_function(ShaderFunctions.str_get_pos_from_depth);
				frag.add_function(ShaderFunctions.str_get_nor_from_depth);
				frag.write('fragColor[0] = vec4(get_pos_from_depth(vec2(inpLocal.x, 1.0 - inpLocal.y), invVP, texturePass(gbufferD)), texCoordInp.x);');
				frag.write('fragColor[1] = vec4(get_nor_from_depth(fragColor[0].rgb, vec2(inpLocal.x, 1.0 - inpLocal.y), invVP, vec2(1.0, 1.0) / gbufferSize, texturePass(gbufferD)), texCoordInp.y);');
			}
			else {
				frag.add_out('vec4 fragColor[4]');
				frag.add_uniform('sampler2D texpaint');
				frag.add_uniform('sampler2D texpaint_nor');
				frag.add_uniform('sampler2D texpaint_pack');
				frag.write('fragColor[0] = textureLod(texpaint, texCoordInp, 0.0);');
				frag.write('fragColor[1] = textureLod(texpaint_nor, texCoordInp, 0.0);');
				frag.write('fragColor[2] = textureLod(texpaint_pack, texCoordInp, 0.0);');
				frag.write('fragColor[3].rg = texCoordInp.xy;');
			}
		}
	}
}
