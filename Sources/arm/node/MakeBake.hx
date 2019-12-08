package arm.node;

import arm.ui.UITrait;
import arm.node.MaterialShader;
import arm.Tool;

class MakeBake {

	public static function run(vert: MaterialShader, frag: MaterialShader) {
		if (UITrait.inst.bakeType == BakeAO) { // Voxel
			// Apply normal channel
			frag.wposition = true;
			frag.n = true;
			frag.vVec = true;
			frag.add_function(MaterialFunctions.str_cotangentFrame);
			#if (kha_direct3d11 || kha_direct3d12)
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end
			frag.write('n = nortan * 2.0 - 1.0;');
			frag.write('n.y = -n.y;');
			frag.write('n = normalize(mul(n, TBN));');

			frag.write(MaterialBuilder.voxelgiHalfExtents());
			frag.write('vec3 voxpos = wposition / voxelgiHalfExtents;');
			frag.add_uniform('sampler3D voxels');
			frag.add_function(MaterialFunctions.str_traceAO);
			frag.n = true;
			var strength = UITrait.inst.bakeAoStrength;
			var radius = UITrait.inst.bakeAoRadius;
			var offset = UITrait.inst.bakeAoOffset;
			frag.write('float ao = traceAO(voxpos, n, $radius, $offset) * $strength;');
			if (UITrait.inst.bakeAxis != BakeXYZ) {
				var axis = axisString(UITrait.inst.bakeAxis);
				frag.write('ao *= dot(n, $axis);');
			}
			frag.write('ao = 1.0 - ao;');
			frag.write('fragColor[0] = vec4(ao, ao, ao, 1.0);');
		}
		else if (UITrait.inst.bakeType == BakeCurvature) {
			var strength = UITrait.inst.bakeCurvStrength * 2.0;
			var radius = (1.0 / UITrait.inst.bakeCurvRadius) * 0.25;
			var offset = UITrait.inst.bakeCurvOffset / 10;
			frag.n = true;
			frag.write('vec3 dx = dFdx(n);');
			frag.write('vec3 dy = dFdy(n);');
			frag.write('float curvature = max(dot(dx, dx), dot(dy, dy));');
			frag.write('curvature = clamp(pow(curvature, $radius) * $strength + $offset, 0.0, 1.0);');
			if (UITrait.inst.bakeAxis != BakeXYZ) {
				var axis = axisString(UITrait.inst.bakeAxis);
				frag.write('curvature *= dot(n, $axis);');
			}
			frag.write('fragColor[0] = vec4(curvature, curvature, curvature, 1.0);');
		}
		else if (UITrait.inst.bakeType == BakeNormal) { // Tangent
			frag.n = true;
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo'); // Baked high-poly normals
			frag.write('vec3 n0 = textureLod(texpaint_undo, texCoord, 0.0).rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			frag.add_function(MaterialFunctions.str_cotangentFrame);
			frag.write('mat3 invTBN = transpose(cotangentFrame(n, n, texCoord));');
			frag.write('vec3 res = normalize(mul(n0, invTBN)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
			frag.write('fragColor[0] = vec4(res, 1.0);');
		}
		else if (UITrait.inst.bakeType == BakeNormalObject) {
			frag.n = true;
			frag.write('fragColor[0] = vec4(n * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			if (UITrait.inst.bakeUpAxis == BakeUpY) {
				frag.write('fragColor[0].rgb = vec3(fragColor[0].r, fragColor[0].b, 1.0 - fragColor[0].g);');
			}
		}
		else if (UITrait.inst.bakeType == BakeHeight) {
			frag.wposition = true;
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo'); // Baked high-poly positions
			frag.write('vec3 wpos0 = textureLod(texpaint_undo, texCoord, 0.0).rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			frag.write('float res = distance(wpos0, wposition) * 10.0;');
			frag.write('fragColor[0] = vec4(res, res, res, 1.0);');
		}
		else if (UITrait.inst.bakeType == BakeDerivative) {
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo'); // Baked height
			frag.write('vec2 texDx = dFdx(texCoord);');
			frag.write('vec2 texDy = dFdy(texCoord);');
			frag.write('float h0 = textureLod(texpaint_undo, texCoord, 0.0).r * 100;');
			frag.write('float h1 = textureLod(texpaint_undo, texCoord + texDx, 0.0).r * 100;');
			frag.write('float h2 = textureLod(texpaint_undo, texCoord + texDy, 0.0).r * 100;');
			frag.write('fragColor[0] = vec4((h1 - h0) * 0.5 + 0.5, (h2 - h0) * 0.5 + 0.5, 0.0, 1.0);');
		}
		else if (UITrait.inst.bakeType == BakePosition) {
			frag.wposition = true;
			frag.write('fragColor[0] = vec4(wposition * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			if (UITrait.inst.bakeUpAxis == BakeUpY) {
				frag.write('fragColor[0].rgb = vec3(fragColor[0].r, fragColor[0].b, 1.0 - fragColor[0].g);');
			}
		}
		else if (UITrait.inst.bakeType == BakeTexCoord) {
			frag.write('fragColor[0] = vec4(texCoord.xy, 0.0, 1.0);');
		}
		else if (UITrait.inst.bakeType == BakeMaterialID) {
			frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			frag.write('float sample_matid = textureLod(texpaint_nor_undo, texCoord, 0.0).a + 1.0 / 255.0;');
			frag.write('float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float matid_g = fract(sin(dot(vec2(sample_matid * 20.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('fragColor[0] = vec4(matid_r, matid_g, matid_b, 1.0);');
		}
		else if (UITrait.inst.bakeType == BakeObjectID) {
			frag.add_uniform('float objectId', '_objectId');
			frag.write('float obid = objectId + 1.0 / 255.0;');
			frag.write('float id_r = fract(sin(dot(vec2(obid, obid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float id_g = fract(sin(dot(vec2(obid * 20.0, obid), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float id_b = fract(sin(dot(vec2(obid, obid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('fragColor[0] = vec4(id_r, id_g, id_b, 1.0);');
		}
	}

	public static function positionAndNormal(vert: MaterialShader, frag: MaterialShader) {
		vert.add_out('vec3 position');
		vert.add_out('vec3 normal');
		vert.add_uniform('mat4 W', '_worldMatrix');
		vert.write('position = mul(vec4(pos.xyz, 1.0), W);');
		vert.write('normal = vec3(nor.xy, pos.w);');
		vert.add_uniform('vec2 sub', '_sub');
		vert.write('vec2 subtex = tex + sub;');
		vert.write('vec2 tpos = vec2(subtex.x * 2.0 - 1.0, (1.0 - subtex.y) * 2.0 - 1.0);');
		vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');
		frag.add_out('vec4 fragColor[2]');
		frag.write('fragColor[0] = vec4(position, 1.0);');
		frag.write('fragColor[1] = vec4(normal, 1.0);');
	}

	public static function setColorWrites(con_paint: MaterialShaderContext) {
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

	static function axisString(i:Int):String {
		return i == BakeX  ? "vec3(1,0,0)"  :
			   i == BakeY  ? "vec3(0,1,0)"  :
			   i == BakeZ  ? "vec3(0,0,1)"  :
			   i == BakeMX ? "vec3(-1,0,0)" :
			   i == BakeMY ? "vec3(0,-1,0)" :
							 "vec3(0,0,-1)";
	}
}
