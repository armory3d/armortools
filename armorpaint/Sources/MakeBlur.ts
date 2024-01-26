
class MakeBlur {

	static run = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, sp.y * gbufferSize.y), 0).ba;');
		///else
		NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, (1.0 - sp.y) * gbufferSize.y), 0).ba;');
		///end

		NodeShader.write(frag, 'vec3 basecol = vec3(0.0, 0.0, 0.0);');
		NodeShader.write(frag, 'float roughness = 0.0;');
		NodeShader.write(frag, 'float metallic = 0.0;');
		NodeShader.write(frag, 'float occlusion = 0.0;');
		NodeShader.write(frag, 'vec3 nortan = vec3(0.0, 0.0, 0.0);');
		NodeShader.write(frag, 'float height = 0.0;');
		NodeShader.write(frag, 'float mat_opacity = 1.0;');
		let isMask = SlotLayer.isMask(Context.raw.layer);
		if (isMask) {
			NodeShader.write(frag, 'float opacity = 1.0;');
		}
		else {
			NodeShader.write(frag, 'float opacity = 0.0;');
		}
		if (Context.raw.material.paintEmis) {
			NodeShader.write(frag, 'float emis = 0.0;');
		}
		if (Context.raw.material.paintSubs) {
			NodeShader.write(frag, 'float subs = 0.0;');
		}

		NodeShader.add_uniform(frag, 'vec2 texpaintSize', '_texpaintSize');
		NodeShader.write(frag, 'float blur_step = 1.0 / texpaintSize.x;');
		if (Context.raw.tool == WorkspaceTool.ToolSmudge) {
			///if (krom_direct3d11 || krom_direct3d12 || krom_metal)
			NodeShader.write(frag, 'const float blur_weight[7] = {1.0 / 28.0, 2.0 / 28.0, 3.0 / 28.0, 4.0 / 28.0, 5.0 / 28.0, 6.0 / 28.0, 7.0 / 28.0};');
			///else
			NodeShader.write(frag, 'const float blur_weight[7] = float[](1.0 / 28.0, 2.0 / 28.0, 3.0 / 28.0, 4.0 / 28.0, 5.0 / 28.0, 6.0 / 28.0, 7.0 / 28.0);');
			///end
			NodeShader.add_uniform(frag, 'vec3 brushDirection', '_brushDirection');
			NodeShader.write(frag, 'vec2 blur_direction = brushDirection.yx;');
			NodeShader.write(frag, 'for (int i = 0; i < 7; ++i) {');
			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(frag, 'vec2 texCoordInp2 = texelFetch(gbuffer2, ivec2((sp.x + blur_direction.x * blur_step * float(i)) * gbufferSize.x, (sp.y + blur_direction.y * blur_step * float(i)) * gbufferSize.y), 0).ba;');
			///else
			NodeShader.write(frag, 'vec2 texCoordInp2 = texelFetch(gbuffer2, ivec2((sp.x + blur_direction.x * blur_step * float(i)) * gbufferSize.x, (1.0 - (sp.y + blur_direction.y * blur_step * float(i))) * gbufferSize.y), 0).ba;');
			///end
			NodeShader.write(frag, 'vec4 texpaint_sample = texture(texpaint_undo, texCoordInp2);');
			NodeShader.write(frag, 'opacity += texpaint_sample.a * blur_weight[i];');
			NodeShader.write(frag, 'basecol += texpaint_sample.rgb * blur_weight[i];');
			NodeShader.write(frag, 'vec4 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp2) * blur_weight[i];');
			NodeShader.write(frag, 'roughness += texpaint_pack_sample.g;');
			NodeShader.write(frag, 'metallic += texpaint_pack_sample.b;');
			NodeShader.write(frag, 'occlusion += texpaint_pack_sample.r;');
			NodeShader.write(frag, 'height += texpaint_pack_sample.a;');
			NodeShader.write(frag, 'nortan += texture(texpaint_nor_undo, texCoordInp2).rgb * blur_weight[i];');
			NodeShader.write(frag, '}');
		}
		else {
			///if (krom_direct3d11 || krom_direct3d12 || krom_metal)
			NodeShader.write(frag, 'const float blur_weight[15] = {0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0};');
			///else
			NodeShader.write(frag, 'const float blur_weight[15] = float[](0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0);');
			///end
			// X
			NodeShader.write(frag, 'for (int i = -7; i <= 7; ++i) {');
			NodeShader.write(frag, 'vec4 texpaint_sample = texture(texpaint_undo, texCoordInp + vec2(blur_step * float(i), 0.0));');
			NodeShader.write(frag, 'opacity += texpaint_sample.a * blur_weight[i + 7];');
			NodeShader.write(frag, 'basecol += texpaint_sample.rgb * blur_weight[i + 7];');
			NodeShader.write(frag, 'vec4 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(blur_step * float(i), 0.0)) * blur_weight[i + 7];');
			NodeShader.write(frag, 'roughness += texpaint_pack_sample.g;');
			NodeShader.write(frag, 'metallic += texpaint_pack_sample.b;');
			NodeShader.write(frag, 'occlusion += texpaint_pack_sample.r;');
			NodeShader.write(frag, 'height += texpaint_pack_sample.a;');
			NodeShader.write(frag, 'nortan += texture(texpaint_nor_undo, texCoordInp + vec2(blur_step * float(i), 0.0)).rgb * blur_weight[i + 7];');
			NodeShader.write(frag, '}');
			// Y
			NodeShader.write(frag, 'for (int j = -7; j <= 7; ++j) {');
			NodeShader.write(frag, 'vec4 texpaint_sample = texture(texpaint_undo, texCoordInp + vec2(0.0, blur_step * float(j)));');
			NodeShader.write(frag, 'opacity += texpaint_sample.a * blur_weight[j + 7];');
			NodeShader.write(frag, 'basecol += texpaint_sample.rgb * blur_weight[j + 7];');
			NodeShader.write(frag, 'vec4 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(0.0, blur_step * float(j))) * blur_weight[j + 7];');
			NodeShader.write(frag, 'roughness += texpaint_pack_sample.g;');
			NodeShader.write(frag, 'metallic += texpaint_pack_sample.b;');
			NodeShader.write(frag, 'occlusion += texpaint_pack_sample.r;');
			NodeShader.write(frag, 'height += texpaint_pack_sample.a;');
			NodeShader.write(frag, 'nortan += texture(texpaint_nor_undo, texCoordInp + vec2(0.0, blur_step * float(j))).rgb * blur_weight[j + 7];');
			NodeShader.write(frag, '}');
		}
		NodeShader.write(frag, 'opacity *= brushOpacity;');
	}
}
