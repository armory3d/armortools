package arm.node;

import iron.object.MeshObject;
import iron.data.SceneFormat;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.node.MaterialShader;
import arm.Tool;

class MaterialBuilder {

	public static var heightUsed = false;
	public static var emisUsed = false;
	public static var subsUsed = false;

	public static inline function make_paint(data:MaterialShaderData, matcon:TMaterialContext):MaterialShaderContext {
		return MakePaint.run(data, matcon);
	}

	public static inline function make_mesh(data:MaterialShaderData):MaterialShaderContext {
		return MakeMesh.run(data);
	}

	public static inline function make_mesh_preview(data:MaterialShaderData, matcon:TMaterialContext):MaterialShaderContext {
		return MakeMeshPreview.run(data, matcon);
	}

	public static inline function make_voxel(data:iron.data.ShaderData.ShaderContext) {
		MakeVoxel.run(data);
	}

	public static inline function make_particle(data:MaterialShaderData):MaterialShaderContext {
		return MakeParticle.run(data);
	}

	public static function blendMode(frag:MaterialShader, blending:Int, cola:String, colb:String, opac:String):String {
		if (blending == 0) { // Mix
			return 'mix($cola, $colb, $opac)';
		}
		else if (blending == 1) { // Darken
			return 'mix($cola, min($cola, $colb), $opac)';
		}
		else if (blending == 2) { // Multiply
			return 'mix($cola, $cola * $colb, $opac)';
		}
		else if (blending == 3) { // Burn
			return 'mix($cola, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $cola) / $colb, $opac)';
		}
		else if (blending == 4) { // Lighten
			return 'max($cola, $colb * $opac)';
		}
		else if (blending == 5) { // Screen
			return '(vec3(1.0, 1.0, 1.0) - (vec3(1.0 - $opac, 1.0 - $opac, 1.0 - $opac) + $opac * (vec3(1.0, 1.0, 1.0) - $colb)) * (vec3(1.0, 1.0, 1.0) - $cola))';
		}
		else if (blending == 6) { // Dodge
			return 'mix($cola, $cola / (vec3(1.0, 1.0, 1.0) - $colb), $opac)';
		}
		else if (blending == 7) { // Add
			return 'mix($cola, $cola + $colb, $opac)';
		}
		else if (blending == 8) { // Overlay
			#if (kha_direct3d11 || kha_direct3d12)
			return 'mix($cola, ($cola < vec3(0.5, 0.5, 0.5) ? vec3(2.0, 2.0, 2.0) * $cola * $colb : vec3(1.0, 1.0, 1.0) - vec3(2.0, 2.0, 2.0) * (vec3(1.0, 1.0, 1.0) - $colb) * (vec3(1.0, 1.0, 1.0) - $cola)), $opac)';
			#else
			return 'mix($cola, $colb, $opac)'; // TODO
			#end
		}
		else if (blending == 9) { // Soft Light
			return '((1.0 - $opac) * $cola + $opac * ((vec3(1.0, 1.0, 1.0) - $cola) * $colb * $cola + $cola * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $colb) * (vec3(1.0, 1.0, 1.0) - $cola))))';
		}
		else if (blending == 10) { // Linear Light
			return '($cola + $opac * (vec3(2.0, 2.0, 2.0) * ($colb - vec3(0.5, 0.5, 0.5))))';
		}
		else if (blending == 11) { // Difference
			return 'mix($cola, abs($cola - $colb), $opac)';
		}
		else if (blending == 12) { // Subtract
			return 'mix($cola, $cola - $colb, $opac)';
		}
		else if (blending == 13) { // Divide
			return 'vec3(1.0 - $opac, 1.0 - $opac, 1.0 - $opac) * $cola + vec3($opac, $opac, $opac) * $cola / $colb';
		}
		else if (blending == 14) { // Hue
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($colb).r, rgb_to_hsv($cola).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else if (blending == 15) { // Saturation
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($cola).r, rgb_to_hsv($colb).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else if (blending == 16) { // Color
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($colb).r, rgb_to_hsv($colb).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else { // Value
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($cola).r, rgb_to_hsv($cola).g, rgb_to_hsv($colb).b)), $opac)';
		}
	}

	public static inline function getDisplaceStrength():Float {
		var sc = Context.mainObject().transform.scale.x;
		return UITrait.inst.displaceStrength * 0.02 * sc;
	}

	public static inline function voxelgiHalfExtents():String {
		var ext = UITrait.inst.vxaoExt;
		return 'const vec3 voxelgiHalfExtents = vec3($ext, $ext, $ext);';
	}
}
