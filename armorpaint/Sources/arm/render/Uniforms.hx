package arm.render;

import iron.data.MaterialData;
import iron.object.Object;
import iron.system.Input;
import iron.math.Vec4;
import iron.math.Mat4;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UISidebar;
import arm.ui.UIView2D;
import arm.util.UVUtil;
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
				var decal = Context.tool == ToolDecal || Context.tool == ToolText;
				var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
				var brushDecalMaskRadius = Context.brushDecalMaskRadius;
				if (Config.raw.brush_3d) {
					brushDecalMaskRadius *= Context.paint2d ? 0.55 * UIView2D.inst.panScale : 2.0;
				}
				var radius = decalMask ? brushDecalMaskRadius : Context.brushRadius;
				var val = (radius * Context.brushNodesRadius) / 15.0;
				var pen = Input.getPen();
				if (Config.raw.pressure_radius && pen.down()) {
					val *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				var scale2d = (900 / App.h()) * Config.raw.window_scale;

				if (Config.raw.brush_3d && !decal) {
					val *= Context.paint2d ? 0.55 * scale2d * UIView2D.inst.panScale : 2;
				}
				else {
					val *= scale2d; // Projection ratio
				}
				return val;
			}
			case "_brushScaleX": {
				return 1 / Context.brushScaleX;
			}
			case "_brushOpacity": {
				var val = Context.brushOpacity * Context.brushNodesOpacity;
				var pen = Input.getPen();
				if (Config.raw.pressure_opacity && pen.down()) {
					val *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				return val;
			}
			case "_brushHardness": {
				var decal = Context.tool == ToolDecal || Context.tool == ToolText;
				var decalMask = Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
				if (Context.tool != ToolBrush && Context.tool != ToolEraser && Context.tool != ToolClone && !decalMask) return 1.0;
				var val = Context.brushHardness * Context.brushNodesHardness;
				var pen = Input.getPen();
				if (Config.raw.pressure_hardness && pen.down()) {
					val *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				if (Config.raw.brush_3d) {
					if (Context.paint2d) {
						val *= 1.0 / UIView2D.inst.panScale;
					}
					else {
						val *= val;
					}
				}
				return val;
			}
			case "_brushScale": {
				var fill = Context.layer.fill_layer != null;
				var val = (fill ? Context.layer.scale : Context.brushScale) * Context.brushNodesScale;
				return val;
			}
			case "_objectId": {
				return Project.paintObjects.indexOf(cast object);
			}
			case "_vignetteStrength": {
				return Config.raw.rp_vignette;
			}
			case "_coneOffset": {
				return Context.vxaoOffset;
			}
			case "_coneAperture": {
				return Context.vxaoAperture;
			}
			case "_dilateRadius": {
				return UVUtil.dilatemap != null ? Config.raw.dilate_radius : 0.0;
			}
			case "_decalLayerDim": {
				return Context.layer.decalMat.getScale().z * 0.5;
			}
			case "_pickerOpacity": {
				return Context.pickedColor.opacity;
			}
			case "_pickerOcclusion": {
				return Context.pickedColor.occlusion;
			}
			case "_pickerRoughness": {
				return Context.pickedColor.roughness;
			}
			case "_pickerMetallic": {
				return Context.pickedColor.metallic;
			}
			case "_pickerHeight": {
				return Context.pickedColor.height;
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
			case "_brushAngle": {
				var brushAngle = Context.brushAngle + Context.brushNodesAngle;
				var angle = Context.layer.fill_layer != null ? Context.layer.angle : brushAngle;
				angle *= (Math.PI / 180);
				var pen = Input.getPen();
				if (Config.raw.pressure_angle && pen.down()) {
					angle *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				vec.set(Math.cos(-angle), Math.sin(-angle), 0);
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
			case "_brushDirection": {
				v = iron.object.Uniforms.helpVec;
				// Discard first paint for directional brush
				var allowPaint = Context.prevPaintVecX != Context.lastPaintVecX &&
								 Context.prevPaintVecY != Context.lastPaintVecY &&
								 Context.prevPaintVecX > 0 &&
								 Context.prevPaintVecY > 0;
				var x = Context.paintVec.x;
				var y = Context.paintVec.y;
				var lastx = Context.prevPaintVecX;
				var lasty = Context.prevPaintVecY;
				if (Context.paint2d) {
					x = vec2d(x);
					lastx = vec2d(lastx);
				}
				var angle = Math.atan2(-y + lasty, x - lastx) - Math.PI / 2;
				v.set(Math.cos(angle), Math.sin(angle), allowPaint ? 1 : 0);
				Context.prevPaintVecX = Context.lastPaintVecX;
				Context.prevPaintVecY = Context.lastPaintVecY;
				return v;
			}
			case "_decalLayerLoc": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.layer.decalMat._30, Context.layer.decalMat._31, Context.layer.decalMat._32);
				return v;
			}
			case "_decalLayerNor": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.layer.decalMat._20, Context.layer.decalMat._21, Context.layer.decalMat._22).normalize();
				return v;
			}
			case "_pickerBase": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.pickedColor.base.R, Context.pickedColor.base.G, Context.pickedColor.base.B);
				return v;
			}
			case "_pickerNormal": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.pickedColor.normal.R, Context.pickedColor.normal.G, Context.pickedColor.normal.B);
				return v;
			}
			#if arm_physics
			case "_particleHit": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.particleHitX, Context.particleHitY, Context.particleHitZ);
				return v;
			}
			case "_particleHitLast": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.lastParticleHitX, Context.lastParticleHitY, Context.lastParticleHitZ);
				return v;
			}
			#end
		}

		return v;
	}

	static function vec2d(x: Float) {
		// Transform from 3d viewport coord to 2d view coord
		Context.paint2dView = false;
		var res = (x * App.w() - App.w()) / UIView2D.inst.ww;
		Context.paint2dView = true;
		return res;
	}

	public static function linkVec4(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		switch (link) {
			case "_inputBrush": {
				var down = Input.getMouse().down() || Input.getPen().down();
				vec.set(Context.paintVec.x, Context.paintVec.y, down ? 1.0 : 0.0, 0.0);
				if (Context.paint2d) vec.x = vec2d(vec.x);
				return vec;
			}
			case "_inputBrushLast": {
				var down = Input.getMouse().down() || Input.getPen().down();
				vec.set(Context.lastPaintVecX, Context.lastPaintVecY, down ? 1.0 : 0.0, 0.0);
				if (Context.paint2d) vec.x = vec2d(vec.x);
				return vec;
			}
			case "_stencilTransform": {
				vec.set(Context.brushStencilX, Context.brushStencilY, Context.brushStencilScale, Context.brushStencilAngle);
				if (Context.paint2d) vec.x = vec2d(vec.x);
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
			case "_decalMask": {
				var decal = Context.tool == ToolDecal || Context.tool == ToolText;
				var decalMask = Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
				var val = (Context.brushRadius * Context.brushNodesRadius) / 15.0;
				var scale2d = (900 / App.h()) * Config.raw.window_scale;
				val *= scale2d; // Projection ratio
				vec.set(Context.decalX, Context.decalY, decalMask ? 1 : 0, val);
				if (Context.paint2d) vec.x = vec2d(vec.x);
				return vec;
			}
		}
		return null;
	}

	public static function linkMat4(object: Object, mat: MaterialData, link: String): iron.math.Mat4 {
		switch (link) {
			case "_decalLayerMatrix": { // Decal layer
				var camera = Scene.active.camera;
				var m = iron.object.Uniforms.helpMat;
				m.setFrom(Context.layer.decalMat);
				m.getInverse(m);
				m.multmat(orthoP);
				return m;
			}
		}
		return null;
	}

	public static function linkTex(object: Object, mat: MaterialData, link: String): kha.Image {
		switch (link) {
			case "_texcolorid": {
				if (Project.assets.length == 0) return RenderPath.active.renderTargets.get("empty_white").image;
				else return Project.getImage(Project.assets[Context.colorIdHandle.position]);
			}
			case "_texuvmap": {
				if (!UVUtil.uvmapCached) {
					function _init() {
						UVUtil.cacheUVMap();
					}
					iron.App.notifyOnInit(_init);
				}
				return UVUtil.uvmap;
			}
			case "_textrianglemap": {
				if (!UVUtil.trianglemapCached) {
					function _init() {
						UVUtil.cacheTriangleMap();
					}
					iron.App.notifyOnInit(_init);
				}
				return UVUtil.trianglemap;
			}
			case "_texuvislandmap": {
				function _init() {
					UVUtil.cacheUVIslandMap();
				}
				iron.App.notifyOnInit(_init);
				return UVUtil.uvislandmapCached ? UVUtil.uvislandmap : RenderPath.active.renderTargets.get("empty_black").image;
			}
			case "_texdilatemap": {
				return UVUtil.dilatemap;
			}
			case "_textexttool": { // Opacity map for text
				return Context.textToolImage;
			}
			case "_texbrushmask": {
				return Context.brushMaskImage;
			}
			case "_texbrushstencil": {
				return Context.brushStencilImage;
			}
			case "_texpaint_undo": {
				var i = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
				return RenderPath.active.renderTargets.get("texpaint_undo" + i).image;
			}
			case "_texpaint_nor_undo": {
				var i = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
				return RenderPath.active.renderTargets.get("texpaint_nor_undo" + i).image;
			}
			case "_texpaint_pack_undo": {
				var i = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
				return RenderPath.active.renderTargets.get("texpaint_pack_undo" + i).image;
			}
			case "_texparticle": {
				return RenderPath.active.renderTargets.get("texparticle").image;
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
			return tid < Project.layers.length ? Project.layers[tid].texpaint : null;
		}
		if (link.startsWith("_texpaint_nor")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint_nor : null;
		}
		if (link.startsWith("_texpaint_pack")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint_pack : null;
		}
		if (link.startsWith("_texpaint")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint : null;
		}
		if (link.startsWith("_texblur_")) {
			var id = link.substr(9);
			return Context.nodePreviews != null ? Context.nodePreviews.get(id) : RenderPath.active.renderTargets.get("empty_black").image;
		}
		if (link.startsWith("_texwarp_")) {
			var id = link.substr(9);
			return Context.nodePreviews != null ? Context.nodePreviews.get(id) : RenderPath.active.renderTargets.get("empty_black").image;
		}
		if (link.startsWith("_texbake_")) {
			var id = link.substr(9);
			return Context.nodePreviews != null ? Context.nodePreviews.get(id) : RenderPath.active.renderTargets.get("empty_black").image;
		}

		return null;
	}
}
