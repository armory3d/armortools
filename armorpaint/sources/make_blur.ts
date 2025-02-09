
function make_blur_run(vert: node_shader_t, frag: node_shader_t) {
	node_shader_write(frag, "vec2 tex_coord_inp = texelFetch(gbuffer2, ivec2(sp.x * gbuffer_size.x, sp.y * gbuffer_size.y), 0).ba;");

	node_shader_write(frag, "vec3 basecol = vec3(0.0, 0.0, 0.0);");
	node_shader_write(frag, "float roughness = 0.0;");
	node_shader_write(frag, "float metallic = 0.0;");
	node_shader_write(frag, "float occlusion = 0.0;");
	node_shader_write(frag, "vec3 nortan = vec3(0.0, 0.0, 0.0);");
	node_shader_write(frag, "float height = 0.0;");
	node_shader_write(frag, "float mat_opacity = 1.0;");
	let is_mask: bool = slot_layer_is_mask(context_raw.layer);
	if (is_mask) {
		node_shader_write(frag, "float opacity = 1.0;");
	}
	else {
		node_shader_write(frag, "float opacity = 0.0;");
	}
	if (context_raw.material.paint_emis) {
		node_shader_write(frag, "float emis = 0.0;");
	}
	if (context_raw.material.paint_subs) {
		node_shader_write(frag, "float subs = 0.0;");
	}

	node_shader_add_uniform(frag, "vec2 texpaint_size", "_texpaint_size");
	node_shader_write(frag, "float blur_step = 1.0 / texpaint_size.x;");
	if (context_raw.tool == workspace_tool_t.SMUDGE) {
		///if (arm_direct3d12 || arm_metal)
		node_shader_write(frag, "const float blur_weight[7] = {1.0 / 28.0, 2.0 / 28.0, 3.0 / 28.0, 4.0 / 28.0, 5.0 / 28.0, 6.0 / 28.0, 7.0 / 28.0};");
		///else
		node_shader_write(frag, "const float blur_weight[7] = float[](1.0 / 28.0, 2.0 / 28.0, 3.0 / 28.0, 4.0 / 28.0, 5.0 / 28.0, 6.0 / 28.0, 7.0 / 28.0);");
		///end
		node_shader_add_uniform(frag, "vec3 brush_direction", "_brush_direction");
		node_shader_write(frag, "vec2 blur_direction = brush_direction.yx;");
		node_shader_write(frag, "for (int i = 0; i < 7; ++i) {");
		node_shader_write(frag, "vec2 tex_coord_inp2 = texelFetch(gbuffer2, ivec2((sp.x + blur_direction.x * blur_step * float(i)) * gbuffer_size.x, (sp.y + blur_direction.y * blur_step * float(i)) * gbuffer_size.y), 0).ba;");
		node_shader_write(frag, "vec4 texpaint_sample = texture(texpaint_undo, tex_coord_inp2);");
		node_shader_write(frag, "opacity += texpaint_sample.a * blur_weight[i];");
		node_shader_write(frag, "basecol += texpaint_sample.rgb * blur_weight[i];");
		node_shader_write(frag, "vec4 texpaint_pack_sample = texture(texpaint_pack_undo, tex_coord_inp2) * blur_weight[i];");
		node_shader_write(frag, "roughness += texpaint_pack_sample.g;");
		node_shader_write(frag, "metallic += texpaint_pack_sample.b;");
		node_shader_write(frag, "occlusion += texpaint_pack_sample.r;");
		node_shader_write(frag, "height += texpaint_pack_sample.a;");
		node_shader_write(frag, "nortan += texture(texpaint_nor_undo, tex_coord_inp2).rgb * blur_weight[i];");
		node_shader_write(frag, "}");
	}
	else {
		///if (arm_direct3d12 || arm_metal)
		node_shader_write(frag, "const float blur_weight[15] = {0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0};");
		///else
		node_shader_write(frag, "const float blur_weight[15] = float[](0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0);");
		///end
		// X
		node_shader_write(frag, "for (int i = -7; i <= 7; ++i) {");
		node_shader_write(frag, "vec4 texpaint_sample = texture(texpaint_undo, tex_coord_inp + vec2(blur_step * float(i), 0.0));");
		node_shader_write(frag, "opacity += texpaint_sample.a * blur_weight[i + 7];");
		node_shader_write(frag, "basecol += texpaint_sample.rgb * blur_weight[i + 7];");
		node_shader_write(frag, "vec4 texpaint_pack_sample = texture(texpaint_pack_undo, tex_coord_inp + vec2(blur_step * float(i), 0.0)) * blur_weight[i + 7];");
		node_shader_write(frag, "roughness += texpaint_pack_sample.g;");
		node_shader_write(frag, "metallic += texpaint_pack_sample.b;");
		node_shader_write(frag, "occlusion += texpaint_pack_sample.r;");
		node_shader_write(frag, "height += texpaint_pack_sample.a;");
		node_shader_write(frag, "nortan += texture(texpaint_nor_undo, tex_coord_inp + vec2(blur_step * float(i), 0.0)).rgb * blur_weight[i + 7];");
		node_shader_write(frag, "}");
		// Y
		node_shader_write(frag, "for (int j = -7; j <= 7; ++j) {");
		node_shader_write(frag, "vec4 texpaint_sample = texture(texpaint_undo, tex_coord_inp + vec2(0.0, blur_step * float(j)));");
		node_shader_write(frag, "opacity += texpaint_sample.a * blur_weight[j + 7];");
		node_shader_write(frag, "basecol += texpaint_sample.rgb * blur_weight[j + 7];");
		node_shader_write(frag, "vec4 texpaint_pack_sample = texture(texpaint_pack_undo, tex_coord_inp + vec2(0.0, blur_step * float(j))) * blur_weight[j + 7];");
		node_shader_write(frag, "roughness += texpaint_pack_sample.g;");
		node_shader_write(frag, "metallic += texpaint_pack_sample.b;");
		node_shader_write(frag, "occlusion += texpaint_pack_sample.r;");
		node_shader_write(frag, "height += texpaint_pack_sample.a;");
		node_shader_write(frag, "nortan += texture(texpaint_nor_undo, tex_coord_inp + vec2(0.0, blur_step * float(j))).rgb * blur_weight[j + 7];");
		node_shader_write(frag, "}");
	}
	node_shader_write(frag, "opacity *= brush_opacity;");
}
