package arm.render;

import iron.data.MaterialData;
import iron.object.Object;
import iron.system.Input;
import iron.math.Vec4;
import iron.math.Mat4;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UIView2D;
import arm.util.UVUtil;
import arm.shader.MaterialParser;

class Uniforms {

	static var vec = new Vec4();
	static var orthoP = Mat4.ortho(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);

	public static function init() {
		iron.object.Uniforms.externalIntLinks = [linkInt];
		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];
		iron.object.Uniforms.externalVec3Links = [linkVec3];
		iron.object.Uniforms.externalVec4Links = [linkVec4];
		iron.object.Uniforms.externalMat4Links = [linkMat4];
		iron.object.Uniforms.externalTextureLinks = [linkTex];
	}

	public static function linkInt(object: Object, mat: MaterialData, link: String): Null<Int> {
		return null;
	}

	public static function linkFloat(object: Object, mat: MaterialData, link: String): Null<kha.FastFloat> {
		switch (link) {
			case "_brushRadius": {
				var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
				var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
				var brushDecalMaskRadius = Context.raw.brushDecalMaskRadius;
				if (Config.raw.brush_3d) {
					brushDecalMaskRadius *= Context.raw.paint2d ? 0.55 * UIView2D.inst.panScale : 2.0;
				}
				var radius = decalMask ? brushDecalMaskRadius : Context.raw.brushRadius;
				var val = (radius * Context.raw.brushNodesRadius) / 15.0;
				var pen = Input.getPen();
				if (Config.raw.pressure_radius && pen.down()) {
					val *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				var scale2d = (900 / App.h()) * Config.raw.window_scale;

				if (Config.raw.brush_3d && !decal) {
					val *= Context.raw.paint2d ? 0.55 * scale2d * UIView2D.inst.panScale : 2;
				}
				else {
					val *= scale2d; // Projection ratio
				}
				return val;
			}
			case "_brushScaleX": {
				return 1 / Context.raw.brushScaleX;
			}
			case "_brushOpacity": {
				var val = Context.raw.brushOpacity * Context.raw.brushNodesOpacity;
				var pen = Input.getPen();
				if (Config.raw.pressure_opacity && pen.down()) {
					val *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				return val;
			}
			case "_brushHardness": {
				var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
				var decalMask = Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
				if (Context.raw.tool != ToolBrush && Context.raw.tool != ToolEraser && Context.raw.tool != ToolClone && !decalMask) return 1.0;
				var val = Context.raw.brushHardness * Context.raw.brushNodesHardness;
				var pen = Input.getPen();
				if (Config.raw.pressure_hardness && pen.down()) {
					val *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				if (Config.raw.brush_3d) {
					if (Context.raw.paint2d) {
						val *= 1.0 / UIView2D.inst.panScale;
					}
					else {
						val *= val;
					}
				}
				return val;
			}
			case "_brushScale": {
				var fill = Context.raw.layer.fill_layer != null;
				var val = (fill ? Context.raw.layer.scale : Context.raw.brushScale) * Context.raw.brushNodesScale;
				return val;
			}
			case "_objectId": {
				return Project.paintObjects.indexOf(cast object);
			}
			case "_vignetteStrength": {
				return Config.raw.rp_vignette;
			}
			case "_coneOffset": {
				return Context.raw.vxaoOffset;
			}
			case "_coneAperture": {
				return Context.raw.vxaoAperture;
			}
			case "_dilateRadius": {
				return UVUtil.dilatemap != null ? Config.raw.dilate_radius : 0.0;
			}
			case "_decalLayerDim": {
				return Context.raw.layer.decalMat.getScale().z * 0.5;
			}
			case "_pickerOpacity": {
				return Context.raw.pickedColor.opacity;
			}
			case "_pickerOcclusion": {
				return Context.raw.pickedColor.occlusion;
			}
			case "_pickerRoughness": {
				return Context.raw.pickedColor.roughness;
			}
			case "_pickerMetallic": {
				return Context.raw.pickedColor.metallic;
			}
			case "_pickerHeight": {
				return Context.raw.pickedColor.height;
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
				vec.set(Context.raw.cloneDeltaX, Context.raw.cloneDeltaY, 0);
				return vec;
			}
			case "_brushAngle": {
				var brushAngle = Context.raw.brushAngle + Context.raw.brushNodesAngle;
				var angle = Context.raw.layer.fill_layer != null ? Context.raw.layer.angle : brushAngle;
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
				var allowPaint = Context.raw.prevPaintVecX != Context.raw.lastPaintVecX &&
								 Context.raw.prevPaintVecY != Context.raw.lastPaintVecY &&
								 Context.raw.prevPaintVecX > 0 &&
								 Context.raw.prevPaintVecY > 0;
				var x = Context.raw.paintVec.x;
				var y = Context.raw.paintVec.y;
				var lastx = Context.raw.prevPaintVecX;
				var lasty = Context.raw.prevPaintVecY;
				if (Context.raw.paint2d) {
					x = vec2d(x);
					lastx = vec2d(lastx);
				}
				var angle = Math.atan2(-y + lasty, x - lastx) - Math.PI / 2;
				v.set(Math.cos(angle), Math.sin(angle), allowPaint ? 1 : 0);
				Context.raw.prevPaintVecX = Context.raw.lastPaintVecX;
				Context.raw.prevPaintVecY = Context.raw.lastPaintVecY;
				return v;
			}
			case "_decalLayerLoc": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.raw.layer.decalMat._30, Context.raw.layer.decalMat._31, Context.raw.layer.decalMat._32);
				return v;
			}
			case "_decalLayerNor": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.raw.layer.decalMat._20, Context.raw.layer.decalMat._21, Context.raw.layer.decalMat._22).normalize();
				return v;
			}
			case "_pickerBase": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.raw.pickedColor.base.R, Context.raw.pickedColor.base.G, Context.raw.pickedColor.base.B);
				return v;
			}
			case "_pickerNormal": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.raw.pickedColor.normal.R, Context.raw.pickedColor.normal.G, Context.raw.pickedColor.normal.B);
				return v;
			}
			#if arm_physics
			case "_particleHit": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.raw.particleHitX, Context.raw.particleHitY, Context.raw.particleHitZ);
				return v;
			}
			case "_particleHitLast": {
				v = iron.object.Uniforms.helpVec;
				v.set(Context.raw.lastParticleHitX, Context.raw.lastParticleHitY, Context.raw.lastParticleHitZ);
				return v;
			}
			#end
		}

		return v;
	}

	static function vec2d(x: Float) {
		// Transform from 3d viewport coord to 2d view coord
		Context.raw.paint2dView = false;
		var res = (x * App.w() - App.w()) / UIView2D.inst.ww;
		Context.raw.paint2dView = true;
		return res;
	}

	public static function linkVec4(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		switch (link) {
			case "_inputBrush": {
				var down = Input.getMouse().down() || Input.getPen().down();
				vec.set(Context.raw.paintVec.x, Context.raw.paintVec.y, down ? 1.0 : 0.0, 0.0);
				if (Context.raw.paint2d) vec.x = vec2d(vec.x);
				return vec;
			}
			case "_inputBrushLast": {
				var down = Input.getMouse().down() || Input.getPen().down();
				vec.set(Context.raw.lastPaintVecX, Context.raw.lastPaintVecY, down ? 1.0 : 0.0, 0.0);
				if (Context.raw.paint2d) vec.x = vec2d(vec.x);
				return vec;
			}
			case "_stencilTransform": {
				vec.set(Context.raw.brushStencilX, Context.raw.brushStencilY, Context.raw.brushStencilScale, Context.raw.brushStencilAngle);
				if (Context.raw.paint2d) vec.x = vec2d(vec.x);
				return vec;
			}
			case "_envmapData": {
				vec.set(Context.raw.envmapAngle, Math.sin(-Context.raw.envmapAngle), Math.cos(-Context.raw.envmapAngle), Scene.active.world.probe.raw.strength);
				return vec;
			}
			case "_envmapDataWorld": {
				vec.set(Context.raw.envmapAngle, Math.sin(-Context.raw.envmapAngle), Math.cos(-Context.raw.envmapAngle), Context.raw.showEnvmap ? Scene.active.world.probe.raw.strength : 1.0);
				return vec;
			}
			case "_decalMask": {
				var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
				var decalMask = Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
				var val = (Context.raw.brushRadius * Context.raw.brushNodesRadius) / 15.0;
				var scale2d = (900 / App.h()) * Config.raw.window_scale;
				val *= scale2d; // Projection ratio
				vec.set(Context.raw.decalX, Context.raw.decalY, decalMask ? 1 : 0, val);
				if (Context.raw.paint2d) vec.x = vec2d(vec.x);
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
				m.setFrom(Context.raw.layer.decalMat);
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
				else return Project.getImage(Project.assets[Context.raw.colorIdHandle.position]);
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
				return Context.raw.textToolImage;
			}
			case "_texbrushmask": {
				return Context.raw.brushMaskImage;
			}
			case "_texbrushstencil": {
				return Context.raw.brushStencilImage;
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
			return Context.raw.nodePreviews != null ? Context.raw.nodePreviews.get(id) : RenderPath.active.renderTargets.get("empty_black").image;
		}
		if (link.startsWith("_texwarp_")) {
			var id = link.substr(9);
			return Context.raw.nodePreviews != null ? Context.raw.nodePreviews.get(id) : RenderPath.active.renderTargets.get("empty_black").image;
		}
		if (link.startsWith("_texbake_")) {
			var id = link.substr(9);
			return Context.raw.nodePreviews != null ? Context.raw.nodePreviews.get(id) : RenderPath.active.renderTargets.get("empty_black").image;
		}

		return null;
	}
}
