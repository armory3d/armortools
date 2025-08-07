
function make_paint_color_attachments(): string[] {
	if (context_raw.tool == tool_type_t.PICKER) {
		let res: string[] = ["RGBA32", "RGBA32", "RGBA32", "RGBA32"];
		return res;
	}
	let res: string[] = ["RGBA32", "RGBA32", "RGBA32", "R8"];
	return res;
}

function make_paint_run(data: material_t, matcon: material_context_t): node_shader_context_t {
	let props: shader_context_t = {
		name: "paint",
		depth_write: false,
		compare_mode: "always",
		cull_mode: "none",
		vertex_elements: [
			{
				name: "pos",
				data: "short4norm"
			},
			{
				name: "nor",
				data: "short2norm"
			},
			{
				name: "tex",
				data: "short2norm"
			}
		],
		color_attachments: make_paint_color_attachments()
	};
	let con_paint: node_shader_context_t = node_shader_context_create(data, props);

	con_paint.data.color_writes_red = [true, true, true, true];
	con_paint.data.color_writes_green = [true, true, true, true];
	con_paint.data.color_writes_blue = [true, true, true, true];
	con_paint.data.color_writes_alpha = [true, true, true, true];
	con_paint.allow_vcols = mesh_data_get_vertex_array(context_raw.paint_object.data, "col") != null;

	let kong: node_shader_t = node_shader_context_make_kong(con_paint);

	if (context_raw.tool == tool_type_t.PICKER) {
		// Mangle vertices to form full screen triangle
		node_shader_write_vert(kong, "output.pos = float4(-1.0 + float((vertex_id() & 1) << 2), -1.0 + float((vertex_id() & 2) << 1), 0.0, 1.0);");

		node_shader_add_texture(kong, "gbuffer2");
		node_shader_add_constant(kong, "gbuffer_size: float2", "_gbuffer_size");
		node_shader_add_constant(kong, "inp: float4", "_input_brush");

		// node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[uint2(constants.inp.x * constants.gbuffer_size.x, constants.inp.y * constants.gbuffer_size.y)].ba;");
		node_shader_write_frag(kong, "var tex_coord_inp4: float4 = gbuffer2[uint2(constants.inp.x * constants.gbuffer_size.x, constants.inp.y * constants.gbuffer_size.y)];");
		node_shader_write_frag(kong, "var tex_coord_inp: float2 = tex_coord_inp4.ba;");

		kong.frag_out = "float4[4]";
		node_shader_add_texture(kong, "texpaint");
		node_shader_add_texture(kong, "texpaint_nor");
		node_shader_add_texture(kong, "texpaint_pack");
		node_shader_write_frag(kong, "output[0] = sample_lod(texpaint, sampler_linear, tex_coord_inp, 0.0);");
		node_shader_write_frag(kong, "output[1] = sample_lod(texpaint_nor, sampler_linear, tex_coord_inp, 0.0);");
		node_shader_write_frag(kong, "output[2] = sample_lod(texpaint_pack, sampler_linear, tex_coord_inp, 0.0);");
		node_shader_write_frag(kong, "output[3].rg = tex_coord_inp.xy;");
		con_paint.data.shader_from_source = true;
		gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_paint.data.vertex_shader), ADDRESS(con_paint.data.fragment_shader), ADDRESS(con_paint.data._.vertex_shader_size), ADDRESS(con_paint.data._.fragment_shader_size));
		return con_paint;
	}

	node_shader_write_vert(kong, "var tpos: float2 = float2(input.tex.x * 2.0 - 1.0, (1.0 - input.tex.y) * 2.0 - 1.0);");

	node_shader_write_vert(kong, "output.pos = float4(tpos, 0.0, 1.0);");

	node_shader_add_constant(kong, "WVP: float4x4", "_world_view_proj_matrix");

	node_shader_add_out(kong, "ndc: float4");
	node_shader_write_attrib_vert(kong, "output.ndc = constants.WVP * float4(input.pos.xyz, 1.0);");

	node_shader_write_attrib_frag(kong, "var sp: float3 = (input.ndc.xyz / input.ndc.w) * 0.5 + 0.5;");
	node_shader_write_attrib_frag(kong, "sp.y = 1.0 - sp.y;");
	node_shader_write_attrib_frag(kong, "sp.z -= 0.0001;"); // small bias

	node_shader_add_constant(kong, "inp: float4", "_input_brush");
	node_shader_add_constant(kong, "inplast: float4", "_input_brush_last");
	node_shader_add_constant(kong, "aspect_ratio: float", "_aspect_ratio_window");
	node_shader_write_frag(kong, "var bsp: float2 = sp.xy * 2.0 - 1.0;");
	node_shader_write_frag(kong, "bsp.x *= constants.aspect_ratio;");
	node_shader_write_frag(kong, "bsp = bsp * 0.5 + 0.5;");

	node_shader_add_texture(kong, "gbufferD");

	kong.frag_out = "float4[4]";

	node_shader_add_constant(kong, "brush_radius: float", "_brush_radius");
	node_shader_add_constant(kong, "brush_opacity: float", "_brush_opacity");
	node_shader_add_constant(kong, "brush_hardness: float", "_brush_hardness");

	if (context_raw.tool == tool_type_t.ERASER ||
		context_raw.tool == tool_type_t.CLONE  ||
		context_raw.tool == tool_type_t.BLUR   ||
		context_raw.tool == tool_type_t.SMUDGE) {

		node_shader_write_frag(kong, "var dist: float = 0.0;");

		node_shader_write_frag(kong, "var depth: float = sample_lod(gbufferD, sampler_linear, constants.inp.xy, 0.0).r;");

		node_shader_add_constant(kong, "invVP: float4x4", "_inv_view_proj_matrix");
		node_shader_write_frag(kong, "var winp: float4 = float4(float2(constants.inp.x, 1.0 - constants.inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "winp = constants.invVP * winp;");
		node_shader_write_frag(kong, "winp.xyz = winp.xyz / winp.w;");
		kong.frag_wposition = true;

		node_shader_write_frag(kong, "var depthlast: float = sample_lod(gbufferD, sampler_linear, constants.inplast.xy, 0.0).r;");

		node_shader_write_frag(kong, "var winplast: float4 = float4(float2(constants.inplast.x, 1.0 - constants.inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "winplast = constants.invVP * winplast;");
		node_shader_write_frag(kong, "winplast.xyz = winplast.xyz / winplast.w;");

		node_shader_write_frag(kong, "var pa: float3 = input.wposition - winp.xyz;");
		node_shader_write_frag(kong, "var ba: float3 = winplast.xyz - winp.xyz;");

		// Capsule
		node_shader_write_frag(kong, "var h: float = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);");
		node_shader_write_frag(kong, "dist = length(pa - ba * h);");

		node_shader_write_frag(kong, "if (dist > constants.brush_radius) { discard; }");
	}

	if (context_raw.tool == tool_type_t.CLONE || context_raw.tool == tool_type_t.BLUR || context_raw.tool == tool_type_t.SMUDGE) {
		node_shader_add_texture(kong, "gbuffer2");
		node_shader_add_constant(kong, "gbuffer_size: float2", "_gbuffer_size");
		node_shader_add_texture(kong, "texpaint_undo", "_texpaint_undo");
		node_shader_add_texture(kong, "texpaint_nor_undo", "_texpaint_nor_undo");
		node_shader_add_texture(kong, "texpaint_pack_undo", "_texpaint_pack_undo");

		if (context_raw.tool == tool_type_t.CLONE) {
		}
		else { // Blur
		}
	}

	node_shader_write_frag(kong, "var opacity: float = 1.0;");
	node_shader_write_frag(kong, "if (opacity == 0.0) { discard; }");

	node_shader_write_frag(kong, "var str: float = clamp((constants.brush_radius - dist) * constants.brush_hardness * 400.0, 0.0, 1.0) * opacity;");

	// Manual blending to preserve memory
	kong.frag_wvpposition = true;
	node_shader_write_frag(kong, "var sample_tc: float2 = float2(input.wvpposition.x / input.wvpposition.w, input.wvpposition.y / input.wvpposition.w) * 0.5 + 0.5;");
	node_shader_write_frag(kong, "sample_tc.y = 1.0 - sample_tc.y;");
	node_shader_add_texture(kong, "paintmask");
	node_shader_write_frag(kong, "var sample_mask: float = sample_lod(paintmask, sampler_linear, sample_tc, 0.0).r;");
	node_shader_write_frag(kong, "str = max(str, sample_mask);");

	node_shader_add_texture(kong, "texpaint_undo", "_texpaint_undo");
	node_shader_write_frag(kong, "var sample_undo: float4 = sample_lod(texpaint_undo, sampler_linear, sample_tc, 0.0);");

	if (context_raw.tool == tool_type_t.ERASER) {
		node_shader_write_frag(kong, "output[0] = float4(0.0, 0.0, 0.0, 0.0);");
		node_shader_write_frag(kong, "output[1] = float4(0.5, 0.5, 1.0, 0.0);");
		node_shader_write_frag(kong, "output[2] = float4(1.0, 0.0, 0.0, 0.0);");
	}

	node_shader_write_frag(kong, "output[3] = float4(str, 0.0, 0.0, 1.0);");

	parser_material_finalize(con_paint);
	parser_material_sample_keep_aspect = false;
	con_paint.data.shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_paint.data.vertex_shader), ADDRESS(con_paint.data.fragment_shader), ADDRESS(con_paint.data._.vertex_shader_size), ADDRESS(con_paint.data._.fragment_shader_size));

	return con_paint;
}
