
function make_brush_run(kong: node_shader_t) {

	node_shader_write_frag(kong, "var dist: float = 0.0;");

	if (context_raw.tool == workspace_tool_t.PARTICLE) {
		return;
	}

	let fill_layer: bool = context_raw.layer.fill_layer != null;
	let decal: bool = context_is_decal();
	if (decal && !fill_layer) {
		node_shader_write_frag(kong, "if (constants.decal_mask.z > 0.0) {");
	}

	if (config_raw.brush_3d) {
		node_shader_write_frag(kong, "var depth: float = sample_lod(gbufferD, sampler_linear, constants.inp.xy, 0.0).r;");

		node_shader_add_constant(kong, "invVP: float4x4", "_inv_view_proj_matrix");
		node_shader_write_frag(kong, "var winp: float4 = float4(float2(constants.inp.x, 1.0 - constants.inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "winp = constants.invVP * winp;");
		node_shader_write_frag(kong, "winp.xyz = winp.xyz / winp.w;");

		kong.frag_wposition = true;

		if (config_raw.brush_angle_reject || context_raw.xray) {
			node_shader_add_function(kong, str_octahedron_wrap);
			node_shader_add_texture(kong, "gbuffer0");
			node_shader_write_frag(kong, "var g0: float2 = sample_lod(gbuffer0, sampler_linear, constants.inp.xy, 0.0).rg;");
			node_shader_write_frag(kong, "var wn: float3;");
			node_shader_write_frag(kong, "wn.z = 1.0 - abs(g0.x) - abs(g0.y);");
			// node_shader_write_frag(kong, "wn.xy = wn.z >= 0.0 ? g0.xy : octahedron_wrap(g0.xy);");
			node_shader_write_frag(kong, "if (wn.z >= 0.0) { wn.x = g0.x; wn.y = g0.y; } else { var f2: float2 = octahedron_wrap(g0.xy); wn.x = f2.x; wn.y = f2.y; }");
			node_shader_write_frag(kong, "wn = normalize(wn);");
			node_shader_write_frag(kong, "var plane_dist: float = dot(wn, winp.xyz - input.wposition);");

			if (config_raw.brush_angle_reject && !context_raw.xray) {
				// constants.inp.w = paint2d ? 0.0 : 1.0
				node_shader_write_frag(kong, "if (plane_dist < -0.01 && constants.inp.w == 0.0) { discard; }");
				kong.frag_n = true;
				let angle: f32 = context_raw.brush_angle_reject_dot;
				node_shader_write_frag(kong, "if (dot(wn, n) < " + angle + " && constants.inp.w == 0.0) { discard; }");
			}
		}

		node_shader_write_frag(kong, "var depthlast: float = sample_lod(gbufferD, sampler_linear, constants.inplast.xy, 0.0).r;");

		node_shader_write_frag(kong, "var winplast: float4 = float4(float2(constants.inplast.x, 1.0 - constants.inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "winplast = constants.invVP * winplast;");
		node_shader_write_frag(kong, "winplast.xyz = winplast.xyz / winplast.w;");

		node_shader_write_frag(kong, "var pa: float3 = input.wposition - winp.xyz;");
		if (context_raw.xray) {
			node_shader_write_frag(kong, "pa += wn * float3(plane_dist, plane_dist, plane_dist);");
		}
		node_shader_write_frag(kong, "var ba: float3 = winplast.xyz - winp.xyz;");

		if (context_raw.brush_lazy_radius > 0 && context_raw.brush_lazy_step > 0) {
			// Sphere
			node_shader_write_frag(kong, "dist = distance(input.wposition, winp.xyz);");
		}
		else {
			// Capsule
			node_shader_write_frag(kong, "var h: float = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);");
			node_shader_write_frag(kong, "dist = length(pa - ba * h);");
		}
	}
	else { // !brush3d
		node_shader_write_frag(kong, "var binp: float2 = constants.inp.xy * 2.0 - 1.0;");
		node_shader_write_frag(kong, "binp.x *= constants.aspect_ratio;");
		node_shader_write_frag(kong, "binp = binp * 0.5 + 0.5;");

		node_shader_write_frag(kong, "var binplast: float2 = constants.inplast.xy * 2.0 - 1.0;");
		node_shader_write_frag(kong, "binplast.x *= constants.aspect_ratio;");
		node_shader_write_frag(kong, "binplast = binplast * 0.5 + 0.5;");

		node_shader_write_frag(kong, "var pa: float2 = bsp.xy - binp.xy;");
		node_shader_write_frag(kong, "var ba: float2 = binplast.xy - binp.xy;");
		node_shader_write_frag(kong, "var h: float = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);");
		node_shader_write_frag(kong, "dist = length(pa - ba * h);");
	}

	node_shader_write_frag(kong, "if (dist > constants.brush_radius) { discard; }");

	if (decal && !fill_layer) {
		node_shader_write_frag(kong, "}");
	}
}
