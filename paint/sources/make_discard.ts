
function make_discard_color_id(kong: node_shader_t) {
	node_shader_add_texture(kong, "texpaint_colorid");          // 1x1 picker
	node_shader_add_texture(kong, "texcolorid", "_texcolorid"); // color map
	// node_shader_write_frag(kong, "var colorid_c1: float3 = texpaint_colorid[uint2(0, 0)].rgb;");
	node_shader_write_frag(kong, "var colorid_c14: float4 = texpaint_colorid[uint2(uint(0), uint(0))];");
	node_shader_write_frag(kong, "var colorid_c1: float3 = colorid_c14.rgb;");
	node_shader_write_frag(kong, "var colorid_c2: float3 = sample_lod(texcolorid, sampler_linear, input.tex_coord_pick, 0.0).rgb;");
	// node_shader_write_frag(kong, "if (any(colorid_c1 != colorid_c2)) { discard };");
	node_shader_write_frag(kong, "if (colorid_c1.r != colorid_c2.r || colorid_c1.g != colorid_c2.g || colorid_c1.b != colorid_c2.b) { discard; }");
}

function make_discard_face(kong: node_shader_t) {
	node_shader_add_texture(kong, "gbuffer2");
	node_shader_add_texture(kong, "textrianglemap", "_textrianglemap");
	node_shader_add_constant(kong, "textrianglemap_size: float2", "_texpaint_size");
	node_shader_add_constant(kong, "gbuffer_size: float2", "_gbuffer_size");
	// node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[uint2(constants.inp.x * constants.gbuffer_size.x, constants.inp.y *
	// constants.gbuffer_size.y)].ba;");
	node_shader_write_frag(
	    kong,
	    "var tex_coord_inp4: float4 = gbuffer2[uint2(uint(constants.inp.x * constants.gbuffer_size.x), uint(constants.inp.y * constants.gbuffer_size.y))];");
	node_shader_write_frag(kong, "var tex_coord_inp: float2 = tex_coord_inp4.ba;");
	node_shader_write_frag(
	    kong,
	    "var face_c1: float4 = textrianglemap[uint2(uint(tex_coord_inp.x * constants.textrianglemap_size.x), uint(tex_coord_inp.y * constants.textrianglemap_size.y))];");
	node_shader_write_frag(kong, "var face_c2: float4 = sample_lod(textrianglemap, sampler_linear, input.tex_coord_pick, 0.0);");
	// node_shader_write_frag(kong, "if (any(face_c1 != face_c2)) { discard; }");
	node_shader_write_frag(kong, "if (face_c1.x != face_c2.x || face_c1.y != face_c2.y || face_c1.z != face_c2.z || face_c1.w != face_c2.w) { discard; }");
}

function make_discard_uv_island(kong: node_shader_t) {
	node_shader_add_texture(kong, "texuvislandmap", "_texuvislandmap");
	node_shader_write_frag(kong, "if (sample_lod(texuvislandmap, sampler_linear, input.tex_coord_pick, 0.0).r == 0.0) { discard; }");
}

function make_discard_material_id(kong: node_shader_t) {
	kong.frag_wvpposition = true;
	node_shader_write_frag(
	    kong, "var picker_sample_tc: float2 = float2(input.wvpposition.x / input.wvpposition.w, input.wvpposition.y / input.wvpposition.w) * 0.5 + 0.5;");
	node_shader_write_frag(kong, "picker_sample_tc.y = 1.0 - picker_sample_tc.y;");
	node_shader_add_texture(kong, "texpaint_nor_undo", "_texpaint_nor_undo");
	let matid: i32 = context_raw.materialid_picked / 255;
	node_shader_write_frag(kong, "if (" + matid + " != sample_lod(texpaint_nor_undo, sampler_linear, picker_sample_tc, 0.0).a) { discard; }");
}
