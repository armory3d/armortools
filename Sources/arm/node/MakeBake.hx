package arm.node;

import iron.object.MeshObject;
import iron.data.SceneFormat;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.node.MaterialShader;
import arm.Tool;

class MakeBake {

	public static function run(vert:MaterialShader, frag:MaterialShader) {
		if (UITrait.inst.bakeType == 0) { // AO
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
			if (UITrait.inst.bakeAxis > 0) {
				var axis = axisString(UITrait.inst.bakeAxis);
				frag.write('ao *= dot(n, $axis);');
			}
			frag.write('ao = 1.0 - ao;');
			frag.write('fragColor[0] = vec4(ao, ao, ao, 1.0);');
		}
		else if (UITrait.inst.bakeType == 1) { // Curvature
			var strength = UITrait.inst.bakeCurvStrength * 2.0;
			var radius = (1.0 / UITrait.inst.bakeCurvRadius) * 0.25;
			var offset = UITrait.inst.bakeCurvOffset / 10;
			frag.n = true;
			frag.write('vec3 dx = dFdx(n);');
			frag.write('vec3 dy = dFdy(n);');
			frag.write('float curvature = max(dot(dx, dx), dot(dy, dy));');
			frag.write('curvature = clamp(pow(curvature, $radius) * $strength + $offset, 0.0, 1.0);');
			if (UITrait.inst.bakeAxis > 0) {
				var axis = axisString(UITrait.inst.bakeAxis);
				frag.write('curvature *= dot(n, $axis);');
			}
			frag.write('fragColor[0] = vec4(curvature, curvature, curvature, 1.0);');
		}
		else if (UITrait.inst.bakeType == 2) { // Normal (Tangent)
			frag.n = true;
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
			frag.write('vec3 n0 = textureLod(texpaint_undo, texCoord, 0.0).rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			frag.add_function(MaterialFunctions.str_cotangentFrame);
			frag.write('mat3 invTBN = transpose(cotangentFrame(n, n, texCoord));');
			frag.write('vec3 res = normalize(mul(n0, invTBN)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
			frag.write('fragColor[0] = vec4(res, 1.0);');
		}
		else if (UITrait.inst.bakeType == 3) { // Normal (World)
			frag.n = true;
			frag.write('fragColor[0] = vec4(n * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			if (UITrait.inst.bakeUpAxis == 1) { // Y up
				frag.write('fragColor[0].rgb = vec3(fragColor[0].r, fragColor[0].b, 1.0 - fragColor[0].g);');
			}
		}
		else if (UITrait.inst.bakeType == 4) { // Position
			frag.wposition = true;
			frag.write('fragColor[0] = vec4(wposition * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			if (UITrait.inst.bakeUpAxis == 1) { // Y up
				frag.write('fragColor[0].rgb = vec3(fragColor[0].r, fragColor[0].b, 1.0 - fragColor[0].g);');
			}
		}
		else if (UITrait.inst.bakeType == 5) { // TexCoord
			frag.write('fragColor[0] = vec4(texCoord.xy, 0.0, 1.0);');
		}
		else if (UITrait.inst.bakeType == 6) { // Material ID
			frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			frag.write('float sample_matid = textureLod(texpaint_nor_undo, texCoord, 0.0).a;');
			frag.write('float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float matid_g = fract(sin(dot(vec2(sample_matid * 20.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('fragColor[0] = vec4(matid_r, matid_g, matid_b, 1.0);');
		}
		else if (UITrait.inst.bakeType == 7) { // Object ID
			frag.add_uniform('float objectId', '_objectId');
			frag.write('float id_r = fract(sin(dot(vec2(objectId, objectId * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float id_g = fract(sin(dot(vec2(objectId * 20.0, objectId), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('float id_b = fract(sin(dot(vec2(objectId, objectId * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
			frag.write('fragColor[0] = vec4(id_r, id_g, id_b, 1.0);');
		}
	}

	public static function positionAndNormal(vert:MaterialShader, frag:MaterialShader) {
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

	public static function setColorWrites(con_paint:MaterialShaderContext) {
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
		return i == 1 ? "vec3(1,0,0)" :
			   i == 2 ? "vec3(0,1,0)" :
			   i == 3 ? "vec3(0,0,1)" :
			   i == 4 ? "vec3(-1,0,0)" :
			   i == 5 ? "vec3(0,-1,0)" :
						"vec3(0,0,-1)";
	}
}
