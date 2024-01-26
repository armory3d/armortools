
class MakeClone {

	static run = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		NodeShader.add_uniform(frag, 'vec2 cloneDelta', '_cloneDelta');
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp.xy + cloneDelta) * gbufferSize), 0).ba;');
		///else
		NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp.x + cloneDelta.x) * gbufferSize.x, (1.0 - (sp.y + cloneDelta.y)) * gbufferSize.y), 0).ba;');
		///end

		NodeShader.write(frag, 'vec3 texpaint_pack_sample = textureLod(texpaint_pack_undo, texCoordInp, 0.0).rgb;');
		let base = 'textureLod(texpaint_undo, texCoordInp, 0.0).rgb';
		let rough = 'texpaint_pack_sample.g';
		let met = 'texpaint_pack_sample.b';
		let occ = 'texpaint_pack_sample.r';
		let nortan = 'textureLod(texpaint_nor_undo, texCoordInp, 0.0).rgb';
		let height = '0.0';
		let opac = '1.0';
		NodeShader.write(frag, `vec3 basecol = ${base};`);
		NodeShader.write(frag, `float roughness = ${rough};`);
		NodeShader.write(frag, `float metallic = ${met};`);
		NodeShader.write(frag, `float occlusion = ${occ};`);
		NodeShader.write(frag, `vec3 nortan = ${nortan};`);
		NodeShader.write(frag, `float height = ${height};`);
		NodeShader.write(frag, `float mat_opacity = ${opac};`);
		NodeShader.write(frag, 'float opacity = mat_opacity * brushOpacity;');
		if (Context.raw.material.paintEmis) {
			NodeShader.write(frag, 'float emis = 0.0;');
		}
		if (Context.raw.material.paintSubs) {
			NodeShader.write(frag, 'float subs = 0.0;');
		}
	}
}
