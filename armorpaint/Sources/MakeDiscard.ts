
class MakeDiscard {

	static colorId = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		NodeShader.add_uniform(frag, 'sampler2D texpaint_colorid'); // 1x1 picker
		NodeShader.add_uniform(frag, 'sampler2D texcolorid', '_texcolorid'); // color map
		NodeShader.write(frag, 'vec3 colorid_c1 = texelFetch(texpaint_colorid, ivec2(0, 0), 0).rgb;');
		NodeShader.write(frag, 'vec3 colorid_c2 = textureLod(texcolorid, texCoordPick, 0).rgb;');
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal)
		NodeShader.write(frag, 'if (any(colorid_c1 != colorid_c2)) discard;');
		///else
		NodeShader.write(frag, 'if (colorid_c1 != colorid_c2) discard;');
		///end
	}

	static face = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		NodeShader.add_uniform(frag, 'sampler2D gbuffer2');
		NodeShader.add_uniform(frag, 'sampler2D textrianglemap', '_textrianglemap');
		NodeShader.add_uniform(frag, 'vec2 textrianglemapSize', '_texpaintSize');
		NodeShader.add_uniform(frag, 'vec2 gbufferSize', '_gbufferSize');
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
		///else
		NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
		///end
		NodeShader.write(frag, 'vec4 face_c1 = texelFetch(textrianglemap, ivec2(texCoordInp * textrianglemapSize), 0);');
		NodeShader.write(frag, 'vec4 face_c2 = textureLod(textrianglemap, texCoordPick, 0);');
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal)
		NodeShader.write(frag, 'if (any(face_c1 != face_c2)) discard;');
		///else
		NodeShader.write(frag, 'if (face_c1 != face_c2) discard;');
		///end
	}

	static uvIsland = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		NodeShader.add_uniform(frag, 'sampler2D texuvislandmap', '_texuvislandmap');
		NodeShader.write(frag, 'if (textureLod(texuvislandmap, texCoordPick, 0).r == 0.0) discard;');
	}

	static materialId = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		frag.wvpposition = true;
		NodeShader.write(frag, 'vec2 picker_sample_tc = vec2(wvpposition.x / wvpposition.w, wvpposition.y / wvpposition.w) * 0.5 + 0.5;');
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(frag, 'picker_sample_tc.y = 1.0 - picker_sample_tc.y;');
		///end
		NodeShader.add_uniform(frag, 'sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
		let matid = Context.raw.materialIdPicked / 255;
		NodeShader.write(frag, `if (${matid} != textureLod(texpaint_nor_undo, picker_sample_tc, 0.0).a) discard;`);
	}
}
