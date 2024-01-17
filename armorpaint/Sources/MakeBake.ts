
class MakeBake {

	static run = (con: NodeShaderContext, vert: NodeShader, frag: NodeShader) => {
		if (Context.raw.bakeType == BakeType.BakeAO) { // Voxel
			///if arm_voxels
			// Apply normal channel
			frag.wposition = true;
			frag.n = true;
			frag.vVec = true;
			frag.add_function(ShaderFunctions.str_cotangentFrame);
			///if krom_direct3d11
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			///else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			///end
			frag.write('n = nortan * 2.0 - 1.0;');
			frag.write('n.y = -n.y;');
			frag.write('n = normalize(mul(n, TBN));');

			frag.write(MakeMaterial.voxelgiHalfExtents());
			frag.write('vec3 voxpos = wposition / voxelgiHalfExtents;');
			frag.add_uniform('sampler3D voxels');
			frag.add_function(ShaderFunctions.str_traceAO);
			frag.n = true;
			let strength = Context.raw.bakeAoStrength;
			let radius = Context.raw.bakeAoRadius;
			let offset = Context.raw.bakeAoOffset;
			frag.write(`float ao = traceAO(voxpos, n, ${radius}, ${offset}) * ${strength};`);
			if (Context.raw.bakeAxis != BakeAxis.BakeXYZ) {
				let axis = MakeBake.axisString(Context.raw.bakeAxis);
				frag.write(`ao *= dot(n, ${axis});`);
			}
			frag.write('ao = 1.0 - ao;');
			frag.write('fragColor[0] = vec4(ao, ao, ao, 1.0);');
			///end
		}
		else if (Context.raw.bakeType == BakeType.BakeCurvature) {
			let pass = ParserMaterial.bake_passthrough;
			let strength = pass ? ParserMaterial.bake_passthrough_strength : Context.raw.bakeCurvStrength + "";
			let radius = pass ? ParserMaterial.bake_passthrough_radius : Context.raw.bakeCurvRadius + "";
			let offset = pass ? ParserMaterial.bake_passthrough_offset : Context.raw.bakeCurvOffset + "";
			strength = `float(${strength})`;
			radius = `float(${radius})`;
			offset = `float(${offset})`;
			frag.n = true;
			frag.write('vec3 dx = dFdx(n);');
			frag.write('vec3 dy = dFdy(n);');
			frag.write('float curvature = max(dot(dx, dx), dot(dy, dy));');
			frag.write('curvature = clamp(pow(curvature, (1.0 / ' + radius + ') * 0.25) * ' + strength + ' * 2.0 + ' + offset + ' / 10.0, 0.0, 1.0);');
			if (Context.raw.bakeAxis != BakeAxis.BakeXYZ) {
				let axis = MakeBake.axisString(Context.raw.bakeAxis);
				frag.write(`curvature *= dot(n, ${axis});`);
			}
			frag.write('fragColor[0] = vec4(curvature, curvature, curvature, 1.0);');
		}
		else if (Context.raw.bakeType == BakeType.BakeNormal) { // Tangent
			frag.n = true;
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo'); // Baked high-poly normals
			frag.write('vec3 n0 = textureLod(texpaint_undo, texCoord, 0.0).rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			frag.add_function(ShaderFunctions.str_cotangentFrame);
			frag.write('mat3 invTBN = transpose(cotangentFrame(n, n, texCoord));');
			frag.write('vec3 res = normalize(mul(n0, invTBN)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
			frag.write('fragColor[0] = vec4(res, 1.0);');
		}
		else if (Context.raw.bakeType == BakeType.BakeNormalObject) {
			frag.n = true;
			frag.write('fragColor[0] = vec4(n * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			if (Context.raw.bakeUpAxis == BakeUpAxis.BakeUpY) {
				frag.write('fragColor[0].rgb = vec3(fragColor[0].r, fragColor[0].b, 1.0 - fragColor[0].g);');
			}
		}
		else if (Context.raw.bakeType == BakeType.BakeHeight) {
			frag.wposition = true;
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo'); // Baked high-poly positions
			frag.write('vec3 wpos0 = textureLod(texpaint_undo, texCoord, 0.0).rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			frag.write('float res = distance(wpos0, wposition) * 10.0;');
			frag.write('fragColor[0] = vec4(res, res, res, 1.0);');
		}
		else if (Context.raw.bakeType == BakeType.BakeDerivative) {
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo'); // Baked height
			frag.write('vec2 texDx = dFdx(texCoord);');
			frag.write('vec2 texDy = dFdy(texCoord);');
			frag.write('float h0 = textureLod(texpaint_undo, texCoord, 0.0).r * 100;');
			frag.write('float h1 = textureLod(texpaint_undo, texCoord + texDx, 0.0).r * 100;');
			frag.write('float h2 = textureLod(texpaint_undo, texCoord + texDy, 0.0).r * 100;');
			frag.write('fragColor[0] = vec4((h1 - h0) * 0.5 + 0.5, (h2 - h0) * 0.5 + 0.5, 0.0, 1.0);');
		}
		else if (Context.raw.bakeType == BakeType.BakePosition) {
			frag.wposition = true;
			frag.write('fragColor[0] = vec4(wposition * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			if (Context.raw.bakeUpAxis == BakeUpAxis.BakeUpY) {
				frag.write('fragColor[0].rgb = vec3(fragColor[0].r, fragColor[0].b, 1.0 - fragColor[0].g);');
			}
		}
		else if (Context.raw.bakeType == BakeType.BakeTexCoord) {
			frag.write('fragColor[0] = vec4(texCoord.xy, 0.0, 1.0);');
		}
		else if (Context.raw.bakeType == BakeType.BakeMaterialID) {
			frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			frag.write('float sample_matid = textureLod(texpaint_nor_undo, texCoord, 0.0).a + 1.0 / 255.0;');
			frag.write('float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float matid_g = fract(sin(dot(vec2(sample_matid * 20.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('fragColor[0] = vec4(matid_r, matid_g, matid_b, 1.0);');
		}
		else if (Context.raw.bakeType == BakeType.BakeObjectID) {
			frag.add_uniform('float objectId', '_objectId');
			frag.write('float obid = objectId + 1.0 / 255.0;');
			frag.write('float id_r = fract(sin(dot(vec2(obid, obid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float id_g = fract(sin(dot(vec2(obid * 20.0, obid), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float id_b = fract(sin(dot(vec2(obid, obid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('fragColor[0] = vec4(id_r, id_g, id_b, 1.0);');
		}
		else if (Context.raw.bakeType == BakeType.BakeVertexColor) {
			if (con.allow_vcols) {
				con.add_elem("col", "short4norm");
				frag.write('fragColor[0] = vec4(vcolor.r, vcolor.g, vcolor.b, 1.0);');
			}
			else {
				frag.write('fragColor[0] = vec4(1.0, 1.0, 1.0, 1.0);');
			}
		}
	}

	static positionAndNormal = (vert: NodeShader, frag: NodeShader) => {
		vert.add_out('vec3 position');
		vert.add_out('vec3 normal');
		vert.add_uniform('mat4 W', '_worldMatrix');
		vert.write('position = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;');
		vert.write('normal = vec3(nor.xy, pos.w);');
		vert.write('vec2 tpos = vec2(tex.x * 2.0 - 1.0, (1.0 - tex.y) * 2.0 - 1.0);');
		vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');
		frag.add_out('vec4 fragColor[2]');
		frag.write('fragColor[0] = vec4(position, 1.0);');
		frag.write('fragColor[1] = vec4(normal, 1.0);');
	}

	static setColorWrites = (con_paint: NodeShaderContext) => {
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

	static axisString = (i: i32): string => {
		return i == BakeAxis.BakeX  ? "vec3(1,0,0)"  :
			   i == BakeAxis.BakeY  ? "vec3(0,1,0)"  :
			   i == BakeAxis.BakeZ  ? "vec3(0,0,1)"  :
			   i == BakeAxis.BakeMX ? "vec3(-1,0,0)" :
			   i == BakeAxis.BakeMY ? "vec3(0,-1,0)" :
							 		  "vec3(0,0,-1)";
	}
}
