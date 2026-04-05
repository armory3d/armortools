
#include "global.h"

void make_clone_run(node_shader_t *kong) {
	node_shader_add_constant(kong, "clone_delta: float2", "_clone_delta");
	// node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[uint2((sp.xy + constants.clone_delta) * constants.gbuffer_size)].ba;");
	node_shader_write_frag(kong, "var tex_coord_coord: uint2 = uint2(uint((sp.x + constants.clone_delta.x) * constants.gbuffer_size.x), uint((sp.y + "
	                             "constants.clone_delta.y) * constants.gbuffer_size.y));");
	node_shader_write_frag(kong, "var tex_coord_inp4: float4 = gbuffer2[tex_coord_coord];");
	node_shader_write_frag(kong, "var tex_coord_inp: float2 = tex_coord_inp4.ba;");

	node_shader_write_frag(kong, "var texpaint_pack_sample: float3 = sample_lod(texpaint_pack_undo, sampler_linear, tex_coord_inp, 0.0).rgb;");
	char *base   = "sample_lod(texpaint_undo, sampler_linear, tex_coord_inp, 0.0).rgb";
	char *rough  = "texpaint_pack_sample.g";
	char *met    = "texpaint_pack_sample.b";
	char *occ    = "texpaint_pack_sample.r";
	char *nortan = "sample_lod(texpaint_nor_undo, sampler_linear, tex_coord_inp, 0.0).rgb";
	char *height = "0.0";
	char *opac   = "1.0";
	node_shader_write_frag(kong, string("var basecol: float3 = %s;", base));
	node_shader_write_frag(kong, string("var roughness: float = %s;", rough));
	node_shader_write_frag(kong, string("var metallic: float = %s;", met));
	node_shader_write_frag(kong, string("var occlusion: float = %s;", occ));
	node_shader_write_frag(kong, string("var nortan: float3 = %s;", nortan));
	node_shader_write_frag(kong, string("var height: float = %s;", height));
	node_shader_write_frag(kong, string("var mat_opacity: float = %s;", opac));
	node_shader_write_frag(kong, "var opacity: float = mat_opacity * constants.brush_opacity;");
	if (g_context->material->paint_emis) {
		node_shader_write_frag(kong, "var emis: float = 0.0;");
	}
	if (g_context->material->paint_subs) {
		node_shader_write_frag(kong, "var subs: float = 0.0;");
	}
}
