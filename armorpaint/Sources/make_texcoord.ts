
function make_texcoord_run(vert: NodeShaderRaw, frag: NodeShaderRaw) {

	let fill_layer: bool = context_raw.layer.fill_layer != null;
	let uv_type: uv_type_t = fill_layer ? context_raw.layer.uv_type : context_raw.brush_paint;
	let decal: bool = context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT;
	let angle: f32 = context_raw.brush_angle + context_raw.brush_nodes_angle;
	let uvAngle: f32 = fill_layer ? context_raw.layer.angle : angle;

	if (uv_type == uv_type_t.PROJECT || decal) { // TexCoords - project
		node_shader_add_uniform(frag, 'float brushScale', '_brushScale');
		node_shader_write_attrib(frag, 'vec2 uvsp = sp.xy;');

		if (fill_layer) { // Decal layer
			node_shader_write_attrib(frag, 'if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;');

			if (uvAngle != 0.0) {
				node_shader_add_uniform(frag, 'vec2 brushAngle', '_brushAngle');
				node_shader_write_attrib(frag, 'uvsp = vec2(uvsp.x * brushAngle.x - uvsp.y * brushAngle.y, uvsp.x * brushAngle.y + uvsp.y * brushAngle.x);');
			}

			frag.n = true;
			node_shader_add_uniform(frag, 'vec3 decalLayerNor', '_decalLayerNor');
			let dot_angle: f32 = context_raw.brush_angle_reject_dot;
			node_shader_write(frag, `if (abs(dot(n, decalLayerNor) - 1.0) > ${dot_angle}) discard;`);

			frag.wposition = true;
			node_shader_add_uniform(frag, 'vec3 decalLayerLoc', '_decalLayerLoc');
			node_shader_add_uniform(frag, 'float decalLayerDim', '_decalLayerDim');
			node_shader_write_attrib(frag, 'if (abs(dot(decalLayerNor, decalLayerLoc - wposition)) > decalLayerDim) discard;');
		}
		else if (decal) {
			node_shader_add_uniform(frag, 'vec4 decalMask', '_decalMask');
			node_shader_write_attrib(frag, 'uvsp -= decalMask.xy;');
			node_shader_write_attrib(frag, 'uvsp.x *= aspectRatio;');
			node_shader_write_attrib(frag, 'uvsp *= 0.21 / (decalMask.w * 0.9);'); // Decal radius

			if (context_raw.brush_directional) {
				node_shader_add_uniform(frag, 'vec3 brushDirection', '_brushDirection');
				node_shader_write_attrib(frag, 'if (brushDirection.z == 0.0) discard;');
				node_shader_write_attrib(frag, 'uvsp = vec2(uvsp.x * brushDirection.x - uvsp.y * brushDirection.y, uvsp.x * brushDirection.y + uvsp.y * brushDirection.x);');
			}

			if (uvAngle != 0.0) {
				node_shader_add_uniform(frag, 'vec2 brushAngle', '_brushAngle');
				node_shader_write_attrib(frag, 'uvsp = vec2(uvsp.x * brushAngle.x - uvsp.y * brushAngle.y, uvsp.x * brushAngle.y + uvsp.y * brushAngle.x);');
			}

			node_shader_add_uniform(frag, 'float brushScaleX', '_brushScaleX');
			node_shader_write_attrib(frag, 'uvsp.x *= brushScaleX;');

			node_shader_write_attrib(frag, 'uvsp += vec2(0.5, 0.5);');

			node_shader_write_attrib(frag, 'if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;');
		}
		else {
			node_shader_write_attrib(frag, 'uvsp.x *= aspectRatio;');

			if (uvAngle != 0.0) {
				node_shader_add_uniform(frag, 'vec2 brushAngle', '_brushAngle');
				node_shader_write_attrib(frag, 'uvsp = vec2(uvsp.x * brushAngle.x - uvsp.y * brushAngle.y, uvsp.x * brushAngle.y + uvsp.y * brushAngle.x);');
			}
		}

		node_shader_write_attrib(frag, 'vec2 texCoord = uvsp * brushScale;');
	}
	else if (uv_type == uv_type_t.UVMAP) { // TexCoords - uvmap
		node_shader_add_uniform(vert, 'float brushScale', '_brushScale');
		node_shader_add_out(vert, 'vec2 texCoord');
		node_shader_write(vert, 'texCoord = tex * brushScale;');

		if (uvAngle > 0.0) {
			node_shader_add_uniform(vert, 'vec2 brushAngle', '_brushAngle');
			node_shader_write(vert, 'texCoord = vec2(texCoord.x * brushAngle.x - texCoord.y * brushAngle.y, texCoord.x * brushAngle.y + texCoord.y * brushAngle.x);');
		}
	}
	else { // TexCoords - triplanar
		frag.wposition = true;
		frag.n = true;
		node_shader_add_uniform(frag, 'float brushScale', '_brushScale');
		node_shader_write_attrib(frag, 'vec3 triWeight = wnormal * wnormal;'); // n * n
		node_shader_write_attrib(frag, 'float triMax = max(triWeight.x, max(triWeight.y, triWeight.z));');
		node_shader_write_attrib(frag, 'triWeight = max(triWeight - triMax * 0.75, 0.0);');
		node_shader_write_attrib(frag, 'vec3 texCoordBlend = triWeight * (1.0 / (triWeight.x + triWeight.y + triWeight.z));');
		node_shader_write_attrib(frag, 'vec2 texCoord = wposition.yz * brushScale * 0.5;');
		node_shader_write_attrib(frag, 'vec2 texCoord1 = wposition.xz * brushScale * 0.5;');
		node_shader_write_attrib(frag, 'vec2 texCoord2 = wposition.xy * brushScale * 0.5;');

		if (uvAngle != 0.0) {
			node_shader_add_uniform(frag, 'vec2 brushAngle', '_brushAngle');
			node_shader_write_attrib(frag, 'texCoord = vec2(texCoord.x * brushAngle.x - texCoord.y * brushAngle.y, texCoord.x * brushAngle.y + texCoord.y * brushAngle.x);');
			node_shader_write_attrib(frag, 'texCoord1 = vec2(texCoord1.x * brushAngle.x - texCoord1.y * brushAngle.y, texCoord1.x * brushAngle.y + texCoord1.y * brushAngle.x);');
			node_shader_write_attrib(frag, 'texCoord2 = vec2(texCoord2.x * brushAngle.x - texCoord2.y * brushAngle.y, texCoord2.x * brushAngle.y + texCoord2.y * brushAngle.x);');
		}
	}
}
