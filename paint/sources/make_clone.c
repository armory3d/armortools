void make_clone_run(node_shader_t *kong) {
	node_shader_add_constant(kong, "clone_delta: float2", "_clone_delta");
	// node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[uint2((sp.xy + constants.clone_delta) * constants.gbuffer_size)].ba;");
	node_shader_write_frag(kong, "var tex_coord_coord: uint2 = uint2(uint((sp.x + constants.clone_delta.x) * constants.gbuffer_size.x), uint((sp.y + "
	                             "constants.clone_delta.y) * constants.gbuffer_size.y));");
	node_shader_write_frag(kong, "var tex_coord_inp4: float4 = gbuffer2[tex_coord_coord];");
	node_shader_write_frag(kong, "var tex_coord_inp: float2 = tex_coord_inp4.ba;");

	node_shader_write_frag(kong, "var texpaint_pack_sample: float3 = sample_lod(texpaint_pack_undo, sampler_linear, tex_coord_inp, 0.0).rgb;");
	string_t *base   = "sample_lod(texpaint_undo, sampler_linear, tex_coord_inp, 0.0).rgb";
	string_t *rough  = "texpaint_pack_sample.g";
	string_t *met    = "texpaint_pack_sample.b";
	string_t *occ    = "texpaint_pack_sample.r";
	string_t *nortan = "sample_lod(texpaint_nor_undo, sampler_linear, tex_coord_inp, 0.0).rgb";
	string_t *height = "0.0";
	string_t *opac   = "1.0";
	node_shader_write_frag(kong, string_join(string_join("var basecol: float3 = ", base), ";"));
	node_shader_write_frag(kong, string_join(string_join("var roughness: float = ", rough), ";"));
	node_shader_write_frag(kong, string_join(string_join("var metallic: float = ", met), ";"));
	node_shader_write_frag(kong, string_join(string_join("var occlusion: float = ", occ), ";"));
	node_shader_write_frag(kong, string_join(string_join("var nortan: float3 = ", nortan), ";"));
	node_shader_write_frag(kong, string_join(string_join("var height: float = ", height), ";"));
	node_shader_write_frag(kong, string_join(string_join("var mat_opacity: float = ", opac), ";"));
	node_shader_write_frag(kong, "var opacity: float = mat_opacity * constants.brush_opacity;");
	if (context_raw->material->paint_emis) {
		node_shader_write_frag(kong, "var emis: float = 0.0;");
	}
	if (context_raw->material->paint_subs) {
		node_shader_write_frag(kong, "var subs: float = 0.0;");
	}
}
