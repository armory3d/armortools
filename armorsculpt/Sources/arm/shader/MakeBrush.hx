package arm.shader;

import arm.shader.NodeShader;

class MakeBrush {

	public static function run(vert: NodeShader, frag: NodeShader) {

		frag.write('float dist = 0.0;');

		if (Config.raw.brush_3d) {
			frag.write('vec4 inpLocal = inp;'); // TODO: spirv workaround
			frag.write('vec4 inplastLocal = inplast;'); // TODO: spirv workaround
			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('float depth = textureLod(gbufferD, inpLocal.xy, 0.0).r;');
			#else
			frag.write('float depth = textureLod(gbufferD, vec2(inpLocal.x, 1.0 - inpLocal.y), 0.0).r;');
			#end

			frag.add_uniform('mat4 invVP', '_inverseViewProjectionMatrix');
			frag.write('vec4 winp = vec4(vec2(inpLocal.x, 1.0 - inpLocal.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);');
			frag.write('winp = mul(winp, invVP);');
			frag.write('winp.xyz /= winp.w;');

			frag.add_uniform('mat4 W', '_worldMatrix');

			frag.write_attrib('vec3 wposition = mul(texelFetch(texpaint_undo, ivec2(texCoord.x * textureSize(texpaint_undo, 0).x, texCoord.y * textureSize(texpaint_undo, 0).y), 0), W).xyz;');

			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('float depthlast = textureLod(gbufferD, inplastLocal.xy, 0.0).r;');
			#else
			frag.write('float depthlast = textureLod(gbufferD, vec2(inplastLocal.x, 1.0 - inplastLocal.y), 0.0).r;');
			#end

			frag.write('vec4 winplast = vec4(vec2(inplastLocal.x, 1.0 - inplastLocal.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);');
			frag.write('winplast = mul(winplast, invVP);');
			frag.write('winplast.xyz /= winplast.w;');

			frag.write('dist = distance(wposition, winp.xyz);');
		}

		frag.write('if (dist > brushRadius) discard;');
	}
}
