package arm.node;

import arm.ui.UISidebar;
import arm.shader.NodeShader;

class MakeDiscard {

	public static function colorId(vert: NodeShader, frag: NodeShader) {
		frag.add_uniform('sampler2D texpaint_colorid'); // 1x1 picker
		frag.add_uniform('sampler2D texcolorid', '_texcolorid'); // color map
		frag.write('vec3 colorid_c1 = texelFetch(texpaint_colorid, ivec2(0, 0), 0).rgb;');
		frag.write('vec3 colorid_c2 = textureLod(texcolorid, texCoordPick, 0).rgb;');
		#if (kha_direct3d11 || kha_direct3d12 || kha_metal)
		frag.write('if (any(colorid_c1 != colorid_c2)) discard;');
		#else
		frag.write('if (colorid_c1 != colorid_c2) discard;');
		#end
	}

	public static function face(vert: NodeShader, frag: NodeShader) {
		frag.add_uniform('sampler2D gbuffer2');
		frag.add_uniform('sampler2D textrianglemap', '_textrianglemap');
		frag.add_uniform('vec2 textrianglemapSize', '_texpaintSize');
		frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
		#else
		frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
		#end
		frag.write('vec4 face_c1 = texelFetch(textrianglemap, ivec2(texCoordInp * textrianglemapSize), 0);');
		frag.write('vec4 face_c2 = textureLod(textrianglemap, texCoordPick, 0);');
		#if (kha_direct3d11 || kha_direct3d12 || kha_metal)
		frag.write('if (any(face_c1 != face_c2)) discard;');
		#else
		frag.write('if (face_c1 != face_c2) discard;');
		#end
	}

	public static function uvIsland(vert: NodeShader, frag: NodeShader) {
		frag.add_uniform('sampler2D texuvislandmap', '_texuvislandmap');
		frag.write('if (textureLod(texuvislandmap, texCoordPick, 0).r == 0.0) discard;');
	}

	public static function materialId(vert: NodeShader, frag: NodeShader) {
		frag.wvpposition = true;
		frag.write('vec2 picker_sample_tc = vec2(wvpposition.x / wvpposition.w, wvpposition.y / wvpposition.w) * 0.5 + 0.5;');
		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		frag.write('picker_sample_tc.y = 1.0 - picker_sample_tc.y;');
		#end
		frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
		var matid = Context.materialIdPicked / 255;
		frag.write('if ($matid != textureLod(texpaint_nor_undo, picker_sample_tc, 0.0).a) discard;');
	}
}
