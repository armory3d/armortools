package arm.node;

import iron.data.SceneFormat;
import arm.ui.UITrait;
import arm.node.MaterialShader;
import arm.Tool;

class MaterialBuilder {

	public static var heightUsed = false;
	public static var emisUsed = false;
	public static var subsUsed = false;

	public static inline function make_paint(data: MaterialShaderData, matcon: TMaterialContext): MaterialShaderContext {
		return MakePaint.run(data, matcon);
	}

	public static inline function make_mesh(data: MaterialShaderData): MaterialShaderContext {
		return MakeMesh.run(data);
	}

	public static inline function make_mesh_preview(data: MaterialShaderData, matcon: TMaterialContext): MaterialShaderContext {
		return MakeMeshPreview.run(data, matcon);
	}

	public static inline function make_voxel(data: iron.data.ShaderData.ShaderContext) {
		#if rp_voxelao
		MakeVoxel.run(data);
		#end
	}

	public static inline function make_particle(data: MaterialShaderData): MaterialShaderContext {
		return MakeParticle.run(data);
	}

	public static function blendMode(frag: MaterialShader, blending: Int, cola: String, colb: String, opac: String): String {
		if (blending == BlendMix) {
			return 'mix($cola, $colb, $opac)';
		}
		else if (blending == BlendDarken) {
			return 'mix($cola, min($cola, $colb), $opac)';
		}
		else if (blending == BlendMultiply) {
			return 'mix($cola, $cola * $colb, $opac)';
		}
		else if (blending == BlendBurn) {
			return 'mix($cola, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $cola) / $colb, $opac)';
		}
		else if (blending == BlendLighten) {
			return 'max($cola, $colb * $opac)';
		}
		else if (blending == BlendScreen) {
			return '(vec3(1.0, 1.0, 1.0) - (vec3(1.0 - $opac, 1.0 - $opac, 1.0 - $opac) + $opac * (vec3(1.0, 1.0, 1.0) - $colb)) * (vec3(1.0, 1.0, 1.0) - $cola))';
		}
		else if (blending == BlendDodge) {
			return 'mix($cola, $cola / (vec3(1.0, 1.0, 1.0) - $colb), $opac)';
		}
		else if (blending == BlendAdd) {
			return 'mix($cola, $cola + $colb, $opac)';
		}
		else if (blending == BlendOverlay) {
			#if (kha_direct3d11 || kha_direct3d12)
			return 'mix($cola, ($cola < vec3(0.5, 0.5, 0.5) ? vec3(2.0, 2.0, 2.0) * $cola * $colb : vec3(1.0, 1.0, 1.0) - vec3(2.0, 2.0, 2.0) * (vec3(1.0, 1.0, 1.0) - $colb) * (vec3(1.0, 1.0, 1.0) - $cola)), $opac)';
			#else
			return 'mix($cola, $colb, $opac)'; // TODO
			#end
		}
		else if (blending == BlendSoftLight) {
			return '((1.0 - $opac) * $cola + $opac * ((vec3(1.0, 1.0, 1.0) - $cola) * $colb * $cola + $cola * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $colb) * (vec3(1.0, 1.0, 1.0) - $cola))))';
		}
		else if (blending == BlendLinearLight) {
			return '($cola + $opac * (vec3(2.0, 2.0, 2.0) * ($colb - vec3(0.5, 0.5, 0.5))))';
		}
		else if (blending == BlendDifference) {
			return 'mix($cola, abs($cola - $colb), $opac)';
		}
		else if (blending == BlendSubtract) {
			return 'mix($cola, $cola - $colb, $opac)';
		}
		else if (blending == BlendDivide) {
			return 'vec3(1.0 - $opac, 1.0 - $opac, 1.0 - $opac) * $cola + vec3($opac, $opac, $opac) * $cola / $colb';
		}
		else if (blending == BlendHue) {
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($colb).r, rgb_to_hsv($cola).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else if (blending == BlendSaturation) {
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($cola).r, rgb_to_hsv($colb).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else if (blending == BlendColor) {
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($colb).r, rgb_to_hsv($colb).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else { // BlendValue
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
