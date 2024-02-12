
class MakePaint {

	static run = (data: TMaterial, matcon: material_context_t): NodeShaderContextRaw => {
		let con_paint = NodeShaderContext.create(data, {
			name: "paint",
			depth_write: false,
			compare_mode: "always", // TODO: align texcoords winding order
			// cull_mode: "counter_clockwise",
			cull_mode: "none",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}],
			color_attachments:
				Context.raw.tool == WorkspaceTool.ToolPicker ? ["RGBA32", "RGBA32", "RGBA32", "RGBA32"] :
					["RGBA32", "RGBA32", "RGBA32", "R8"]
		});

		con_paint.data.color_writes_red = [true, true, true, true];
		con_paint.data.color_writes_green = [true, true, true, true];
		con_paint.data.color_writes_blue = [true, true, true, true];
		con_paint.data.color_writes_alpha = [true, true, true, true];
		con_paint.allow_vcols = mesh_data_get_vertex_array(Context.raw.paintObject.data, "col") != null;

		let vert = NodeShaderContext.make_vert(con_paint);
		let frag = NodeShaderContext.make_frag(con_paint);
		frag.ins = vert.outs;

		if (Context.raw.tool == WorkspaceTool.ToolPicker) {
			// Mangle vertices to form full screen triangle
			NodeShader.write(vert, 'gl_Position = vec4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);');

			NodeShader.add_uniform(frag, 'sampler2D gbuffer2');
			NodeShader.add_uniform(frag, 'vec2 gbufferSize', '_gbufferSize');
			NodeShader.add_uniform(frag, 'vec4 inp', '_inputBrush');

			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
			///else
			NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
			///end

			NodeShader.add_out(frag, 'vec4 fragColor[4]');
			NodeShader.add_uniform(frag, 'sampler2D texpaint');
			NodeShader.add_uniform(frag, 'sampler2D texpaint_nor');
			NodeShader.add_uniform(frag, 'sampler2D texpaint_pack');
			NodeShader.write(frag, 'fragColor[0] = textureLod(texpaint, texCoordInp, 0.0);');
			NodeShader.write(frag, 'fragColor[1] = textureLod(texpaint_nor, texCoordInp, 0.0);');
			NodeShader.write(frag, 'fragColor[2] = textureLod(texpaint_pack, texCoordInp, 0.0);');
			NodeShader.write(frag, 'fragColor[3].rg = texCoordInp.xy;');
			con_paint.data.shader_from_source = true;
			con_paint.data.vertex_shader = NodeShader.get(vert);
			con_paint.data.fragment_shader = NodeShader.get(frag);
			return con_paint;
		}

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(vert, 'vec2 tpos = vec2(tex.x * 2.0 - 1.0, (1.0 - tex.y) * 2.0 - 1.0);');
		// NodeShader.write(vert, 'vec2 tpos = vec2(frac(tex.x * texScale) * 2.0 - 1.0, (1.0 - frac(tex.y * texScale)) * 2.0 - 1.0);'); // 3D View
		///else
		NodeShader.write(vert, 'vec2 tpos = vec2(tex.xy * 2.0 - 1.0);');
		///end

		NodeShader.write(vert, 'gl_Position = vec4(tpos, 0.0, 1.0);');

		NodeShader.add_uniform(vert, 'mat4 WVP', '_world_view_proj_matrix');

		NodeShader.add_out(vert, 'vec4 ndc');
		NodeShader.write_attrib(vert, 'ndc = mul(vec4(pos.xyz, 1.0), WVP);');

		NodeShader.write_attrib(frag, 'vec3 sp = vec3((ndc.xyz / ndc.w) * 0.5 + 0.5);');
		NodeShader.write_attrib(frag, 'sp.y = 1.0 - sp.y;');
		NodeShader.write_attrib(frag, 'sp.z -= 0.0001;'); // small bias

		NodeShader.add_uniform(frag, 'vec4 inp', '_inputBrush');
		NodeShader.add_uniform(frag, 'vec4 inplast', '_inputBrushLast');
		NodeShader.add_uniform(frag, 'float aspectRatio', '_aspect_ratio_window');
		NodeShader.write(frag, 'vec2 bsp = sp.xy * 2.0 - 1.0;');
		NodeShader.write(frag, 'bsp.x *= aspectRatio;');
		NodeShader.write(frag, 'bsp = bsp * 0.5 + 0.5;');

		NodeShader.add_uniform(frag, 'sampler2D gbufferD');

		NodeShader.add_out(frag, 'vec4 fragColor[4]');

		NodeShader.add_uniform(frag, 'float brushRadius', '_brushRadius');
		NodeShader.add_uniform(frag, 'float brushOpacity', '_brushOpacity');
		NodeShader.add_uniform(frag, 'float brushHardness', '_brushHardness');

		if (Context.raw.tool == WorkspaceTool.ToolEraser ||
			Context.raw.tool == WorkspaceTool.ToolClone  ||
			Context.raw.tool == WorkspaceTool.ToolBlur   ||
			Context.raw.tool == WorkspaceTool.ToolSmudge) {

			NodeShader.write(frag, 'float dist = 0.0;');

			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(frag, 'float depth = textureLod(gbufferD, inp.xy, 0.0).r;');
			///else
			NodeShader.write(frag, 'float depth = textureLod(gbufferD, vec2(inp.x, 1.0 - inp.y), 0.0).r;');
			///end

			NodeShader.add_uniform(frag, 'mat4 invVP', '_inv_view_proj_matrix');
			NodeShader.write(frag, 'vec4 winp = vec4(vec2(inp.x, 1.0 - inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);');
			NodeShader.write(frag, 'winp = mul(winp, invVP);');
			NodeShader.write(frag, 'winp.xyz /= winp.w;');
			frag.wposition = true;

			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(frag, 'float depthlast = textureLod(gbufferD, inplast.xy, 0.0).r;');
			///else
			NodeShader.write(frag, 'float depthlast = textureLod(gbufferD, vec2(inplast.x, 1.0 - inplast.y), 0.0).r;');
			///end

			NodeShader.write(frag, 'vec4 winplast = vec4(vec2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);');
			NodeShader.write(frag, 'winplast = mul(winplast, invVP);');
			NodeShader.write(frag, 'winplast.xyz /= winplast.w;');

			NodeShader.write(frag, 'vec3 pa = wposition - winp.xyz;');
			NodeShader.write(frag, 'vec3 ba = winplast.xyz - winp.xyz;');

			// Capsule
			NodeShader.write(frag, 'float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
			NodeShader.write(frag, 'dist = length(pa - ba * h);');

			NodeShader.write(frag, 'if (dist > brushRadius) discard;');
		}

		// NodeShader.add_uniform(vert, 'float brushScale', '_brushScale');
		// NodeShader.add_uniform(vert, 'float texScale', '_tex_unpack');
		// NodeShader.add_out(vert, 'vec2 texCoord');
		// NodeShader.write(vert, 'texCoord = tex * brushScale * texScale;');

		if (Context.raw.tool == WorkspaceTool.ToolClone || Context.raw.tool == WorkspaceTool.ToolBlur || Context.raw.tool == WorkspaceTool.ToolSmudge) {
			NodeShader.add_uniform(frag, 'sampler2D gbuffer2');
			NodeShader.add_uniform(frag, 'vec2 gbufferSize', '_gbufferSize');
			NodeShader.add_uniform(frag, 'sampler2D texpaint_undo', '_texpaint_undo');
			NodeShader.add_uniform(frag, 'sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			NodeShader.add_uniform(frag, 'sampler2D texpaint_pack_undo', '_texpaint_pack_undo');

			if (Context.raw.tool == WorkspaceTool.ToolClone) {
				// NodeShader.add_uniform(frag, 'vec2 cloneDelta', '_cloneDelta');
				// ///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
				// NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp.xy + cloneDelta) * gbufferSize), 0).ba;');
				// ///else
				// NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp.x + cloneDelta.x) * gbufferSize.x, (1.0 - (sp.y + cloneDelta.y)) * gbufferSize.y), 0).ba;');
				// ///end

				// NodeShader.write(frag, 'vec3 texpaint_pack_sample = textureLod(texpaint_pack_undo, texCoordInp, 0.0).rgb;');
				// let base = 'textureLod(texpaint_undo, texCoordInp, 0.0).rgb';
				// let rough = 'texpaint_pack_sample.g';
				// let met = 'texpaint_pack_sample.b';
				// let occ = 'texpaint_pack_sample.r';
				// let nortan = 'textureLod(texpaint_nor_undo, texCoordInp, 0.0).rgb';
				// let height = '0.0';
				// let opac = '1.0';
				// NodeShader.write(frag, `vec3 basecol = ${base};`);
				// NodeShader.write(frag, `float roughness = ${rough};`);
				// NodeShader.write(frag, `float metallic = ${met};`);
				// NodeShader.write(frag, `float occlusion = ${occ};`);
				// NodeShader.write(frag, `vec3 nortan = ${nortan};`);
				// NodeShader.write(frag, `float height = ${height};`);
				// NodeShader.write(frag, `float mat_opacity = ${opac};`);
				// NodeShader.write(frag, 'float opacity = mat_opacity * brushOpacity;');
			}
			else { // Blur
				// ///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
				// NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, sp.y * gbufferSize.y), 0).ba;');
				// ///else
				// NodeShader.write(frag, 'vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, (1.0 - sp.y) * gbufferSize.y), 0).ba;');
				// ///end

				// NodeShader.write(frag, 'vec3 basecol = vec3(0.0, 0.0, 0.0);');
				// NodeShader.write(frag, 'float roughness = 0.0;');
				// NodeShader.write(frag, 'float metallic = 0.0;');
				// NodeShader.write(frag, 'float occlusion = 0.0;');
				// NodeShader.write(frag, 'vec3 nortan = vec3(0.0, 0.0, 0.0);');
				// NodeShader.write(frag, 'float height = 0.0;');
				// NodeShader.write(frag, 'float mat_opacity = 1.0;');
				// NodeShader.write(frag, 'float opacity = 0.0;');

				// NodeShader.add_uniform(frag, 'vec2 texpaintSize', '_texpaintSize');
				// NodeShader.write(frag, 'float blur_step = 1.0 / texpaintSize.x;');
				// if (Context.raw.blurDirectional) {
				// 	///if (krom_direct3d11 || krom_direct3d12 || krom_metal)
				// 	NodeShader.write(frag, 'const float blur_weight[7] = {1.0 / 28.0, 2.0 / 28.0, 3.0 / 28.0, 4.0 / 28.0, 5.0 / 28.0, 6.0 / 28.0, 7.0 / 28.0};');
				// 	///else
				// 	NodeShader.write(frag, 'const float blur_weight[7] = float[](1.0 / 28.0, 2.0 / 28.0, 3.0 / 28.0, 4.0 / 28.0, 5.0 / 28.0, 6.0 / 28.0, 7.0 / 28.0);');
				// 	///end
				// 	NodeShader.add_uniform(frag, 'vec3 brushDirection', '_brushDirection');
				// 	NodeShader.write(frag, 'vec2 blur_direction = brushDirection.yx;');
				// 	NodeShader.write(frag, 'for (int i = 0; i < 7; ++i) {');
				// 	///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
				// 	NodeShader.write(frag, 'vec2 texCoordInp2 = texelFetch(gbuffer2, ivec2((sp.x + blur_direction.x * blur_step * float(i)) * gbufferSize.x, (sp.y + blur_direction.y * blur_step * float(i)) * gbufferSize.y), 0).ba;');
				// 	///else
				// 	NodeShader.write(frag, 'vec2 texCoordInp2 = texelFetch(gbuffer2, ivec2((sp.x + blur_direction.x * blur_step * float(i)) * gbufferSize.x, (1.0 - (sp.y + blur_direction.y * blur_step * float(i))) * gbufferSize.y), 0).ba;');
				// 	///end
				// 	NodeShader.write(frag, 'vec4 texpaint_sample = texture(texpaint_undo, texCoordInp2);');
				// 	NodeShader.write(frag, 'opacity += texpaint_sample.a * blur_weight[i];');
				// 	NodeShader.write(frag, 'basecol += texpaint_sample.rgb * blur_weight[i];');
				// 	NodeShader.write(frag, 'vec4 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp2) * blur_weight[i];');
				// 	NodeShader.write(frag, 'roughness += texpaint_pack_sample.g;');
				// 	NodeShader.write(frag, 'metallic += texpaint_pack_sample.b;');
				// 	NodeShader.write(frag, 'occlusion += texpaint_pack_sample.r;');
				// 	NodeShader.write(frag, 'height += texpaint_pack_sample.a;');
				// 	NodeShader.write(frag, 'nortan += texture(texpaint_nor_undo, texCoordInp2).rgb * blur_weight[i];');
				// 	NodeShader.write(frag, '}');
				// }
				// else {
				// 	///if (krom_direct3d11 || krom_direct3d12 || krom_metal)
				// 	NodeShader.write(frag, 'const float blur_weight[15] = {0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0};');
				// 	///else
				// 	NodeShader.write(frag, 'const float blur_weight[15] = float[](0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0);');
				// 	///end
				// 	// X
				// 	NodeShader.write(frag, 'for (int i = -7; i <= 7; ++i) {');
				// 	NodeShader.write(frag, 'vec4 texpaint_sample = texture(texpaint_undo, texCoordInp + vec2(blur_step * float(i), 0.0));');
				// 	NodeShader.write(frag, 'opacity += texpaint_sample.a * blur_weight[i + 7];');
				// 	NodeShader.write(frag, 'basecol += texpaint_sample.rgb * blur_weight[i + 7];');
				// 	NodeShader.write(frag, 'vec4 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(blur_step * float(i), 0.0)) * blur_weight[i + 7];');
				// 	NodeShader.write(frag, 'roughness += texpaint_pack_sample.g;');
				// 	NodeShader.write(frag, 'metallic += texpaint_pack_sample.b;');
				// 	NodeShader.write(frag, 'occlusion += texpaint_pack_sample.r;');
				// 	NodeShader.write(frag, 'height += texpaint_pack_sample.a;');
				// 	NodeShader.write(frag, 'nortan += texture(texpaint_nor_undo, texCoordInp + vec2(blur_step * float(i), 0.0)).rgb * blur_weight[i + 7];');
				// 	NodeShader.write(frag, '}');
				// 	// Y
				// 	NodeShader.write(frag, 'for (int j = -7; j <= 7; ++j) {');
				// 	NodeShader.write(frag, 'vec4 texpaint_sample = texture(texpaint_undo, texCoordInp + vec2(0.0, blur_step * float(j)));');
				// 	NodeShader.write(frag, 'opacity += texpaint_sample.a * blur_weight[j + 7];');
				// 	NodeShader.write(frag, 'basecol += texpaint_sample.rgb * blur_weight[j + 7];');
				// 	NodeShader.write(frag, 'vec4 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(0.0, blur_step * float(j))) * blur_weight[j + 7];');
				// 	NodeShader.write(frag, 'roughness += texpaint_pack_sample.g;');
				// 	NodeShader.write(frag, 'metallic += texpaint_pack_sample.b;');
				// 	NodeShader.write(frag, 'occlusion += texpaint_pack_sample.r;');
				// 	NodeShader.write(frag, 'height += texpaint_pack_sample.a;');
				// 	NodeShader.write(frag, 'nortan += texture(texpaint_nor_undo, texCoordInp + vec2(0.0, blur_step * float(j))).rgb * blur_weight[j + 7];');
				// 	NodeShader.write(frag, '}');
				// }
				// NodeShader.write(frag, 'opacity *= brushOpacity;');
			}
		}

		NodeShader.write(frag, 'float opacity = 1.0;');
		NodeShader.write(frag, 'if (opacity == 0.0) discard;');

		NodeShader.write(frag, 'float str = clamp((brushRadius - dist) * brushHardness * 400.0, 0.0, 1.0) * opacity;');

		// Manual blending to preserve memory
		frag.wvpposition = true;
		NodeShader.write(frag, 'vec2 sample_tc = vec2(wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(frag, 'sample_tc.y = 1.0 - sample_tc.y;');
		///end
		NodeShader.add_uniform(frag, 'sampler2D paintmask');
		NodeShader.write(frag, 'float sample_mask = textureLod(paintmask, sample_tc, 0.0).r;');
		NodeShader.write(frag, 'str = max(str, sample_mask);');

		NodeShader.add_uniform(frag, 'sampler2D texpaint_undo', '_texpaint_undo');
		NodeShader.write(frag, 'vec4 sample_undo = textureLod(texpaint_undo, sample_tc, 0.0);');

		if (Context.raw.tool == WorkspaceTool.ToolEraser) {
			// NodeShader.write(frag, 'fragColor[0] = vec4(mix(sample_undo.rgb, vec3(0.0, 0.0, 0.0), str), sample_undo.a - str);');
			NodeShader.write(frag, 'fragColor[0] = vec4(0.0, 0.0, 0.0, 0.0);');
			NodeShader.write(frag, 'fragColor[1] = vec4(0.5, 0.5, 1.0, 0.0);');
			NodeShader.write(frag, 'fragColor[2] = vec4(1.0, 0.0, 0.0, 0.0);');
		}

		NodeShader.write(frag, 'fragColor[3] = vec4(str, 0.0, 0.0, 1.0);');

		ParserMaterial.finalize(con_paint);
		ParserMaterial.sample_keep_aspect = false;
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = NodeShader.get(vert);
		con_paint.data.fragment_shader = NodeShader.get(frag);

		return con_paint;
	}
}
