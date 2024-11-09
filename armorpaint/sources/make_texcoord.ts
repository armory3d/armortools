
function make_texcoord_run(vert: node_shader_t, frag: node_shader_t) {

	let fill_layer: bool = context_raw.layer.fill_layer != null;
	let uv_type: uv_type_t = fill_layer ? context_raw.layer.uv_type : context_raw.brush_paint;
	let decal: bool = context_is_decal();
	let angle: f32 = context_raw.brush_angle + context_raw.brush_nodes_angle;
	let uv_angle: f32 = fill_layer ? context_raw.layer.angle : angle;

	if (uv_type == uv_type_t.PROJECT || decal) { // TexCoords - project
		node_shader_add_uniform(frag, "float brush_scale", "_brush_scale");
		node_shader_write_attrib(frag, "vec2 uvsp = sp.xy;");

		if (fill_layer) { // Decal layer
			node_shader_write_attrib(frag, "if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;");

			if (uv_angle != 0.0) {
				node_shader_add_uniform(frag, "vec2 brush_angle", "_brush_angle");
				node_shader_write_attrib(frag, "uvsp = vec2(uvsp.x * brush_angle.x - uvsp.y * brush_angle.y, uvsp.x * brush_angle.y + uvsp.y * brush_angle.x);");
			}

			frag.n = true;
			node_shader_add_uniform(frag, "vec3 decal_layer_nor", "_decal_layer_nor");
			let dot_angle: f32 = context_raw.brush_angle_reject_dot;
			node_shader_write(frag, "if (abs(dot(n, decal_layer_nor) - 1.0) > " + dot_angle + ") discard;");

			frag.wposition = true;
			node_shader_add_uniform(frag, "vec3 decal_layer_loc", "_decal_layer_loc");
			node_shader_add_uniform(frag, "float decal_layer_dim", "_decal_layer_dim");
			node_shader_write_attrib(frag, "if (abs(dot(decal_layer_nor, decal_layer_loc - wposition)) > decal_layer_dim) discard;");
		}
		else if (decal) {
			node_shader_add_uniform(frag, "vec4 decal_mask", "_decal_mask");
			node_shader_write_attrib(frag, "uvsp -= decal_mask.xy;");
			node_shader_write_attrib(frag, "uvsp.x *= aspect_ratio;");
			node_shader_write_attrib(frag, "uvsp *= 0.21 / (decal_mask.w * 0.9);"); // Decal radius

			if (context_raw.brush_directional) {
				node_shader_add_uniform(frag, "vec3 brush_direction", "_brush_direction");
				node_shader_write_attrib(frag, "if (brush_direction.z == 0.0) discard;");
				node_shader_write_attrib(frag, "uvsp = vec2(uvsp.x * brush_direction.x - uvsp.y * brush_direction.y, uvsp.x * brush_direction.y + uvsp.y * brush_direction.x);");
			}

			if (uv_angle != 0.0) {
				node_shader_add_uniform(frag, "vec2 brush_angle", "_brush_angle");
				node_shader_write_attrib(frag, "uvsp = vec2(uvsp.x * brush_angle.x - uvsp.y * brush_angle.y, uvsp.x * brush_angle.y + uvsp.y * brush_angle.x);");
			}

			node_shader_add_uniform(frag, "float brush_scale_x", "_brush_scale_x");
			node_shader_write_attrib(frag, "uvsp.x *= brush_scale_x;");

			node_shader_write_attrib(frag, "uvsp += vec2(0.5, 0.5);");

			node_shader_write_attrib(frag, "if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;");
		}
		else {
			node_shader_write_attrib(frag, "uvsp.x *= aspect_ratio;");

			if (uv_angle != 0.0) {
				node_shader_add_uniform(frag, "vec2 brush_angle", "_brush_angle");
				node_shader_write_attrib(frag, "uvsp = vec2(uvsp.x * brush_angle.x - uvsp.y * brush_angle.y, uvsp.x * brush_angle.y + uvsp.y * brush_angle.x);");
			}
		}

		node_shader_write_attrib(frag, "vec2 tex_coord = uvsp * brush_scale;");
	}
	else if (uv_type == uv_type_t.UVMAP) { // TexCoords - uvmap
		node_shader_add_uniform(vert, "float brush_scale", "_brush_scale");
		node_shader_add_out(vert, "vec2 tex_coord");
		node_shader_write(vert, "tex_coord = tex * brush_scale;");

		if (uv_angle > 0.0) {
			node_shader_add_uniform(vert, "vec2 brush_angle", "_brush_angle");
			node_shader_write(vert, "tex_coord = vec2(tex_coord.x * brush_angle.x - tex_coord.y * brush_angle.y, tex_coord.x * brush_angle.y + tex_coord.y * brush_angle.x);");
		}
	}
	else { // TexCoords - triplanar
		frag.wposition = true;
		frag.n = true;
		node_shader_add_uniform(frag, "float brush_scale", "_brush_scale");
		node_shader_write_attrib(frag, "vec3 tri_weight = wnormal * wnormal;"); // n * n
		node_shader_write_attrib(frag, "float tri_max = max(tri_weight.x, max(tri_weight.y, tri_weight.z));");
		node_shader_write_attrib(frag, "tri_weight = max(tri_weight - tri_max * 0.75, 0.0);");
		node_shader_write_attrib(frag, "vec3 tex_coord_blend = tri_weight * (1.0 / (tri_weight.x + tri_weight.y + tri_weight.z));");
		node_shader_write_attrib(frag, "vec2 tex_coord = wposition.yz * brush_scale * 0.5;");
		node_shader_write_attrib(frag, "vec2 tex_coord1 = wposition.xz * brush_scale * 0.5;");
		node_shader_write_attrib(frag, "vec2 tex_coord2 = wposition.xy * brush_scale * 0.5;");

		if (uv_angle != 0.0) {
			node_shader_add_uniform(frag, "vec2 brush_angle", "_brush_angle");
			node_shader_write_attrib(frag, "tex_coord = vec2(tex_coord.x * brush_angle.x - tex_coord.y * brush_angle.y, tex_coord.x * brush_angle.y + tex_coord.y * brush_angle.x);");
			node_shader_write_attrib(frag, "tex_coord1 = vec2(tex_coord1.x * brush_angle.x - tex_coord1.y * brush_angle.y, tex_coord1.x * brush_angle.y + tex_coord1.y * brush_angle.x);");
			node_shader_write_attrib(frag, "tex_coord2 = vec2(tex_coord2.x * brush_angle.x - tex_coord2.y * brush_angle.y, tex_coord2.x * brush_angle.y + tex_coord2.y * brush_angle.x);");
		}
	}
}
