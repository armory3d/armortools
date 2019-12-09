package arm.render;

import iron.data.MaterialData;
import iron.object.Object;
import iron.system.Input;
import iron.math.Vec4;
import iron.RenderPath;
import iron.Scene;
#if arm_painter
import arm.ui.UITrait;
import arm.util.UVUtil;
import arm.Tool;
#end
using StringTools;

class Uniforms {

	public static function init() {
		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];
		iron.object.Uniforms.externalVec3Links = [linkVec3];
		iron.object.Uniforms.externalVec4Links = [linkVec4];
		iron.object.Uniforms.externalTextureLinks = [linkTex];
	}

	public static function linkFloat(object: Object, mat: MaterialData, link: String): Null<kha.FastFloat> {
		#if arm_painter
		if (link == "_brushRadius") {
			var val = (UITrait.inst.brushRadius * UITrait.inst.brushNodesRadius) / 15.0;
			var pen = Input.getPen();
			if (UITrait.penPressureRadius && pen.down()) val *= pen.pressure;
			var decal = Context.tool == ToolDecal || Context.tool == ToolText;
			if (UITrait.inst.brush3d && !decal) {
				val *= UITrait.inst.paint2d ? 0.6 : 2;
			}
			else val *= 900 / App.h(); // Projection ratio
			return val;
		}
		if (link == "_brushScaleX") {
			return 1 / UITrait.inst.brushScaleX;
		}
		if (link == "_brushOpacity") {
			var val = UITrait.inst.brushOpacity * UITrait.inst.brushNodesOpacity;
			var pen = Input.getPen();
			if (UITrait.penPressureOpacity && pen.down()) val *= pen.pressure;
			return val;
		}
		if (link == "_brushHardness") {
			if (Context.tool != ToolBrush && Context.tool != ToolEraser) return 1.0;
			var val = UITrait.inst.brushHardness * UITrait.inst.brushNodesHardness;
			var pen = Input.getPen();
			if (UITrait.penPressureHardness && pen.down()) val *= pen.pressure;
			if (UITrait.inst.brush3d && !UITrait.inst.paint2d) val *= val;
			return val;
		}
		if (link == "_brushScale") {
			var nodesScale = UITrait.inst.brushNodesScale;
			var fill = Context.layer.material_mask != null;
			var val = (fill ? Context.layer.uvScale : UITrait.inst.brushScale) * nodesScale;
			return val;
		}
		if (link == "_texpaintSize") {
			return Config.getTextureRes();
		}
		if (link == "_objectId") {
			return Project.paintObjects.indexOf(Context.paintObject);
		}
		#end
		#if arm_world
		if (link == "_voxelgiHalfExtentsUni") {
			#if arm_painter
			return UITrait.inst.vxaoExt;
			#else
			return 10.0;
			#end
		}
		#end
		if (link == "_vignetteStrength") {
			#if arm_painter
			return UITrait.inst.vignetteStrength;
			#else
			return 0.4;
			#end
		}
		if (link == "_coneOffset") {
			#if arm_painter
			return UITrait.inst.vxaoOffset;
			#else
			return 1.5;
			#end
		}
		if (link == "_coneAperture") {
			#if arm_painter
			return UITrait.inst.vxaoAperture;
			#else
			return 1.2;
			#end
		}
		if (link == "_dilateRadius") {
			#if arm_painter
			return UITrait.inst.dilateRadius;
			#else
			return 8.0;
			#end
		}
		return null;
	}

	public static function linkVec2(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		#if arm_painter
		var vec2 = UITrait.inst.vec2;
		if (link == "_sub") {
			UITrait.inst.sub = (UITrait.inst.sub + 1) % 4;
			var eps = UITrait.inst.brushBias * 0.00022 * Config.getTextureResBias();
			UITrait.inst.sub == 0 ? vec2.set(eps, eps, 0.0) :
			UITrait.inst.sub == 1 ? vec2.set(eps, -eps, 0.0) :
			UITrait.inst.sub == 2 ? vec2.set(-eps, -eps, 0.0) :
									vec2.set(-eps, eps, 0.0);
			return vec2;
		}
		if (link == "_texcoloridSize") {
			if (Project.assets.length == 0) return vec2;
			var img = UITrait.inst.getImage(Project.assets[UITrait.inst.colorIdHandle.position]);
			vec2.set(img.width, img.height, 0);
			return vec2;
		}
		if (link == "_gbufferSize") {
			vec2.set(0, 0, 0);
			var gbuffer2 = RenderPath.active.renderTargets.get("gbuffer2");
			vec2.set(gbuffer2.image.width, gbuffer2.image.height, 0);
			return vec2;
		}
		if (link == "_cloneDelta") {
			vec2.set(UITrait.inst.cloneDeltaX, UITrait.inst.cloneDeltaY, 0);
			return vec2;
		}
		#end
		return null;
	}

	public static function linkVec3(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		var v: Vec4 = null;
		#if arm_world
		if (link == "_hosekA") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekA.x;
				v.y = arm.data.HosekWilkie.data.hosekA.y;
				v.z = arm.data.HosekWilkie.data.hosekA.z;
			}
			return v;
		}
		if (link == "_hosekB") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekB.x;
				v.y = arm.data.HosekWilkie.data.hosekB.y;
				v.z = arm.data.HosekWilkie.data.hosekB.z;
			}
			return v;
		}
		if (link == "_hosekC") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekC.x;
				v.y = arm.data.HosekWilkie.data.hosekC.y;
				v.z = arm.data.HosekWilkie.data.hosekC.z;
			}
			return v;
		}
		if (link == "_hosekD") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekD.x;
				v.y = arm.data.HosekWilkie.data.hosekD.y;
				v.z = arm.data.HosekWilkie.data.hosekD.z;
			}
			return v;
		}
		if (link == "_hosekE") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekE.x;
				v.y = arm.data.HosekWilkie.data.hosekE.y;
				v.z = arm.data.HosekWilkie.data.hosekE.z;
			}
			return v;
		}
		if (link == "_hosekF") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekF.x;
				v.y = arm.data.HosekWilkie.data.hosekF.y;
				v.z = arm.data.HosekWilkie.data.hosekF.z;
			}
			return v;
		}
		if (link == "_hosekG") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekG.x;
				v.y = arm.data.HosekWilkie.data.hosekG.y;
				v.z = arm.data.HosekWilkie.data.hosekG.z;
			}
			return v;
		}
		if (link == "_hosekH") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekH.x;
				v.y = arm.data.HosekWilkie.data.hosekH.y;
				v.z = arm.data.HosekWilkie.data.hosekH.z;
			}
			return v;
		}
		if (link == "_hosekI") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekI.x;
				v.y = arm.data.HosekWilkie.data.hosekI.y;
				v.z = arm.data.HosekWilkie.data.hosekI.z;
			}
			return v;
		}
		if (link == "_hosekZ") {
			if (arm.data.HosekWilkie.data == null) {
				arm.data.HosekWilkie.recompute(Scene.active.world);
			}
			if (arm.data.HosekWilkie.data != null) {
				v = iron.object.Uniforms.helpVec;
				v.x = arm.data.HosekWilkie.data.hosekZ.x;
				v.y = arm.data.HosekWilkie.data.hosekZ.y;
				v.z = arm.data.HosekWilkie.data.hosekZ.z;
			}
			return v;
		}
		#end

		return v;
	}

	public static function linkVec4(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		#if arm_painter
		var vec2 = UITrait.inst.vec2;
		if (link == "_inputBrush") {
			var down = Input.getMouse().down() || Input.getPen().down();
			vec2.set(UITrait.inst.paintVec.x, UITrait.inst.paintVec.y, down ? 1.0 : 0.0, 0.0);
			if (UITrait.inst.paint2d) vec2.x -= 1.0;
			return vec2;
		}
		if (link == "_inputBrushLast") {
			var down = Input.getMouse().down() || Input.getPen().down();
			vec2.set(UITrait.inst.lastPaintVecX, UITrait.inst.lastPaintVecY, down ? 1.0 : 0.0, 0.0);
			if (UITrait.inst.paint2d) vec2.x -= 1.0;
			return vec2;
		}
		#end
		return null;
	}

	public static function linkTex(object: Object, mat: MaterialData, link: String): kha.Image {
		#if arm_painter
		if (link == "_texcolorid") {
			if (Project.assets.length == 0) return RenderPath.active.renderTargets.get("empty_white").image;
			else return UITrait.inst.getImage(Project.assets[UITrait.inst.colorIdHandle.position]);
		}
		if (link == "_texuvmap") {
			UVUtil.cacheUVMap(); // TODO: Check overlapping g4 calls here
			return UVUtil.uvmap;
		}
		if (link == "_textrianglemap") {
			UVUtil.cacheTriangleMap(); // TODO: Check overlapping g4 calls here
			return UVUtil.trianglemap;
		}
		if (link == "_textexttool") { // Opacity map for text
			return UITrait.inst.textToolImage;
		}
		if (link == "_texdecalmask") { // Opacity map for decal
			return UITrait.inst.decalMaskImage;
		}
		if (link == "_texbrushmask") {
			return UITrait.inst.brushMaskImage;
		}
		if (link == "_texpaint_undo") {
			var i = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
			return RenderPath.active.renderTargets.get("texpaint_undo" + i).image;
		}
		if (link == "_texpaint_nor_undo") {
			var i = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
			return RenderPath.active.renderTargets.get("texpaint_nor_undo" + i).image;
		}
		if (link == "_texpaint_pack_undo") {
			var i = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
			return RenderPath.active.renderTargets.get("texpaint_pack_undo" + i).image;
		}
		if (link.startsWith("_texpaint_pack_vert")) {
			var tid = link.substr(link.length - 1);
			return RenderPath.active.renderTargets.get("texpaint_pack" + tid).image;
		}

		if (link == "_texpaint_mask") {
			return Context.layer.texpaint_mask;
		}
		if (link.startsWith("_texpaint_mask")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			trace(tid);
			return Project.layers[tid].texpaint_mask;
		}
		if (link.startsWith("_texpaint_nor")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return Project.layers[tid].texpaint_nor;
		}
		if (link.startsWith("_texpaint_pack")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return Project.layers[tid].texpaint_pack;
		}
		if (link.startsWith("_texpaint")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return Project.layers[tid].texpaint;
		}

		if (link == "_texparticle") {
			return RenderPath.active.renderTargets.get("texparticle").image;
		}
		#end
		#if arm_ltc
		if (link == "_ltcMat") {
			if (arm.data.ConstData.ltcMatTex == null) arm.data.ConstData.initLTC();
			return arm.data.ConstData.ltcMatTex;
		}
		if (link == "_ltcMag") {
			if (arm.data.ConstData.ltcMagTex == null) arm.data.ConstData.initLTC();
			return arm.data.ConstData.ltcMagTex;
		}
		#end
		return null;
	}
}
