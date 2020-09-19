package arm.render;

import iron.data.MaterialData;
import iron.object.Object;
import iron.system.Input;
import iron.math.Vec4;
import iron.math.Mat4;
import iron.RenderPath;
import iron.Scene;
#if arm_painter
import arm.ui.UISidebar;
import arm.util.UVUtil;
import arm.Enums;
#end

class Uniforms {

	static var vec = new Vec4();

	public static function init() {
		iron.object.Uniforms.externalFloatLinks = [linkFloat];
		iron.object.Uniforms.externalVec2Links = [linkVec2];
		iron.object.Uniforms.externalVec3Links = [linkVec3];
		iron.object.Uniforms.externalVec4Links = [linkVec4];
		iron.object.Uniforms.externalMat4Links = [linkMat4];
		iron.object.Uniforms.externalTextureLinks = [linkTex];
	}

	public static function linkFloat(object: Object, mat: MaterialData, link: String): Null<kha.FastFloat> {
		#if arm_painter
		switch (link) {
			case "_brushRadius": {
				var val = (Context.brushRadius * Context.brushNodesRadius) / 15.0;
				var pen = Input.getPen();
				if (Config.raw.pressure_radius && pen.down()) {
					val *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				var scale2d = (900 / App.h()) * Config.raw.window_scale;
				var decal = Context.tool == ToolDecal || Context.tool == ToolText;
				if (Config.raw.brush_3d && !decal) {
					val *= Context.paint2d ? 0.55 * scale2d : 2;
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
				if (Context.tool != ToolBrush && Context.tool != ToolEraser) return 1.0;
				var val = Context.brushHardness * Context.brushNodesHardness;
				var pen = Input.getPen();
				if (Config.raw.pressure_hardness && pen.down()) {
					val *= pen.pressure * Config.raw.pressure_sensitivity;
				}
				if (Config.raw.brush_3d && !Context.paint2d) {
					val *= val;
				}
				return val;
			}
			case "_brushScale": {
				var fill = Context.layer.fill_layer != null;
				var val = (fill ? Context.layer.scale : Context.brushScale) * Context.brushNodesScale;
				return val;
			}
			case "_objectId": {
				return Project.paintObjects.indexOf(Context.paintObject);
			}
			#end
			#if arm_world
			case "_voxelgiHalfExtentsUni": {
				#if arm_painter
				return Context.vxaoExt;
				#else
				return 10.0;
				#end
			}
			#end
			case "_vignetteStrength": {
				#if arm_painter
				return Config.raw.rp_vignette;
				#else
				return 0.4;
				#end
			}
			case "_coneOffset": {
				#if arm_painter
				return Context.vxaoOffset;
				#else
				return 1.5;
				#end
			}
			case "_coneAperture": {
				#if arm_painter
				return Context.vxaoAperture;
				#else
				return 1.2;
				#end
			}
			case "_dilateRadius": {
				#if arm_painter
				return Context.dilateRadius;
				#else
				return 8.0;
				#end
			}
		}
		return null;
	}

	public static function linkVec2(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		#if arm_painter
		switch (link) {
			case "_sub": {
				Context.sub = (Context.sub + 1) % 4;
				var eps = Context.brushBias * 0.00022 * Config.getTextureResBias();
				Context.sub == 0 ? vec.set(eps, eps, 0.0) :
				Context.sub == 1 ? vec.set(eps, -eps, 0.0) :
				Context.sub == 2 ? vec.set(-eps, -eps, 0.0) :
										  vec.set(-eps, eps, 0.0);
				return vec;
			}
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
				vec.set(Math.cos(angle), Math.sin(angle), 0);
				return vec;
			}
			case "_texpaintSize": {
				vec.set(Config.getTextureResX(), Config.getTextureResY(), 0);
				return vec;
			}
		}
		#end
		return null;
	}

	public static function linkVec3(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		var v: Vec4 = null;
		switch (link) {
			#if arm_world
			case "_hosekA": {
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
			case "_hosekB": {
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
			case "_hosekC": {
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
			case "_hosekD": {
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
			case "_hosekE": {
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
			case "_hosekF": {
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
			case "_hosekG": {
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
			case "_hosekH": {
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
			case "_hosekI": {
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
			case "_hosekZ": {
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
			#if arm_painter
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
				if (Context.paint2d) { x -= 1.0; lastx -= 1.0; }
				var angle = Math.atan2(-y + lasty, x - lastx) - Math.PI / 2;
				v.set(Math.cos(angle), Math.sin(angle), allowPaint ? 1 : 0);
				Context.prevPaintVecX = Context.lastPaintVecX;
				Context.prevPaintVecY = Context.lastPaintVecY;
				return v;
			}
			#end
		}

		return v;
	}

	public static function linkVec4(object: Object, mat: MaterialData, link: String): iron.math.Vec4 {
		#if arm_painter
		switch (link) {
			case "_inputBrush": {
				var down = Input.getMouse().down() || Input.getPen().down();
				vec.set(Context.paintVec.x, Context.paintVec.y, down ? 1.0 : 0.0, 0.0);
				if (Context.paint2d) vec.x -= 1.0;
				return vec;
			}
			case "_inputBrushLast": {
				var down = Input.getMouse().down() || Input.getPen().down();
				vec.set(Context.lastPaintVecX, Context.lastPaintVecY, down ? 1.0 : 0.0, 0.0);
				if (Context.paint2d) vec.x -= 1.0;
				return vec;
			}
			case "_stencilTransform": {
				vec.set(Context.brushStencilX, Context.brushStencilY, Context.brushStencilScale, Context.brushStencilAngle);
				if (Context.paint2d) vec.x -= 1.0;
				return vec;
			}
			case "_envmapData": {
				vec.set(Context.envmapAngle, Math.sin(-Context.envmapAngle), Math.cos(-Context.envmapAngle), Scene.active.world.probe.raw.strength);
				return vec;
			}
		}
		#end
		return null;
	}

	public static function linkMat4(object: Object, mat: MaterialData, link: String): iron.math.Mat4 {
		switch (link) {
			case "_decalLayerMatrix": { // Decal layer
				var camera = Scene.active.camera;
				var m = iron.object.Uniforms.helpMat;
				m.setFrom(Context.layer.decalMat);
				m.getInverse(m);
				m.multmat(camera.P);
				return m;
			}
		}
		return null;
	}

	public static function linkTex(object: Object, mat: MaterialData, link: String): kha.Image {
		switch (link) {
			#if arm_painter
			case "_texcolorid": {
				if (Project.assets.length == 0) return RenderPath.active.renderTargets.get("empty_white").image;
				else return UISidebar.inst.getImage(Project.assets[Context.colorIdHandle.position]);
			}
			case "_texuvmap": {
				if (!UVUtil.uvmapCached) {
					function _render(_) {
						UVUtil.cacheUVMap();
						iron.App.removeRender(_render);
					}
					iron.App.notifyOnRender(_render);
				}
				return UVUtil.uvmap;
			}
			case "_textrianglemap": {
				if (!UVUtil.trianglemapCached) {
					function _render(_) {
						UVUtil.cacheTriangleMap();
						iron.App.removeRender(_render);
					}
					iron.App.notifyOnRender(_render);
				}
				return UVUtil.trianglemap;
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
			case "_texpaint_mask": {
				return Context.layer.texpaint_mask;
			}
			case "_texparticle": {
				return RenderPath.active.renderTargets.get("texparticle").image;
			}
			#end
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

		#if arm_painter
		if (link.startsWith("_texpaint_pack_vert")) {
			var tid = link.substr(link.length - 1);
			return RenderPath.active.renderTargets.get("texpaint_pack" + tid).image;
		}
		if (link.startsWith("_texpaint_mask_vert")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return Project.layers[tid].texpaint_mask;
		}
		if (link.startsWith("_texpaint_mask")) {
			var tid = Std.parseInt(link.substr(link.length - 1));
			return tid < Project.layers.length ? Project.layers[tid].texpaint_mask : null;
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
			return Context.nodePreviewsBlur != null ? Context.nodePreviewsBlur.get(id) : RenderPath.active.renderTargets.get("empty_black").image;
		}
		#end

		return null;
	}
}
