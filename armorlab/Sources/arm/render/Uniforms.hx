package arm.render;

import iron.data.MaterialData;
import iron.object.Object;
import iron.system.Input;
import iron.math.Vec4;
import iron.math.Mat4;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UISidebar;
import arm.shader.MaterialParser;
import arm.Enums;

class Uniforms {

	static var vec = new Vec4();
	static var orthoP = Mat4.ortho(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);

	public static function init() {
		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];
		iron.object.Uniforms.externalVec3Links = [linkVec3];
		iron.object.Uniforms.externalVec4Links = [linkVec4];
		iron.object.Uniforms.externalMat4Links = [linkMat4];
		iron.object.Uniforms.externalTextureLinks = [linkTex];
	}

	public static function linkFloat(object: Object, mat: MaterialData, link: String): Null<kha.FastFloat> {
		switch (link) {
			case "_brushRadius": {
				var radius = Context.brushRadius;
				var val = radius / 15.0;
				var pen = Input.getPen();
				if (Config.raw.pressure_radius && pen.down()) {
					val *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				val *= 2;
				return val;
			}
			case "_vignetteStrength": {
				return Config.raw.rp_vignette;
			}
		}
		if (MaterialParser.script_links != null) {
			for (key in MaterialParser.script_links.keys()) {
				var script = MaterialParser.script_links[key];
				var result = 0.0;
				if (script != "") {
					try {
						result = js.Lib.eval(script);
					}
					catch(e: Dynamic) {
						Console.log(e);
					}
				}
				return result;
			}
		}
		return null;
	}

	public static function linkVec2(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		switch (link) {
			case "_gbufferSize": {
				vec.set(0, 0, 0);
				var gbuffer2 = RenderPath.active.renderTargets.get("gbuffer2");
				vec.set(gbuffer2.image.width, gbuffer2.image.height, 0);
				return vec;
			}
			case "_cloneDelta": {
				vec.set(Context.cloneDeltaX, Context.cloneDeltaY, 0);
				return vec;
			}
			case "_texpaintSize": {
				vec.set(Config.getTextureResX(), Config.getTextureResY(), 0);
				return vec;
			}
		}
		return null;
	}

	public static function linkVec3(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		var v: Vec4 = null;
		switch (link) {
		}

		return v;
	}

	public static function linkVec4(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		switch (link) {
			case "_inputBrush": {
				var down = Input.getMouse().down() || Input.getPen().down();
				vec.set(Context.paintVec.x, Context.paintVec.y, down ? 1.0 : 0.0, 0.0);
				return vec;
			}
			case "_inputBrushLast": {
				var down = Input.getMouse().down() || Input.getPen().down();
				vec.set(Context.lastPaintVecX, Context.lastPaintVecY, down ? 1.0 : 0.0, 0.0);
				return vec;
			}
			case "_envmapData": {
				vec.set(Context.envmapAngle, Math.sin(-Context.envmapAngle), Math.cos(-Context.envmapAngle), Scene.active.world.probe.raw.strength);
				return vec;
			}
			case "_envmapDataWorld": {
				vec.set(Context.envmapAngle, Math.sin(-Context.envmapAngle), Math.cos(-Context.envmapAngle), Context.showEnvmap ? Scene.active.world.probe.raw.strength : 4.0);
				return vec;
			}
		}
		return null;
	}

	public static function linkMat4(object: Object, mat: MaterialData, link: String): iron.math.Mat4 {
		switch (link) {
		}
		return null;
	}

	public static function linkTex(object: Object, mat: MaterialData, link: String): kha.Image {
		switch (link) {
			case "_texpaint_undo": {
				return null;
			}
			case "_texpaint_nor_undo": {
				return null;
			}
			case "_texpaint_pack_undo": {
				return null;
			}
			#if arm_ltc
			case "_ltcMat": {
				if (arm.data.ConstData.ltcMatTex == null) arm.data.ConstData.initLTC();
				return arm.data.ConstData.ltcMatTex;
			}
			case "_ltcMag": {
				if (arm.data.ConstData.ltcMagTex == null) arm.data.ConstData.initLTC();
				return arm.data.ConstData.ltcMagTex;
			}
			#end
		}

		if (link.startsWith("_texpaint_pack_vert")) {
			var tid = link.substr(link.length - 1);
			return RenderPath.active.renderTargets.get("texpaint_pack" + tid).image;
		}
		if (link.startsWith("_texpaint_vert")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return arm.node.brush.BrushOutputNode.inst.texpaint;
		}
		if (link.startsWith("_texpaint_nor")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return arm.node.brush.BrushOutputNode.inst.texpaint_nor;
		}
		if (link.startsWith("_texpaint_pack")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return arm.node.brush.BrushOutputNode.inst.texpaint_pack;
		}
		if (link.startsWith("_texpaint")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return arm.node.brush.BrushOutputNode.inst.texpaint;
		}

		return null;
	}
}
