
function make_sculpt_run(data: material_t, matcon: material_context_t): node_shader_context_t {
	let context_id: string = "paint";

	let props: shader_context_t = {
		name: context_id,
		depth_write: false,
		compare_mode: "always",
		cull_mode: "none",
		vertex_elements: [
			{
				name: "pos",
				data: "float2"
			}
		],
		color_attachments: [
			"RGBA32",
			"RGBA32",
			"RGBA32",
			"R8"
		]
	};
	let con_paint: node_shader_context_t = node_shader_context_create(data, props);

	con_paint.data.color_writes_red = [true, true, true, true];
	con_paint.data.color_writes_green = [true, true, true, true];
	con_paint.data.color_writes_blue = [true, true, true, true];
	con_paint.data.color_writes_alpha = [true, true, true, true];
	con_paint.allow_vcols = mesh_data_get_vertex_array(context_raw.paint_object.data, "col") != null;

	let kong: node_shader_t = node_shader_context_make_kong(con_paint);

	let decal: bool = context_is_decal();

	node_shader_add_out(kong, "tex_coord: float2");
	node_shader_write_vert(kong, "var madd: float2 = float2(0.5, 0.5);");
	node_shader_write_vert(kong, "output.tex_coord = input.pos.xy * madd + madd;");
	node_shader_write_vert(kong, "output.tex_coord.y = 1.0 - output.tex_coord.y;");
	node_shader_write_vert(kong, "output.pos = float4(input.pos.xy, 0.0, 1.0);");

	node_shader_add_constant(kong, "inp: float4", "_input_brush");
	node_shader_add_constant(kong, "inplast: float4", "_input_brush_last");
	node_shader_add_texture(kong, "gbufferD");

	kong.frag_out = "float4[2]";

	node_shader_add_constant(kong, "brush_radius: float", "_brush_radius");
	node_shader_add_constant(kong, "brush_opacity: float", "_brush_opacity");
	node_shader_add_constant(kong, "brush_hardness: float", "_brush_hardness");

	if (context_raw.tool == workspace_tool_t.BRUSH  ||
		context_raw.tool == workspace_tool_t.ERASER ||
		context_raw.tool == workspace_tool_t.CLONE  ||
		context_raw.tool == workspace_tool_t.BLUR   ||
		context_raw.tool == workspace_tool_t.SMUDGE   ||
		context_raw.tool == workspace_tool_t.PARTICLE ||
		decal) {

		let depth_reject: bool = !context_raw.xray;

		make_brush_run(kong);
	}

	node_shader_write_frag(kong, "var basecol: float3 = float3(1.0, 1.0, 1.0);");
	node_shader_write_frag(kong, "var opacity: float = 1.0;");
	node_shader_write_frag(kong, "if (opacity == 0.0) { discard; }");

	node_shader_write_frag(kong, "var str: float = clamp((constants.brush_radius - dist) * constants.brush_hardness * 400.0, 0.0, 1.0) * opacity;");

	node_shader_add_texture(kong, "texpaint_undo", "_texpaint_undo");
	node_shader_write_frag(kong, "var sample_undo: float4 = sample_lod(texpaint_undo, sampler_linear, input.tex_coord, 0.0);");

	node_shader_write_frag(kong, "if (sample_undo.r == 0 && sample_undo.g == 0 && sample_undo.b == 0) { discard; }");

	node_shader_add_function(kong, str_octahedron_wrap);
	node_shader_add_texture(kong, "gbuffer0_undo");
	node_shader_write_frag(kong, "var g0_undo: float2 = sample_lod(gbuffer0_undo, sampler_linear, constants.inp.xy, 0.0).rg;");
	node_shader_write_frag(kong, "var wn: float3;");
	node_shader_write_frag(kong, "wn.z = 1.0 - abs(g0_undo.x) - abs(g0_undo.y);");
	// node_shader_write_frag(kong, "wn.xy = wn.z >= 0.0 ? g0_undo.xy : octahedron_wrap(g0_undo.xy);");
	node_shader_write_frag(kong, "if (wn.z >= 0.0) { wn.xy = g0_undo.xy; } else { wn.xy = octahedron_wrap(g0_undo.xy); }");
	node_shader_write_frag(kong, "var n: float3 = normalize(wn);");

	node_shader_write_frag(kong, "output[0] = float4(sample_undo.rgb + n * 0.1 * str, 1.0);");

	node_shader_write_frag(kong, "output[1] = float4(str, 0.0, 0.0, 1.0);");

	parser_material_finalize(con_paint);
	con_paint.data.shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_paint.data.vertex_shader), ADDRESS(con_paint.data.fragment_shader));

	return con_paint;
}
