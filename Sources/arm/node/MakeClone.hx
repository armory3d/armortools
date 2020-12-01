package arm.node;

import arm.shader.NodeShader;

class MakeClone {

	public static function run(vert: NodeShader, frag: NodeShader) {
		frag.add_uniform('vec2 cloneDelta', '_cloneDelta');
		frag.write('vec2 cloneDeltaLocal = cloneDelta;'); // TODO: spirv workaround
		frag.write('vec2 gbufferSizeLocal = gbufferSize;'); // TODO: spirv workaround
		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp + cloneDeltaLocal) * gbufferSizeLocal), 0).ba;');
		#else
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp.x + cloneDeltaLocal.x) * gbufferSizeLocal.x, (1.0 - (sp.y + cloneDeltaLocal.y)) * gbufferSizeLocal.y), 0).ba;');
		#end

		frag.write('vec3 texpaint_pack_sample = textureLod(texpaint_pack_undo, texCoordInp, 0.0).rgb;');
		var base = 'textureLod(texpaint_undo, texCoordInp, 0.0).rgb';
		var rough = 'texpaint_pack_sample.g';
		var met = 'texpaint_pack_sample.b';
		var occ = 'texpaint_pack_sample.r';
		var nortan = 'textureLod(texpaint_nor_undo, texCoordInp, 0.0).rgb';
		var height = '0.0';
		var opac = '1.0';
		frag.write('vec3 basecol = $base;');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');
		frag.write('vec3 nortan = $nortan;');
		frag.write('float height = $height;');
		frag.write('float mat_opacity = $opac;');
		frag.write('float opacity = mat_opacity * brushOpacity;');
		if (Context.material.paintEmis) {
			frag.write('float emis = 0.0;');
		}
		if (Context.material.paintSubs) {
			frag.write('float subs = 0.0;');
		}
	}
}
