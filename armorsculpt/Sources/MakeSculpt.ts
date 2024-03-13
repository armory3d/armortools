
class MakeSculpt {

	static run = (data: TMaterial, matcon: material_context_t): NodeShaderContextRaw => {
		let context_id = "paint";
		let con_paint = NodeShadercontext_create(data, {
			name: context_id,
			depth_write: false,
			compare_mode: "always", // TODO: align texcoords winding order
			// cull_mode: "counter_clockwise",
			cull_mode: "none",
			vertex_elements: [{name: "pos", data: "float2"}],
			color_attachments: ["RGBA32", "RGBA32", "RGBA32", "R8"]
		});

		con_paint.data.color_writes_red = [true, true, true, true];
		con_paint.data.color_writes_green = [true, true, true, true];
		con_paint.data.color_writes_blue = [true, true, true, true];
		con_paint.data.color_writes_alpha = [true, true, true, true];
		con_paint.allow_vcols = mesh_data_get_vertex_array(context_raw.paintObject.data, "col") != null;

		let vert = NodeShadercontext_make_vert(con_paint);
		let frag = NodeShadercontext_make_frag(con_paint);
		frag.ins = vert.outs;

		let faceFill = context_raw.tool == WorkspaceTool.ToolFill && context_raw.fillTypeHandle.position == FillType.FillFace;
		let decal = context_raw.tool == WorkspaceTool.ToolDecal || context_raw.tool == WorkspaceTool.ToolText;

		node_shader_add_out(vert, 'vec2 texCoord');
		node_shader_write(vert, 'const vec2 madd = vec2(0.5, 0.5);');
		node_shader_write(vert, 'texCoord = pos.xy * madd + madd;');
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		node_shader_write(vert, 'texCoord.y = 1.0 - texCoord.y;');
		///end
		node_shader_write(vert, 'gl_Position = vec4(pos.xy, 0.0, 1.0);');

		node_shader_add_uniform(frag, 'vec4 inp', '_inputBrush');
		node_shader_add_uniform(frag, 'vec4 inplast', '_inputBrushLast');

		node_shader_add_uniform(frag, 'sampler2D gbufferD');

		node_shader_add_out(frag, 'vec4 fragColor[2]');

		node_shader_add_uniform(frag, 'float brushRadius', '_brushRadius');
		node_shader_add_uniform(frag, 'float brushOpacity', '_brushOpacity');
		node_shader_add_uniform(frag, 'float brushHardness', '_brushHardness');

		if (context_raw.tool == WorkspaceTool.ToolBrush  ||
			context_raw.tool == WorkspaceTool.ToolEraser ||
			context_raw.tool == WorkspaceTool.ToolClone  ||
			context_raw.tool == WorkspaceTool.ToolBlur   ||
			context_raw.tool == WorkspaceTool.ToolSmudge   ||
			context_raw.tool == WorkspaceTool.ToolParticle ||
			decal) {

			let depthReject = !context_raw.xray;

			MakeBrush.run(vert, frag);
		}

		node_shader_write(frag, 'vec3 basecol = vec3(1.0, 1.0, 1.0);');
		node_shader_write(frag, 'float opacity = 1.0;');
		node_shader_write(frag, 'if (opacity == 0.0) discard;');

		node_shader_write(frag, 'float str = clamp((brushRadius - dist) * brushHardness * 400.0, 0.0, 1.0) * opacity;');

		node_shader_add_uniform(frag, 'sampler2D texpaint_undo', '_texpaint_undo');
		node_shader_write(frag, 'vec4 sample_undo = textureLod(texpaint_undo, texCoord, 0.0);');

		node_shader_write(frag, 'if (sample_undo.r == 0 && sample_undo.g == 0 && sample_undo.b == 0) discard;');

		node_shader_add_function(frag, str_octahedronWrap);
		node_shader_add_uniform(frag, 'sampler2D gbuffer0_undo');
		node_shader_write(frag, 'vec2 g0_undo = textureLod(gbuffer0_undo, inp.xy, 0.0).rg;');
		node_shader_write(frag, 'vec3 wn;');
		node_shader_write(frag, 'wn.z = 1.0 - abs(g0_undo.x) - abs(g0_undo.y);');
		node_shader_write(frag, 'wn.xy = wn.z >= 0.0 ? g0_undo.xy : octahedronWrap(g0_undo.xy);');
		node_shader_write(frag, 'vec3 n = normalize(wn);');

		node_shader_write(frag, 'fragColor[0] = vec4(sample_undo.rgb + n * 0.1 * str, 1.0);');

		node_shader_write(frag, 'fragColor[1] = vec4(str, 0.0, 0.0, 1.0);');

		parser_material_finalize(con_paint);
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = node_shader_get(vert);
		con_paint.data.fragment_shader = node_shader_get(frag);

		return con_paint;
	}
}
