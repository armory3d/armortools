
class MakeSculpt {

	static run = (data: TMaterial, matcon: material_context_t): NodeShaderContextRaw => {
		let context_id = "paint";
		let con_paint = NodeShaderContext.create(data, {
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
		con_paint.allow_vcols = mesh_data_get_vertex_array(Context.raw.paintObject.data, "col") != null;

		let vert = NodeShaderContext.make_vert(con_paint);
		let frag = NodeShaderContext.make_frag(con_paint);
		frag.ins = vert.outs;

		let faceFill = Context.raw.tool == WorkspaceTool.ToolFill && Context.raw.fillTypeHandle.position == FillType.FillFace;
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;

		NodeShader.add_out(vert, 'vec2 texCoord');
		NodeShader.write(vert, 'const vec2 madd = vec2(0.5, 0.5);');
		NodeShader.write(vert, 'texCoord = pos.xy * madd + madd;');
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(vert, 'texCoord.y = 1.0 - texCoord.y;');
		///end
		NodeShader.write(vert, 'gl_Position = vec4(pos.xy, 0.0, 1.0);');

		NodeShader.add_uniform(frag, 'vec4 inp', '_inputBrush');
		NodeShader.add_uniform(frag, 'vec4 inplast', '_inputBrushLast');

		NodeShader.add_uniform(frag, 'sampler2D gbufferD');

		NodeShader.add_out(frag, 'vec4 fragColor[2]');

		NodeShader.add_uniform(frag, 'float brushRadius', '_brushRadius');
		NodeShader.add_uniform(frag, 'float brushOpacity', '_brushOpacity');
		NodeShader.add_uniform(frag, 'float brushHardness', '_brushHardness');

		if (Context.raw.tool == WorkspaceTool.ToolBrush  ||
			Context.raw.tool == WorkspaceTool.ToolEraser ||
			Context.raw.tool == WorkspaceTool.ToolClone  ||
			Context.raw.tool == WorkspaceTool.ToolBlur   ||
			Context.raw.tool == WorkspaceTool.ToolSmudge   ||
			Context.raw.tool == WorkspaceTool.ToolParticle ||
			decal) {

			let depthReject = !Context.raw.xray;

			MakeBrush.run(vert, frag);
		}

		NodeShader.write(frag, 'vec3 basecol = vec3(1.0, 1.0, 1.0);');
		NodeShader.write(frag, 'float opacity = 1.0;');
		NodeShader.write(frag, 'if (opacity == 0.0) discard;');

		NodeShader.write(frag, 'float str = clamp((brushRadius - dist) * brushHardness * 400.0, 0.0, 1.0) * opacity;');

		NodeShader.add_uniform(frag, 'sampler2D texpaint_undo', '_texpaint_undo');
		NodeShader.write(frag, 'vec4 sample_undo = textureLod(texpaint_undo, texCoord, 0.0);');

		NodeShader.write(frag, 'if (sample_undo.r == 0 && sample_undo.g == 0 && sample_undo.b == 0) discard;');

		NodeShader.add_function(frag, ShaderFunctions.str_octahedronWrap);
		NodeShader.add_uniform(frag, 'sampler2D gbuffer0_undo');
		NodeShader.write(frag, 'vec2 g0_undo = textureLod(gbuffer0_undo, inp.xy, 0.0).rg;');
		NodeShader.write(frag, 'vec3 wn;');
		NodeShader.write(frag, 'wn.z = 1.0 - abs(g0_undo.x) - abs(g0_undo.y);');
		NodeShader.write(frag, 'wn.xy = wn.z >= 0.0 ? g0_undo.xy : octahedronWrap(g0_undo.xy);');
		NodeShader.write(frag, 'vec3 n = normalize(wn);');

		NodeShader.write(frag, 'fragColor[0] = vec4(sample_undo.rgb + n * 0.1 * str, 1.0);');

		NodeShader.write(frag, 'fragColor[1] = vec4(str, 0.0, 0.0, 1.0);');

		ParserMaterial.finalize(con_paint);
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = NodeShader.get(vert);
		con_paint.data.fragment_shader = NodeShader.get(frag);

		return con_paint;
	}
}
