
function make_paint_color_attachments(): string[] {
	if (context_raw.tool == workspace_tool_t.PICKER) {
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

	if (context_raw.tool == workspace_tool_t.PICKER) {
		// Mangle vertices to form full screen triangle
		node_shader_write_vert(kong, "output.pos = float4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);");

		node_shader_add_uniform(kong, "gbuffer2: tex2d");
		node_shader_add_uniform(kong, "gbuffer_size: float2", "_gbuffer_size");
		node_shader_add_uniform(kong, "inp: float4", "_input_brush");

		node_shader_write_frag(kong, "var tex_coord_inp: float2 = gbuffer2[int2(inp.x * gbuffer_size.x, inp.y * gbuffer_size.y)].ba;");

		kong.frag_out = "float4[4]";
		node_shader_add_uniform(kong, "texpaint: tex2d");
		node_shader_add_uniform(kong, "texpaint_nor: tex2d");
		node_shader_add_uniform(kong, "texpaint_pack: tex2d");
		node_shader_write_frag(kong, "output[0].rgba = sample_lod(texpaint, tex_coord_inp, 0.0);");
		node_shader_write_frag(kong, "output[1].rgba = sample_lod(texpaint_nor, tex_coord_inp, 0.0);");
		node_shader_write_frag(kong, "output[2].rgba = sample_lod(texpaint_pack, tex_coord_inp, 0.0);");
		node_shader_write_frag(kong, "output[3].rg = tex_coord_inp.xy;");
		con_paint.data.shader_from_source = true;
		gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_paint.data.vertex_shader), ADDRESS(con_paint.data.fragment_shader));
		return con_paint;
	}

	node_shader_write_vert(kong, "var tpos: float2 = float2(tex.x * 2.0 - 1.0, (1.0 - tex.y) * 2.0 - 1.0);");

	node_shader_write_vert(kong, "output.pos = float4(tpos, 0.0, 1.0);");

	node_shader_add_uniform(kong, "WVP: float4x4", "_world_view_proj_matrix");

	node_shader_add_out(kong, "ndc: float4");
	node_shader_write_attrib_vert(kong, "ndc = WVP * float4(pos.xyz, 1.0);");

	node_shader_write_attrib_vert(kong, "var sp: float3 = float3((ndc.xyz / ndc.w) * 0.5 + 0.5);");
	node_shader_write_attrib_vert(kong, "sp.y = 1.0 - sp.y;");
	node_shader_write_attrib_vert(kong, "sp.z -= 0.0001;"); // small bias

	node_shader_add_uniform(kong, "inp: float4", "_input_brush");
	node_shader_add_uniform(kong, "inplast: float4", "_input_brush_last");
	node_shader_add_uniform(kong, "aspect_ratio: float", "_aspect_ratio_window");
	node_shader_write_frag(kong, "var bsp: float2 = sp.xy * 2.0 - 1.0;");
	node_shader_write_frag(kong, "bsp.x *= aspect_ratio;");
	node_shader_write_frag(kong, "bsp = bsp * 0.5 + 0.5;");

	node_shader_add_uniform(kong, "gbufferD: tex2d");

	kong.frag_out = "float4[4]";

	node_shader_add_uniform(kong, "brush_radius: float", "_brush_radius");
	node_shader_add_uniform(kong, "brush_opacity: float", "_brush_opacity");
	node_shader_add_uniform(kong, "brush_hardness: float", "_brush_hardness");

	if (context_raw.tool == workspace_tool_t.ERASER ||
		context_raw.tool == workspace_tool_t.CLONE  ||
		context_raw.tool == workspace_tool_t.BLUR   ||
		context_raw.tool == workspace_tool_t.SMUDGE) {

		node_shader_write_frag(kong, "var dist: float = 0.0;");

		node_shader_write_frag(kong, "var depth: float = sample_lod(gbufferD, inp.xy, 0.0).r;");

		node_shader_add_uniform(kong, "invVP: float4x4", "_inv_view_proj_matrix");
		node_shader_write_frag(kong, "var winp: float4 = float4(float2(inp.x, 1.0 - inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "winp = invVP * winp;");
		node_shader_write_frag(kong, "winp.xyz /= winp.w;");
		kong.frag_wposition = true;

		node_shader_write_frag(kong, "var depthlast: float = sample_lod(gbufferD, inplast.xy, 0.0).r;");

		node_shader_write_frag(kong, "var winplast: float4 = float4(float2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "winplast = invVP * winplast;");
		node_shader_write_frag(kong, "winplast.xyz /= winplast.w;");

		node_shader_write_frag(kong, "var pa: float3 = wposition - winp.xyz;");
		node_shader_write_frag(kong, "var ba: float3 = winplast.xyz - winp.xyz;");

		// Capsule
		node_shader_write_frag(kong, "var h: float = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);");
		node_shader_write_frag(kong, "dist = length(pa - ba * h);");

		node_shader_write_frag(kong, "if (dist > brush_radius) { discard; }");
	}

	if (context_raw.tool == workspace_tool_t.CLONE || context_raw.tool == workspace_tool_t.BLUR || context_raw.tool == workspace_tool_t.SMUDGE) {
		node_shader_add_uniform(kong, "gbuffer2: tex2d");
		node_shader_add_uniform(kong, "gbuffer_size: float2", "_gbuffer_size");
		node_shader_add_uniform(kong, "texpaint_undo: tex2d", "_texpaint_undo");
		node_shader_add_uniform(kong, "texpaint_nor_undo: tex2d", "_texpaint_nor_undo");
		node_shader_add_uniform(kong, "texpaint_pack_undo: tex2d", "_texpaint_pack_undo");

		if (context_raw.tool == workspace_tool_t.CLONE) {
		}
		else { // Blur
		}
	}

	node_shader_write_frag(kong, "var opacity: float = 1.0;");
	node_shader_write_frag(kong, "if (opacity == 0.0) { discard; }");

	node_shader_write_frag(kong, "var str: float = clamp((brush_radius - dist) * brush_hardness * 400.0, 0.0, 1.0) * opacity;");

	// Manual blending to preserve memory
	kong.frag_wvpposition = true;
	node_shader_write_frag(kong, "var sample_tc: float2 = float2(wvpposition.xy / wvpposition.w) * 0.5 + 0.5;");
	node_shader_write_frag(kong, "sample_tc.y = 1.0 - sample_tc.y;");
	node_shader_add_uniform(kong, "paintmask: tex2d");
	node_shader_write_frag(kong, "var sample_mask: float = sample_lod(paintmask, sample_tc, 0.0).r;");
	node_shader_write_frag(kong, "str = max(str, sample_mask);");

	node_shader_add_uniform(kong, "texpaint_undo: tex2d", "_texpaint_undo");
	node_shader_write_frag(kong, "var sample_undo: float4 = sample_lod(texpaint_undo, sample_tc, 0.0);");

	if (context_raw.tool == workspace_tool_t.ERASER) {
		node_shader_write_frag(kong, "output[0].rgba = float4(0.0, 0.0, 0.0, 0.0);");
		node_shader_write_frag(kong, "output[1].rgba = float4(0.5, 0.5, 1.0, 0.0);");
		node_shader_write_frag(kong, "output[2].rgba = float4(1.0, 0.0, 0.0, 0.0);");
	}

	node_shader_write_frag(kong, "output[3].rgba = float4(str, 0.0, 0.0, 1.0);");

	parser_material_finalize(con_paint);
	parser_material_sample_keep_aspect = false;
	con_paint.data.shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_paint.data.vertex_shader), ADDRESS(con_paint.data.fragment_shader));

	return con_paint;
}
