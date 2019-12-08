package arm.node;

import arm.ui.UITrait;
import arm.node.MaterialShader;

class MakeDiscard {

	public static function colorId(vert: MaterialShader, frag: MaterialShader) {
		vert.add_out('vec2 texCoordPick');
		vert.write('texCoordPick = fract(subtex);');
		frag.add_uniform('sampler2D texpaint_colorid'); // 1x1 picker
		frag.add_uniform('sampler2D texcolorid', '_texcolorid'); // color map
		frag.add_uniform('vec2 texcoloridSize', '_texcoloridSize'); // color map
		frag.write('vec3 c1 = texelFetch(texpaint_colorid, ivec2(0, 0), 0).rgb;');
		frag.write('vec3 c2 = texelFetch(texcolorid, ivec2(texCoordPick * texcoloridSize), 0).rgb;');
		frag.write('if (any(c1 != c2)) discard;');
	}

	public static function face(vert: MaterialShader, frag: MaterialShader) {
		vert.add_out('vec2 texCoordPick');
		vert.write('texCoordPick = fract(subtex);');
		frag.add_uniform('sampler2D gbuffer2');
		frag.add_uniform('sampler2D textrianglemap', '_textrianglemap'); // triangle map
		frag.add_uniform('float textrianglemapSize', '_texpaintSize');
		frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
		#if (kha_opengl || kha_webgl)
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
		#else
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
		#end
		frag.write('vec4 c1 = texelFetch(textrianglemap, ivec2(texCoordInp * textrianglemapSize), 0);');
		frag.write('vec4 c2 = texelFetch(textrianglemap, ivec2(texCoordPick * textrianglemapSize), 0);');
		frag.write('if (any(c1 != c2)) discard;');
	}

	public static function materialId(vert: MaterialShader, frag: MaterialShader) {
		frag.wvpposition = true;
		frag.write('vec2 picker_sample_tc = vec2(wvpposition.x / wvpposition.w, wvpposition.y / wvpposition.w) * 0.5 + 0.5;');
		#if (kha_direct3d11 || kha_direct3d12)
		frag.write('picker_sample_tc.y = 1.0 - picker_sample_tc.y;');
		#end
		frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
		var matid = UITrait.inst.materialIdPicked / 255;
		frag.write('if ($matid != textureLod(texpaint_nor_undo, picker_sample_tc, 0.0).a) discard;');
	}
}
