package arm;

import iron.data.MaterialData;
import iron.object.Object;
import arm.ui.*;

class Uniforms {
	public static function init() {
		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];
		iron.object.Uniforms.externalVec4Links = [linkVec4];
		iron.object.Uniforms.externalTextureLinks.push(linkTex);
	}

	public static function linkFloat(object:Object, mat:MaterialData, link:String):Null<kha.FastFloat> {
		if (link == '_brushRadius') {
			var r = (UITrait.inst.brushRadius * UITrait.inst.brushNodesRadius) / 15.0;
			var pen = iron.system.Input.getPen();
			if (UITrait.penPressure && pen.down()) r *= pen.pressure;
			return r;
		}
		else if (link == '_brushOpacity') {
			return UITrait.inst.brushOpacity * UITrait.inst.brushOpacity *
				   UITrait.inst.brushNodesOpacity * UITrait.inst.brushNodesOpacity;
		}
		else if (link == '_brushScale') {
			return (UITrait.inst.brushScale * UITrait.inst.brushNodesScale) * 2.0;
		}
		else if (link == '_brushHardness') {
			var f = UITrait.inst.brushHardness * UITrait.inst.brushNodesHardness;
			return f * f * 100;
		}
		else if (link == '_paintDepthBias') {
			return UITrait.inst.paintVisible ? 0.0001 : 1.0;
		}
		return null;
	}

	public static function linkVec2(object:Object, mat:MaterialData, link:String):iron.math.Vec4 {
		var vec2 = UITrait.inst.vec2;
		if (link == '_sub') {
			var seps = UITrait.inst.brushBias * 0.0004 * Config.getTextureResBias();
			UITrait.inst.sub = (UITrait.inst.sub + 1) % 9;
			var sub = UITrait.inst.sub;
			if (sub == 0) vec2.set(0.0 + seps, 0.0, 0.0);
			else if (sub == 1) vec2.set(0.0 - seps, 0.0, 0.0);
			else if (sub == 2) vec2.set(0.0, 0.0 + seps, 0.0);
			else if (sub == 3) vec2.set(0.0, 0.0 - seps, 0.0);
			else if (sub == 4) vec2.set(0.0 + seps, 0.0 + seps, 0.0);
			else if (sub == 5) vec2.set(0.0 - seps, 0.0 - seps, 0.0);
			else if (sub == 6) vec2.set(0.0 + seps, 0.0 - seps, 0.0);
			else if (sub == 7) vec2.set(0.0 - seps, 0.0 + seps, 0.0);
			else if (sub == 8) vec2.set(0.0, 0.0, 0.0);
			return vec2;
		}
		else if (link == '_texcoloridSize') {
			if (UITrait.inst.assets.length == 0) return vec2;
			var img = UITrait.inst.getImage(UITrait.inst.assets[UITrait.inst.colorIdHandle.position]);
			vec2.set(img.width, img.height, 0);
			return vec2;
		}
		else if (link == '_textrianglemapSize') {
			var res = Config.getTextureRes();
			vec2.set(res, res, 0);
			return vec2;
		}
		else if (link == '_gbufferSize') {
			vec2.set(0, 0, 0);
			var gbuffer2 = iron.RenderPath.active.renderTargets.get("gbuffer2");
			vec2.set(gbuffer2.image.width, gbuffer2.image.height, 0);
			return vec2;
			// Check equality at init
			// trace(iron.RenderPath.active.renderTargets.get("gbuffer2").image.width);
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
			return vec2;
		}
		else if (link == '_inputBrushLast') {
			var down = iron.system.Input.getMouse().down() || iron.system.Input.getPen().down();
			vec2.set(UITrait.inst.lastPaintVecX, UITrait.inst.lastPaintVecY, down ? 1.0 : 0.0, 0.0);
			return vec2;
		}
		return null;
	}

	public static function linkTex(object:Object, mat:MaterialData, link:String):kha.Image {
		if (link == "_texcolorid") {
			if (UITrait.inst.assets.length == 0) return UITrait.inst.bundled.get("empty.jpg");
			else return UITrait.inst.getImage(UITrait.inst.assets[UITrait.inst.colorIdHandle.position]);
		}
		else if (link == "_texuvmap") {
			UIView2D.inst.cacheUVMap(); // TODO: Check overlapping g4 calls here
			return UIView2D.inst.uvmap;
		}
		else if (link == "_textrianglemap") {
			UIView2D.inst.cacheTriangleMap(); // TODO: Check overlapping g4 calls here
			return UIView2D.inst.trianglemap;
		}
		else if (link == "_textexttool") { // Opacity map for text
			return UITrait.inst.textToolImage;
		}
		else if (link == "_texdecalmask") { // Opacity map for decal
			return UITrait.inst.decalMaskImage;
		}
		else if (link == "_texpaint_undo") {
			var i = UITrait.inst.undoI - 1 < 0 ? UITrait.inst.C.undo_steps - 1 : UITrait.inst.undoI - 1;
			return iron.RenderPath.active.renderTargets.get("texpaint_undo" + i).image;
		}
		else if (link == "_texpaint_nor_undo") {
			var i = UITrait.inst.undoI - 1 < 0 ? UITrait.inst.C.undo_steps - 1 : UITrait.inst.undoI - 1;
			return iron.RenderPath.active.renderTargets.get("texpaint_nor_undo" + i).image;
		}
		else if (link == "_texpaint_pack_undo") {
			var i = UITrait.inst.undoI - 1 < 0 ? UITrait.inst.C.undo_steps - 1 : UITrait.inst.undoI - 1;
			return iron.RenderPath.active.renderTargets.get("texpaint_pack_undo" + i).image;
		}
		return null;
	}
}
