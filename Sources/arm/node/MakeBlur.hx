package arm.node;

import arm.node.MaterialShader;

class MakeBlur {

	public static function run(vert: MaterialShader, frag: MaterialShader) {
		#if (kha_opengl || kha_webgl)
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, (1.0 - sp.y) * gbufferSize.y), 0).ba;');
		#else
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, sp.y * gbufferSize.y), 0).ba;');
		#end

		frag.write('vec3 basecol = vec3(0.0, 0.0, 0.0);');
		frag.write('float roughness = 0.0;');
		frag.write('float metallic = 0.0;');
		frag.write('float occlusion = 0.0;');
		frag.write('vec3 nortan = vec3(0.0, 0.0, 0.0);');
		frag.write('float height = 0.0;');
		frag.write('float opacity = 1.0 * brushOpacity;');
		if (Context.material.paintEmis) {
			frag.write('float emis = 0.0;');
		}
		if (Context.material.paintSubs) {
			frag.write('float subs = 0.0;');
		}
		#if (kha_direct3d11 || kha_direct3d12)
		frag.write('const float blur_weight[15] = {0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0};');
		#else
		frag.write('const float blur_weight[15] = float[](0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0);');
		#end
		frag.add_uniform('float texpaintSize', '_texpaintSize');
		frag.write('float blur_step = 1.0 / texpaintSize;');
		frag.write('for (int i = -7; i <= 7; i++) {');
		frag.write('basecol += texture(texpaint_undo, texCoordInp + vec2(blur_step * i, 0.0)).rgb * blur_weight[i + 7];');
		frag.write('vec3 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(blur_step * i, 0.0)).rgb * blur_weight[i + 7];');
		frag.write('roughness += texpaint_pack_sample.g;');
		frag.write('metallic += texpaint_pack_sample.b;');
		frag.write('occlusion += texpaint_pack_sample.r;');
		frag.write('nortan += texture(texpaint_nor_undo, texCoordInp + vec2(blur_step * i, 0.0)).rgb * blur_weight[i + 7];');
		frag.write('}');
		frag.write('for (int i = -7; i <= 7; i++) {');
		frag.write('basecol += texture(texpaint_undo, texCoordInp + vec2(0.0, blur_step * i)).rgb * blur_weight[i + 7];');
		frag.write('vec3 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(0.0, blur_step * i)).rgb * blur_weight[i + 7];');
		frag.write('roughness += texpaint_pack_sample.g;');
		frag.write('metallic += texpaint_pack_sample.b;');
		frag.write('occlusion += texpaint_pack_sample.r;');
		frag.write('nortan += texture(texpaint_nor_undo, texCoordInp + vec2(0.0, blur_step * i)).rgb * blur_weight[i + 7];');
		frag.write('}');
	}
}
