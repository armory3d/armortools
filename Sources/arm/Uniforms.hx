package arm;

import iron.data.MaterialData;
import iron.object.Object;
import iron.RenderPath;
import arm.ui.UITrait;
import arm.util.UVUtil;
import arm.Tool;

class Uniforms {
	public static function init() {
		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];
		iron.object.Uniforms.externalVec4Links = [linkVec4];
		iron.object.Uniforms.externalTextureLinks.push(linkTex);
	}

	public static function linkFloat(object:Object, mat:MaterialData, link:String):Null<kha.FastFloat> {
		if (link == '_brushRadius') {
			var val = (UITrait.inst.brushRadius * UITrait.inst.brushNodesRadius) / 15.0;
			var pen = iron.system.Input.getPen();
			if (UITrait.penPressureRadius && pen.down()) val *= pen.pressure;
			var decal = UITrait.inst.selectedTool == ToolDecal || UITrait.inst.selectedTool == ToolText;
			if (UITrait.inst.brush3d && !decal) {
				val *= UITrait.inst.paint2d ? 0.6 : 2;
			}
			else val *= 900 / App.h(); // Projection ratio
			return val;
		}
		else if (link == '_brushOpacity') {
			var val = UITrait.inst.brushOpacity * UITrait.inst.brushNodesOpacity;
			var pen = iron.system.Input.getPen();
			if (UITrait.penPressureOpacity && pen.down()) val *= pen.pressure;
			return val;
		}
		else if (link == '_brushHardness') {
			var val = UITrait.inst.brushHardness * UITrait.inst.brushNodesHardness;
			var pen = iron.system.Input.getPen();
			if (UITrait.penPressureHardness && pen.down()) val *= pen.pressure;
			if (UITrait.inst.brush3d && !UITrait.inst.paint2d) val *= val;
			return val;
		}
		else if (link == '_brushScale') {
			var val = UITrait.inst.brushScale * UITrait.inst.brushNodesScale;
			return val;
		}
		else if (link == '_texpaintSize') {
			return Config.getTextureRes();
		}
		return null;
	}

	public static function linkVec2(object:Object, mat:MaterialData, link:String):iron.math.Vec4 {
		var vec2 = UITrait.inst.vec2;
		if (link == '_sub') {
			UITrait.inst.sub = (UITrait.inst.sub + 1) % 4;
			var eps = UITrait.inst.brushBias * 0.00022 * Config.getTextureResBias();
			UITrait.inst.sub == 0 ? vec2.set(eps, eps, 0.0) :
			UITrait.inst.sub == 1 ? vec2.set(eps, -eps, 0.0) :
			UITrait.inst.sub == 2 ? vec2.set(-eps, -eps, 0.0) :
									vec2.set(-eps, eps, 0.0);
			return vec2;
		}
		else if (link == '_texcoloridSize') {
			if (UITrait.inst.assets.length == 0) return vec2;
			var img = UITrait.inst.getImage(UITrait.inst.assets[UITrait.inst.colorIdHandle.position]);
			vec2.set(img.width, img.height, 0);
			return vec2;
		}
		else if (link == '_gbufferSize') {
			vec2.set(0, 0, 0);
			var gbuffer2 = RenderPath.active.renderTargets.get("gbuffer2");
			vec2.set(gbuffer2.image.width, gbuffer2.image.height, 0);
			return vec2;
			// Check equality at init
			// trace(RenderPath.active.renderTargets.get("gbuffer2").image.width);
			// trace(iron.App.w());
			// var f = armory.renderpath.Inc.getSuperSampling();
		}
		else if (link == '_cloneDelta') {
			vec2.set(UITrait.inst.cloneDeltaX, UITrait.inst.cloneDeltaY, 0);
			return vec2;
		}
		return null;
	}

	public static function linkVec4(object:Object, mat:MaterialData, link:String):iron.math.Vec4 {
		var vec2 = UITrait.inst.vec2;
		if (link == '_inputBrush') {
			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
			vec2.set(UITrait.inst.paintVec.x, UITrait.inst.paintVec.y, down ? 1.0 : 0.0, 0.0);
			if (UITrait.inst.paint2d) vec2.x -= 1.0;
			return vec2;
		}
		else if (link == '_inputBrushLast') {
			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
			vec2.set(UITrait.inst.lastPaintVecX, UITrait.inst.lastPaintVecY, down ? 1.0 : 0.0, 0.0);
			if (UITrait.inst.paint2d) vec2.x -= 1.0;
			return vec2;
		}
		return null;
	}

	public static function linkTex(object:Object, mat:MaterialData, link:String):kha.Image {
		if (link == "_texcolorid") {
			if (UITrait.inst.assets.length == 0) return null;
			else return UITrait.inst.getImage(UITrait.inst.assets[UITrait.inst.colorIdHandle.position]);
		}
		else if (link == "_texuvmap") {
			UVUtil.cacheUVMap(); // TODO: Check overlapping g4 calls here
			return UVUtil.uvmap;
		}
		else if (link == "_textrianglemap") {
			UVUtil.cacheTriangleMap(); // TODO: Check overlapping g4 calls here
			return UVUtil.trianglemap;
		}
		else if (link == "_textexttool") { // Opacity map for text
			return UITrait.inst.textToolImage;
		}
		else if (link == "_texdecalmask") { // Opacity map for decal
			return UITrait.inst.decalMaskImage;
		}
		else if (link == "_texpaint_undo") {
			var i = UITrait.inst.undoI - 1 < 0 ? App.C.undo_steps - 1 : UITrait.inst.undoI - 1;
			return RenderPath.active.renderTargets.get("texpaint_undo" + i).image;
		}
		else if (link == "_texpaint_nor_undo") {
			var i = UITrait.inst.undoI - 1 < 0 ? App.C.undo_steps - 1 : UITrait.inst.undoI - 1;
			return RenderPath.active.renderTargets.get("texpaint_nor_undo" + i).image;
		}
		else if (link == "_texpaint_pack_undo") {
			var i = UITrait.inst.undoI - 1 < 0 ? App.C.undo_steps - 1 : UITrait.inst.undoI - 1;
			return RenderPath.active.renderTargets.get("texpaint_pack_undo" + i).image;
		}
		else if (StringTools.startsWith(link, "_texpaint_pack_vert")) {
			var tid = link.substr(19);
			return RenderPath.active.renderTargets.get("texpaint_pack" + tid).image;
		}
		else if (link == "_texpaint_mask") {
			return UITrait.inst.selectedLayer.texpaint_mask;
		}
		else if (link == "_texparticle") {
			return RenderPath.active.renderTargets.get("texparticle").image;
		}
		return null;
	}
}
