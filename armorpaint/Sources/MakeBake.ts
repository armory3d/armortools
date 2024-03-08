
class MakeBake {

	static run = (con: NodeShaderContextRaw, vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		if (Context.raw.bake_type == bake_type_t.AO) { // Voxel
			///if arm_voxels
			// Apply normal channel
			frag.wposition = true;
			frag.n = true;
			frag.vvec = true;
			NodeShader.add_function(frag, ShaderFunctions.str_cotangent_frame);
			///if krom_direct3d11
			NodeShader.write(frag, 'mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			///else
			NodeShader.write(frag, 'mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			///end
			NodeShader.write(frag, 'n = nortan * 2.0 - 1.0;');
			NodeShader.write(frag, 'n.y = -n.y;');
			NodeShader.write(frag, 'n = normalize(mul(n, TBN));');

			NodeShader.write(frag, MakeMaterial.voxelgi_half_extents());
			NodeShader.write(frag, 'vec3 voxpos = wposition / voxelgiHalfExtents;');
			NodeShader.add_uniform(frag, 'sampler3D voxels');
			NodeShader.add_function(frag, ShaderFunctions.str_trace_ao);
			frag.n = true;
			let strength: f32 = Context.raw.bake_ao_strength;
			let radius: f32 = Context.raw.bake_ao_radius;
			let offset: f32 = Context.raw.bake_ao_offset;
			NodeShader.write(frag, `float ao = traceAO(voxpos, n, ${radius}, ${offset}) * ${strength};`);
			if (Context.raw.bake_axis != bake_axis_t.XYZ) {
				let axis: string = MakeBake.axis_string(Context.raw.bake_axis);
				NodeShader.write(frag, `ao *= dot(n, ${axis});`);
			}
			NodeShader.write(frag, 'ao = 1.0 - ao;');
			NodeShader.write(frag, 'fragColor[0] = vec4(ao, ao, ao, 1.0);');
			///end
		}
		else if (Context.raw.bake_type == bake_type_t.CURVATURE) {
			let pass: bool = ParserMaterial.bake_passthrough;
			let strength: string = pass ? ParserMaterial.bake_passthrough_strength : Context.raw.bake_curv_strength + "";
			let radius: string = pass ? ParserMaterial.bake_passthrough_radius : Context.raw.bake_curv_radius + "";
			let offset: string = pass ? ParserMaterial.bake_passthrough_offset : Context.raw.bake_curv_offset + "";
			strength = `float(${strength})`;
			radius = `float(${radius})`;
			offset = `float(${offset})`;
			frag.n = true;
			NodeShader.write(frag, 'vec3 dx = dFdx(n);');
			NodeShader.write(frag, 'vec3 dy = dFdy(n);');
			NodeShader.write(frag, 'float curvature = max(dot(dx, dx), dot(dy, dy));');
			NodeShader.write(frag, 'curvature = clamp(pow(curvature, (1.0 / ' + radius + ') * 0.25) * ' + strength + ' * 2.0 + ' + offset + ' / 10.0, 0.0, 1.0);');
			if (Context.raw.bake_axis != bake_axis_t.XYZ) {
				let axis: string = MakeBake.axis_string(Context.raw.bake_axis);
				NodeShader.write(frag, `curvature *= dot(n, ${axis});`);
			}
			NodeShader.write(frag, 'fragColor[0] = vec4(curvature, curvature, curvature, 1.0);');
		}
		else if (Context.raw.bake_type == bake_type_t.NORMAL) { // Tangent
			frag.n = true;
			NodeShader.add_uniform(frag, 'sampler2D texpaint_undo', '_texpaint_undo'); // Baked high-poly normals
			NodeShader.write(frag, 'vec3 n0 = textureLod(texpaint_undo, texCoord, 0.0).rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			NodeShader.add_function(frag, ShaderFunctions.str_cotangent_frame);
			NodeShader.write(frag, 'mat3 invTBN = transpose(cotangentFrame(n, n, texCoord));');
			NodeShader.write(frag, 'vec3 res = normalize(mul(n0, invTBN)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
			NodeShader.write(frag, 'fragColor[0] = vec4(res, 1.0);');
		}
		else if (Context.raw.bake_type == bake_type_t.NORMAL_OBJECT) {
			frag.n = true;
			NodeShader.write(frag, 'fragColor[0] = vec4(n * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			if (Context.raw.bake_up_axis == bake_up_axis_t.Y) {
				NodeShader.write(frag, 'fragColor[0].rgb = vec3(fragColor[0].r, fragColor[0].b, 1.0 - fragColor[0].g);');
			}
		}
		else if (Context.raw.bake_type == bake_type_t.HEIGHT) {
			frag.wposition = true;
			NodeShader.add_uniform(frag, 'sampler2D texpaint_undo', '_texpaint_undo'); // Baked high-poly positions
			NodeShader.write(frag, 'vec3 wpos0 = textureLod(texpaint_undo, texCoord, 0.0).rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			NodeShader.write(frag, 'float res = distance(wpos0, wposition) * 10.0;');
			NodeShader.write(frag, 'fragColor[0] = vec4(res, res, res, 1.0);');
		}
		else if (Context.raw.bake_type == bake_type_t.DERIVATIVE) {
			NodeShader.add_uniform(frag, 'sampler2D texpaint_undo', '_texpaint_undo'); // Baked height
			NodeShader.write(frag, 'vec2 texDx = dFdx(texCoord);');
			NodeShader.write(frag, 'vec2 texDy = dFdy(texCoord);');
			NodeShader.write(frag, 'float h0 = textureLod(texpaint_undo, texCoord, 0.0).r * 100;');
			NodeShader.write(frag, 'float h1 = textureLod(texpaint_undo, texCoord + texDx, 0.0).r * 100;');
			NodeShader.write(frag, 'float h2 = textureLod(texpaint_undo, texCoord + texDy, 0.0).r * 100;');
			NodeShader.write(frag, 'fragColor[0] = vec4((h1 - h0) * 0.5 + 0.5, (h2 - h0) * 0.5 + 0.5, 0.0, 1.0);');
		}
		else if (Context.raw.bake_type == bake_type_t.POSITION) {
			frag.wposition = true;
			NodeShader.write(frag, 'fragColor[0] = vec4(wposition * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			if (Context.raw.bake_up_axis == bake_up_axis_t.Y) {
				NodeShader.write(frag, 'fragColor[0].rgb = vec3(fragColor[0].r, fragColor[0].b, 1.0 - fragColor[0].g);');
			}
		}
		else if (Context.raw.bake_type == bake_type_t.TEXCOORD) {
			NodeShader.write(frag, 'fragColor[0] = vec4(texCoord.xy, 0.0, 1.0);');
		}
		else if (Context.raw.bake_type == bake_type_t.MATERIALID) {
			NodeShader.add_uniform(frag, 'sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			NodeShader.write(frag, 'float sample_matid = textureLod(texpaint_nor_undo, texCoord, 0.0).a + 1.0 / 255.0;');
			NodeShader.write(frag, 'float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
			NodeShader.write(frag, 'float matid_g = fract(sin(dot(vec2(sample_matid * 20.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
			NodeShader.write(frag, 'float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
			NodeShader.write(frag, 'fragColor[0] = vec4(matid_r, matid_g, matid_b, 1.0);');
		}
		else if (Context.raw.bake_type == bake_type_t.OBJECTID) {
			NodeShader.add_uniform(frag, 'float objectId', '_objectId');
			NodeShader.write(frag, 'float obid = objectId + 1.0 / 255.0;');
			NodeShader.write(frag, 'float id_r = fract(sin(dot(vec2(obid, obid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
			NodeShader.write(frag, 'float id_g = fract(sin(dot(vec2(obid * 20.0, obid), vec2(12.9898, 78.233))) * 43758.5453);');
			NodeShader.write(frag, 'float id_b = fract(sin(dot(vec2(obid, obid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
			NodeShader.write(frag, 'fragColor[0] = vec4(id_r, id_g, id_b, 1.0);');
		}
		else if (Context.raw.bake_type == bake_type_t.VERTEX_COLOR) {
			if (con.allow_vcols) {
				NodeShaderContext.add_elem(con, "col", "short4norm");
				NodeShader.write(frag, 'fragColor[0] = vec4(vcolor.r, vcolor.g, vcolor.b, 1.0);');
			}
			else {
				NodeShader.write(frag, 'fragColor[0] = vec4(1.0, 1.0, 1.0, 1.0);');
			}
		}
	}

	static position_normal = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		NodeShader.add_out(vert, 'vec3 position');
		NodeShader.add_out(vert, 'vec3 normal');
		NodeShader.add_uniform(vert, 'mat4 W', '_world_matrix');
		NodeShader.write(vert, 'position = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;');
		NodeShader.write(vert, 'normal = vec3(nor.xy, pos.w);');
		NodeShader.write(vert, 'vec2 tpos = vec2(tex.x * 2.0 - 1.0, (1.0 - tex.y) * 2.0 - 1.0);');
		NodeShader.write(vert, 'gl_Position = vec4(tpos, 0.0, 1.0);');
		NodeShader.add_out(frag, 'vec4 fragColor[2]');
		NodeShader.write(frag, 'fragColor[0] = vec4(position, 1.0);');
		NodeShader.write(frag, 'fragColor[1] = vec4(normal, 1.0);');
	}

	static set_color_writes = (con_paint: NodeShaderContextRaw) => {
		// Bake into base color, disable other slots
		con_paint.data.color_writes_red[1] = false;
		con_paint.data.color_writes_green[1] = false;
		con_paint.data.color_writes_blue[1] = false;
		con_paint.data.color_writes_alpha[1] = false;
		con_paint.data.color_writes_red[2] = false;
		con_paint.data.color_writes_green[2] = false;
		con_paint.data.color_writes_blue[2] = false;
		con_paint.data.color_writes_alpha[2] = false;
	}

	static axis_string = (i: i32): string => {
		return i == bake_axis_t.X  ? "vec3(1,0,0)"  :
			   i == bake_axis_t.Y  ? "vec3(0,1,0)"  :
			   i == bake_axis_t.Z  ? "vec3(0,0,1)"  :
			   i == bake_axis_t.MX ? "vec3(-1,0,0)" :
			   i == bake_axis_t.MY ? "vec3(0,-1,0)" :
							 		  "vec3(0,0,-1)";
	}
}
