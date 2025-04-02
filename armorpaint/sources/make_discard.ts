
function make_discard_color_id(kong: node_shader_t) {
	node_shader_add_uniform(kong, "texpaint_colorid: tex2d"); // 1x1 picker
	node_shader_add_uniform(kong, "texcolorid: tex2d", "_texcolorid"); // color map
	node_shader_write_frag(kong, "var colorid_c1: float3 = texpaint_colorid[int2(0, 0)].rgb;");
	node_shader_write_frag(kong, "var colorid_c2: float3 = sample_lod(texcolorid, tex_coord_pick, 0).rgb;");
	// node_shader_write_frag(kong, "if (any(colorid_c1 != colorid_c2)) { discard };");
	node_shader_write_frag(kong, "if (colorid_c1 != colorid_c2) { discard; }");
}

function make_discard_face(kong: node_shader_t) {
	node_shader_add_uniform(kong, "gbuffer2: tex2d");
	node_shader_add_uniform(kong, "textrianglemap: tex2d", "_textrianglemap");
	node_shader_add_uniform(kong, "textrianglemap_size: float2", "_texpaint_size");
	node_shader_add_uniform(kong, "gbuffer_size: float2", "_gbuffer_size");
	node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[int2(inp.x * gbuffer_size.x, inp.y * gbuffer_size.y)].ba;");
	node_shader_write_frag(kong, "var face_c1: float4 = textrianglemap[int2(tex_coord_inp * textrianglemap_size)];");
	node_shader_write_frag(kong, "var face_c2: float4 = sample_lod(textrianglemap, tex_coord_pick, 0.0);");
	// node_shader_write_frag(kong, "if (any(face_c1 != face_c2)) { discard; }");
	node_shader_write_frag(kong, "if (face_c1 != face_c2) { discard; }");
}

function make_discard_uv_island(kong: node_shader_t) {
	node_shader_add_uniform(kong, "texuvislandmap: tex2d", "_texuvislandmap");
	node_shader_write_frag(kong, "if (sample_lod(texuvislandmap, tex_coord_pick, 0.0).r == 0.0) { discard; }");
}

function make_discard_material_id(kong: node_shader_t) {
	kong.frag_wvpposition = true;
	node_shader_write_frag(kong, "var picker_sample_tc: float2 = float2(wvpposition.x / wvpposition.w, wvpposition.y / wvpposition.w) * 0.5 + 0.5;");
	node_shader_write_frag(kong, "picker_sample_tc.y = 1.0 - picker_sample_tc.y;");
	node_shader_add_uniform(kong, "texpaint_nor_undo: tex2d", "_texpaint_nor_undo");
	let matid: i32 = context_raw.materialid_picked / 255;
	node_shader_write_frag(kong, "if (" + matid + " != sample_lod(texpaint_nor_undo, picker_sample_tc, 0.0).a) { discard; }");
}
