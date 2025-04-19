
function make_clone_run(kong: node_shader_t) {
	node_shader_add_constant(kong, "clone_delta: float2", "_clone_delta");
	node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[uint2((sp.xy + constants.clone_delta) * gbuffer_size)].ba;");

	node_shader_write_frag(kong, "var texpaint_pack_sample: float3 = sample_lod(texpaint_pack_undo, sampler_linear, tex_coord_inp, 0.0).rgb;");
	let base: string = "sample_lod(texpaint_undo, sampler_linear, tex_coord_inp, 0.0).rgb";
	let rough: string = "texpaint_pack_sample.g";
	let met: string = "texpaint_pack_sample.b";
	let occ: string = "texpaint_pack_sample.r";
	let nortan: string = "sample_lod(texpaint_nor_undo, sampler_linear, tex_coord_inp, 0.0).rgb";
	let height: string = "0.0";
	let opac: string = "1.0";
	node_shader_write_frag(kong, "var basecol: float3 = " + base + ";");
	node_shader_write_frag(kong, "var roughness: float = " + rough + ";");
	node_shader_write_frag(kong, "var metallic: float = " + met + ";");
	node_shader_write_frag(kong, "var occlusion: float = " + occ + ";");
	node_shader_write_frag(kong, "var nortan: float3 = " + nortan + ";");
	node_shader_write_frag(kong, "var height: float = " + height + ";");
	node_shader_write_frag(kong, "var mat_opacity: float = " + opac + ";");
	node_shader_write_frag(kong, "var opacity: float = mat_opacity * brush_opacity;");
	if (context_raw.material.paint_emis) {
		node_shader_write_frag(kong, "var emis: float = 0.0;");
	}
	if (context_raw.material.paint_subs) {
		node_shader_write_frag(kong, "var subs: float = 0.0;");
	}
}
