
function make_texcoord_run(kong: node_shader_t) {

	let fill_layer: bool = context_raw.layer.fill_layer != null;
	let uv_type: uv_type_t = fill_layer ? context_raw.layer.uv_type : context_raw.brush_paint;
	let decal: bool = context_is_decal();
	let angle: f32 = context_raw.brush_angle + context_raw.brush_nodes_angle;
	let uv_angle: f32 = fill_layer ? context_raw.layer.angle : angle;

	if (uv_type == uv_type_t.PROJECT || decal) { // TexCoords - project
		node_shader_add_constant(kong, "brush_scale: float", "_brush_scale");
		node_shader_write_attrib_frag(kong, "var uvsp: float2 = sp.xy;");

		if (fill_layer) { // Decal layer
			node_shader_write_attrib_frag(kong, "if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) { discard; }");

			if (uv_angle != 0.0) {
				node_shader_add_constant(kong, "brush_angle: float2", "_brush_angle");
				node_shader_write_attrib_frag(kong, "uvsp = float2(uvsp.x * constants.brush_angle.x - uvsp.y * constants.brush_angle.y, uvsp.x * constants.brush_angle.y + uvsp.y * constants.brush_angle.x);");
			}

			kong.frag_n = true;
			node_shader_add_constant(kong, "decal_layer_nor: float3", "_decal_layer_nor");
			let dot_angle: f32 = context_raw.brush_angle_reject_dot;
			node_shader_write_frag(kong, "if (abs(dot(n, constants.decal_layer_nor) - 1.0) > " + dot_angle + ") { discard; }");

			kong.frag_wposition = true;
			node_shader_add_constant(kong, "decal_layer_loc: float3", "_decal_layer_loc");
			node_shader_add_constant(kong, "decal_layer_dim: float", "_decal_layer_dim");
			node_shader_write_attrib_frag(kong, "if (abs(dot(constants.decal_layer_nor, constants.decal_layer_loc - input.wposition)) > constants.decal_layer_dim) { discard; }");
		}
		else if (decal) {
			node_shader_add_constant(kong, "decal_mask: float4", "_decal_mask");
			node_shader_write_attrib_frag(kong, "uvsp = uvsp - constants.decal_mask.xy;");
			node_shader_write_attrib_frag(kong, "uvsp.x *= constants.aspect_ratio;");
			node_shader_write_attrib_frag(kong, "uvsp = uvsp * (0.21 / (constants.decal_mask.w * 0.9));"); // Decal radius

			if (context_raw.brush_directional) {
				node_shader_add_constant(kong, "brush_direction: float3", "_brush_direction");
				node_shader_write_attrib_frag(kong, "if (constants.brush_direction.z == 0.0) { discard; }");
				node_shader_write_attrib_frag(kong, "uvsp = float2(uvsp.x * constants.brush_direction.x - uvsp.y * constants.brush_direction.y, uvsp.x * constants.brush_direction.y + uvsp.y * constants.brush_direction.x);");
			}

			if (uv_angle != 0.0) {
				node_shader_add_constant(kong, "brush_angle: float2", "_brush_angle");
				node_shader_write_attrib_frag(kong, "uvsp = float2(uvsp.x * constants.brush_angle.x - uvsp.y * constants.brush_angle.y, uvsp.x * constants.brush_angle.y + uvsp.y * constants.brush_angle.x);");
			}

			node_shader_add_constant(kong, "brush_scale_x: float", "_brush_scale_x");
			node_shader_write_attrib_frag(kong, "uvsp.x *= constants.brush_scale_x;");
			node_shader_write_attrib_frag(kong, "uvsp += float2(0.5, 0.5);");
			node_shader_write_attrib_frag(kong, "if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) { discard; }");
		}
		else {
			node_shader_write_attrib_frag(kong, "uvsp.x *= constants.aspect_ratio;");
			if (uv_angle != 0.0) {
				node_shader_add_constant(kong, "brush_angle: float2", "_brush_angle");
				node_shader_write_attrib_frag(kong, "uvsp = float2(uvsp.x * constants.brush_angle.x - uvsp.y * constants.brush_angle.y, uvsp.x * constants.brush_angle.y + uvsp.y * constants.brush_angle.x);");
			}
		}

		node_shader_write_attrib_frag(kong, "var tex_coord: float2 = uvsp * constants.brush_scale;");
	}
	else if (uv_type == uv_type_t.UVMAP) { // TexCoords - uvmap
		node_shader_add_constant(kong, "brush_scale: float", "_brush_scale");
		node_shader_add_out(kong, "tex_coord: float2");
		node_shader_write_vert(kong, "output.tex_coord = input.tex * constants.brush_scale;");
		if (uv_angle > 0.0) {
			node_shader_add_constant(kong, "brush_angle: float2", "_brush_angle");
			node_shader_write_vert(kong, "output.tex_coord = float2(output.tex_coord.x * constants.brush_angle.x - output.tex_coord.y * constants.brush_angle.y, output.tex_coord.x * constants.brush_angle.y + output.tex_coord.y * constants.brush_angle.x);");
		}
		node_shader_write_attrib_frag(kong, "var tex_coord: float2 = input.tex_coord;");
	}
	else { // TexCoords - triplanar
		kong.frag_wposition = true;
		kong.frag_n = true;
		node_shader_add_constant(kong, "brush_scale: float", "_brush_scale");
		node_shader_write_attrib_frag(kong, "var tri_weight: float3 = input.wnormal * input.wnormal;"); // n * n
		node_shader_write_attrib_frag(kong, "var tri_max: float = max(tri_weight.x, max(tri_weight.y, tri_weight.z));");
		node_shader_write_attrib_frag(kong, "tri_weight = max3(tri_weight - float3(tri_max * 0.75, tri_max * 0.75, tri_max * 0.75), float3(0.0, 0.0, 0.0));");
		node_shader_write_attrib_frag(kong, "var tex_coord_blend: float3 = tri_weight * (1.0 / (tri_weight.x + tri_weight.y + tri_weight.z));");
		node_shader_write_attrib_frag(kong, "var tex_coord: float2 = input.wposition.yz * constants.brush_scale * 0.5;");
		node_shader_write_attrib_frag(kong, "var tex_coord1: float2 = input.wposition.xz * constants.brush_scale * 0.5;");
		node_shader_write_attrib_frag(kong, "var tex_coord2: float2 = input.wposition.xy * constants.brush_scale * 0.5;");
		if (uv_angle != 0.0) {
			node_shader_add_constant(kong, "brush_angle: float2", "_brush_angle");
			node_shader_write_attrib_frag(kong, "tex_coord = float2(tex_coord.x * constants.brush_angle.x - tex_coord.y * constants.brush_angle.y, tex_coord.x * constants.brush_angle.y + tex_coord.y * constants.brush_angle.x);");
			node_shader_write_attrib_frag(kong, "tex_coord1 = float2(tex_coord1.x * constants.brush_angle.x - tex_coord1.y * constants.brush_angle.y, tex_coord1.x * constants.brush_angle.y + tex_coord1.y * constants.brush_angle.x);");
			node_shader_write_attrib_frag(kong, "tex_coord2 = float2(tex_coord2.x * constants.brush_angle.x - tex_coord2.y * constants.brush_angle.y, tex_coord2.x * constants.brush_angle.y + tex_coord2.y * constants.brush_angle.x);");
		}
	}
}
