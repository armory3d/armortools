
function make_brush_run(vert: node_shader_t, frag: node_shader_t) {

	node_shader_write(frag, "float dist = 0.0;");

	if (context_raw.tool == workspace_tool_t.PARTICLE) {
		return;
	}

	let fill_layer: bool = context_raw.layer.fill_layer != null;
	let decal: bool = context_is_decal();
	if (decal && !fill_layer) {
		node_shader_write(frag, "if (decal_mask.z > 0.0) {");
	}

	if (config_raw.brush_3d) {
		node_shader_write(frag, "float depth = textureLod(gbufferD, inp.xy, 0.0).r;");

		node_shader_add_uniform(frag, "mat4 invVP", "_inv_view_proj_matrix");
		node_shader_write(frag, "vec4 winp = vec4(vec2(inp.x, 1.0 - inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);");
		node_shader_write(frag, "winp = mul(winp, invVP);");
		node_shader_write(frag, "winp.xyz /= winp.w;");
		frag.wposition = true;

		if (config_raw.brush_angle_reject || context_raw.xray) {
			node_shader_add_function(frag, str_octahedron_wrap);
			node_shader_add_uniform(frag, "sampler2D gbuffer0");
			node_shader_write(frag, "vec2 g0 = textureLod(gbuffer0, inp.xy, 0.0).rg;");
			node_shader_write(frag, "vec3 wn;");
			node_shader_write(frag, "wn.z = 1.0 - abs(g0.x) - abs(g0.y);");
			node_shader_write(frag, "wn.xy = wn.z >= 0.0 ? g0.xy : octahedron_wrap(g0.xy);");
			node_shader_write(frag, "wn = normalize(wn);");
			node_shader_write(frag, "float plane_dist = dot(wn, winp.xyz - wposition);");

			if (config_raw.brush_angle_reject && !context_raw.xray) {
				node_shader_write(frag, "if (plane_dist < -0.01) discard;");
				frag.n = true;
				let angle: f32 = context_raw.brush_angle_reject_dot;
				node_shader_write(frag, "if (dot(wn, n) < " + angle + ") discard;");
			}
		}

		node_shader_write(frag, "float depthlast = textureLod(gbufferD, inplast.xy, 0.0).r;");

		node_shader_write(frag, "vec4 winplast = vec4(vec2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);");
		node_shader_write(frag, "winplast = mul(winplast, invVP);");
		node_shader_write(frag, "winplast.xyz /= winplast.w;");

		node_shader_write(frag, "vec3 pa = wposition - winp.xyz;");
		if (context_raw.xray) {
			node_shader_write(frag, "pa += wn * vec3(plane_dist, plane_dist, plane_dist);");
		}
		node_shader_write(frag, "vec3 ba = winplast.xyz - winp.xyz;");

		if (context_raw.brush_lazy_radius > 0 && context_raw.brush_lazy_step > 0) {
			// Sphere
			node_shader_write(frag, "dist = distance(wposition, winp.xyz);");
		}
		else {
			// Capsule
			node_shader_write(frag, "float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);");
			node_shader_write(frag, "dist = length(pa - ba * h);");
		}
	}
	else { // !brush3d
		node_shader_write(frag, "vec2 binp = inp.xy * 2.0 - 1.0;");
		node_shader_write(frag, "binp.x *= aspect_ratio;");
		node_shader_write(frag, "binp = binp * 0.5 + 0.5;");

		node_shader_write(frag, "vec2 binplast = inplast.xy * 2.0 - 1.0;");
		node_shader_write(frag, "binplast.x *= aspect_ratio;");
		node_shader_write(frag, "binplast = binplast * 0.5 + 0.5;");

		node_shader_write(frag, "vec2 pa = bsp.xy - binp.xy;");
		node_shader_write(frag, "vec2 ba = binplast.xy - binp.xy;");
		node_shader_write(frag, "float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);");
		node_shader_write(frag, "dist = length(pa - ba * h);");
	}

	node_shader_write(frag, "if (dist > brush_radius) discard;");

	if (decal && !fill_layer) {
		node_shader_write(frag, "}");
	}
}
