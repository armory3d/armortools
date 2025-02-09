
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

	let vert: node_shader_t = node_shader_context_make_vert(con_paint);
	let frag: node_shader_t = node_shader_context_make_frag(con_paint);
	frag.ins = vert.outs;

	let decal: bool = context_is_decal();

	node_shader_add_out(vert, "vec2 tex_coord");
	node_shader_write(vert, "const vec2 madd = vec2(0.5, 0.5);");
	node_shader_write(vert, "tex_coord = pos.xy * madd + madd;");
	node_shader_write(vert, "tex_coord.y = 1.0 - tex_coord.y;");
	node_shader_write(vert, "gl_Position = vec4(pos.xy, 0.0, 1.0);");

	node_shader_add_uniform(frag, "vec4 inp", "_input_brush");
	node_shader_add_uniform(frag, "vec4 inplast", "_input_brush_last");

	node_shader_add_uniform(frag, "sampler2D gbufferD");

	node_shader_add_out(frag, "vec4 frag_color[2]");

	node_shader_add_uniform(frag, "float brush_radius", "_brush_radius");
	node_shader_add_uniform(frag, "float brush_opacity", "_brush_opacity");
	node_shader_add_uniform(frag, "float brush_hardness", "_brush_hardness");

	if (context_raw.tool == workspace_tool_t.BRUSH  ||
		context_raw.tool == workspace_tool_t.ERASER ||
		context_raw.tool == workspace_tool_t.CLONE  ||
		context_raw.tool == workspace_tool_t.BLUR   ||
		context_raw.tool == workspace_tool_t.SMUDGE   ||
		context_raw.tool == workspace_tool_t.PARTICLE ||
		decal) {

		let depth_reject: bool = !context_raw.xray;

		make_brush_run(vert, frag);
	}

	node_shader_write(frag, "vec3 basecol = vec3(1.0, 1.0, 1.0);");
	node_shader_write(frag, "float opacity = 1.0;");
	node_shader_write(frag, "if (opacity == 0.0) discard;");

	node_shader_write(frag, "float str = clamp((brush_radius - dist) * brush_hardness * 400.0, 0.0, 1.0) * opacity;");

	node_shader_add_uniform(frag, "sampler2D texpaint_undo", "_texpaint_undo");
	node_shader_write(frag, "vec4 sample_undo = textureLod(texpaint_undo, tex_coord, 0.0);");

	node_shader_write(frag, "if (sample_undo.r == 0 && sample_undo.g == 0 && sample_undo.b == 0) discard;");

	node_shader_add_function(frag, str_octahedron_wrap);
	node_shader_add_uniform(frag, "sampler2D gbuffer0_undo");
	node_shader_write(frag, "vec2 g0_undo = textureLod(gbuffer0_undo, inp.xy, 0.0).rg;");
	node_shader_write(frag, "vec3 wn;");
	node_shader_write(frag, "wn.z = 1.0 - abs(g0_undo.x) - abs(g0_undo.y);");
	node_shader_write(frag, "wn.xy = wn.z >= 0.0 ? g0_undo.xy : octahedron_wrap(g0_undo.xy);");
	node_shader_write(frag, "vec3 n = normalize(wn);");

	node_shader_write(frag, "frag_color[0] = vec4(sample_undo.rgb + n * 0.1 * str, 1.0);");

	node_shader_write(frag, "frag_color[1] = vec4(str, 0.0, 0.0, 1.0);");

	parser_material_finalize(con_paint);
	con_paint.data.shader_from_source = true;
	con_paint.data.vertex_shader = node_shader_get(vert);
	con_paint.data.fragment_shader = node_shader_get(frag);

	return con_paint;
}
