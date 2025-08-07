
function make_blur_run(kong: node_shader_t) {
	// node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[uint2(sp.x * constants.gbuffer_size.x, sp.y * constants.gbuffer_size.y)].ba;");
	node_shader_write_frag(kong, "var tex_coord_inp4: float4 = gbuffer2[uint2(uint(sp.x * constants.gbuffer_size.x), uint(sp.y * constants.gbuffer_size.y))];");
	node_shader_write_frag(kong, "var tex_coord_inp: float2 = tex_coord_inp4.ba;");

	node_shader_write_frag(kong, "var basecol: float3 = float3(0.0, 0.0, 0.0);");
	node_shader_write_frag(kong, "var roughness: float = 0.0;");
	node_shader_write_frag(kong, "var metallic: float = 0.0;");
	node_shader_write_frag(kong, "var occlusion: float = 0.0;");
	node_shader_write_frag(kong, "var nortan: float3 = float3(0.0, 0.0, 0.0);");
	node_shader_write_frag(kong, "var height: float = 0.0;");
	node_shader_write_frag(kong, "var mat_opacity: float = 1.0;");
	let is_mask: bool = slot_layer_is_mask(context_raw.layer);
	if (is_mask) {
		node_shader_write_frag(kong, "var opacity: float = 1.0;");
	}
	else {
		node_shader_write_frag(kong, "var opacity: float = 0.0;");
	}
	if (context_raw.material.paint_emis) {
		node_shader_write_frag(kong, "var emis: float = 0.0;");
	}
	if (context_raw.material.paint_subs) {
		node_shader_write_frag(kong, "var subs: float = 0.0;");
	}

	node_shader_add_constant(kong, "texpaint_size: float2", "_texpaint_size");
	node_shader_write_frag(kong, "var blur_step: float = 1.0 / constants.texpaint_size.x;");
	if (context_raw.tool == tool_type_t.SMUDGE) {
		// node_shader_write_frag(kong, "const blur_weight: float[7] = {1.0 / 28.0, 2.0 / 28.0, 3.0 / 28.0, 4.0 / 28.0, 5.0 / 28.0, 6.0 / 28.0, 7.0 / 28.0};");
		node_shader_add_function(kong, str_get_smudge_tool_weight);
		node_shader_add_constant(kong, "brush_direction: float3", "_brush_direction");
		node_shader_write_frag(kong, "var blur_direction: float2 = constants.brush_direction.yx;");
		node_shader_write_frag(kong, "for (var i: int = 0; i < 7; i += 1) {");
		// node_shader_write_frag(kong, "var tex_coord_inp2: float2 = gbuffer2[uint2((sp.x + blur_direction.x * blur_step * float(i)) * constants.gbuffer_size.x, (sp.y + blur_direction.y * blur_step * float(i)) * constants.gbuffer_size.y)].ba;");
		node_shader_write_frag(kong, "var tex_coord_inp24: float4 = gbuffer2[uint2(uint((sp.x + blur_direction.x * blur_step * float(i)) * constants.gbuffer_size.x), uint((sp.y + blur_direction.y * blur_step * float(i)) * constants.gbuffer_size.y))];");
		node_shader_write_frag(kong, "var tex_coord_inp2: float2 = tex_coord_inp24.ba;");
		node_shader_write_frag(kong, "var texpaint_sample: float4 = sample(texpaint_undo, sampler_linear, tex_coord_inp2);");
		node_shader_write_frag(kong, "var blur_weight_i: float = get_smudge_tool_weight(i);");
		node_shader_write_frag(kong, "opacity += texpaint_sample.a * blur_weight_i;");
		node_shader_write_frag(kong, "basecol += texpaint_sample.rgb * blur_weight_i;");
		node_shader_write_frag(kong, "var texpaint_pack_sample: float4 = sample(texpaint_pack_undo, sampler_linear, tex_coord_inp2) * blur_weight_i;");
		node_shader_write_frag(kong, "roughness += texpaint_pack_sample.g;");
		node_shader_write_frag(kong, "metallic += texpaint_pack_sample.b;");
		node_shader_write_frag(kong, "occlusion += texpaint_pack_sample.r;");
		node_shader_write_frag(kong, "height += texpaint_pack_sample.a;");
		node_shader_write_frag(kong, "nortan += sample(texpaint_nor_undo, sampler_linear, tex_coord_inp2).rgb * blur_weight_i;");
		node_shader_write_frag(kong, "}");
	}
	else {
		// node_shader_write_frag(kong, "const blur_weight: float[15] = {0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0};");
		node_shader_add_function(kong, str_get_blur_tool_weight);
		// X
		node_shader_write_frag(kong, "for (var i: int = 0; i <= 14; i += 1) {");
		node_shader_write_frag(kong, "var texpaint_sample: float4 = sample(texpaint_undo, sampler_linear, tex_coord_inp + float2(blur_step * float(i - 7), 0.0));");
		node_shader_write_frag(kong, "var blur_weight_i: float = get_blur_tool_weight(i);");
		node_shader_write_frag(kong, "opacity += texpaint_sample.a * blur_weight_i;");
		node_shader_write_frag(kong, "basecol += texpaint_sample.rgb * blur_weight_i;");
		node_shader_write_frag(kong, "var texpaint_pack_sample: float4 = sample(texpaint_pack_undo, sampler_linear, tex_coord_inp + float2(blur_step * float(i - 7), 0.0)) * blur_weight_i;");
		node_shader_write_frag(kong, "roughness += texpaint_pack_sample.g;");
		node_shader_write_frag(kong, "metallic += texpaint_pack_sample.b;");
		node_shader_write_frag(kong, "occlusion += texpaint_pack_sample.r;");
		node_shader_write_frag(kong, "height += texpaint_pack_sample.a;");
		node_shader_write_frag(kong, "nortan += sample(texpaint_nor_undo, sampler_linear, tex_coord_inp + float2(blur_step * float(i - 7), 0.0)).rgb * blur_weight_i;");
		node_shader_write_frag(kong, "}");
		// Y
		node_shader_write_frag(kong, "for (var j: int = 0; j <= 14; j += 1) {");
		node_shader_write_frag(kong, "var texpaint_sample: float4 = sample(texpaint_undo, sampler_linear, tex_coord_inp + float2(0.0, blur_step * float(j - 7)));");
		node_shader_write_frag(kong, "var blur_weight_j: float = get_blur_tool_weight(j);");
		node_shader_write_frag(kong, "opacity += texpaint_sample.a * blur_weight_j;");
		node_shader_write_frag(kong, "basecol += texpaint_sample.rgb * blur_weight_j;");
		node_shader_write_frag(kong, "var texpaint_pack_sample: float4 = sample(texpaint_pack_undo, sampler_linear, tex_coord_inp + float2(0.0, blur_step * float(j - 7))) * blur_weight_j;");
		node_shader_write_frag(kong, "roughness += texpaint_pack_sample.g;");
		node_shader_write_frag(kong, "metallic += texpaint_pack_sample.b;");
		node_shader_write_frag(kong, "occlusion += texpaint_pack_sample.r;");
		node_shader_write_frag(kong, "height += texpaint_pack_sample.a;");
		node_shader_write_frag(kong, "nortan += sample(texpaint_nor_undo, sampler_linear, tex_coord_inp + float2(0.0, blur_step * float(j - 7))).rgb * blur_weight_j;");
		node_shader_write_frag(kong, "}");
	}
	node_shader_write_frag(kong, "opacity *= constants.brush_opacity;");
}
