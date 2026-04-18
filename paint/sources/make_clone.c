
#include "global.h"

void make_clone_run(node_shader_t *kong) {
	node_shader_add_constant(kong, "clone_delta: float2", "_clone_delta");
	// node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[uint2((sp.xy + constants.clone_delta) * constants.gbuffer_size)].ba;");
	node_shader_write_frag(kong, "var tex_coord_coord: uint2 = uint2(uint((sp.x + constants.clone_delta.x) * constants.gbuffer_size.x), uint((sp.y + "
	                             "constants.clone_delta.y) * constants.gbuffer_size.y));");
	node_shader_write_frag(kong, "var tex_coord_inp4: float4 = gbuffer2[tex_coord_coord];");
	node_shader_write_frag(kong, "var tex_coord_inp: float2 = tex_coord_inp4.ba;");

	node_shader_write_frag(kong, "var texpaint_undo_sample: float4 = sample_lod(texpaint_undo, sampler_linear, tex_coord_inp, 0.0);");
	node_shader_write_frag(kong, "var texpaint_pack_undo_sample: float4 = sample_lod(texpaint_pack_undo, sampler_linear, tex_coord_inp, 0.0);");
	node_shader_write_frag(kong, "var texpaint_nor_undo_sample: float4 = sample_lod(texpaint_nor_undo, sampler_linear, tex_coord_inp, 0.0);");
	node_shader_write_frag(kong, "var clone_src_alpha: float = texpaint_undo_sample.a;");
	node_shader_write_frag(kong, "var basecol: float3 = texpaint_undo_sample.rgb;");
	node_shader_write_frag(kong, "var roughness: float = texpaint_pack_undo_sample.g;");
	node_shader_write_frag(kong, "var metallic: float = texpaint_pack_undo_sample.b;");
	node_shader_write_frag(kong, "var occlusion: float = texpaint_pack_undo_sample.r;");
	node_shader_write_frag(kong, "var nortan: float3 = texpaint_nor_undo_sample.rgb;");
	node_shader_write_frag(kong, "var height: float = texpaint_pack_undo_sample.a;");
	node_shader_write_frag(kong, "var mat_opacity: float = texpaint_undo_sample.a;");
	node_shader_write_frag(kong, "var opacity: float = mat_opacity * constants.brush_opacity;");
	if (g_context->material->paint_emis || g_context->material->paint_subs) {
		node_shader_write_frag(kong, "var clone_matid_mod: float = float(int(texpaint_nor_undo_sample.a * 255.0)) % float(3);");
	}
	if (g_context->material->paint_emis) {
		node_shader_write_frag(kong, "var emis: float = 0.0; if (clone_matid_mod == 1.0) { emis = 1.0; }");
	}
	if (g_context->material->paint_subs) {
		node_shader_write_frag(kong, "var subs: float = 0.0; if (clone_matid_mod == 2.0) { subs = 1.0; }");
	}
}
