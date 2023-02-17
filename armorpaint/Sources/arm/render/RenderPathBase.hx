package arm.render;

import iron.math.Vec4;
import iron.math.Mat4;
import iron.math.Quat;
import iron.object.MeshObject;
import iron.system.Input;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UIView2D;
import arm.Enums;

class RenderPathBase {

	static var superSample = 1.0;
	static var path: RenderPath;
	static var lastX = -1.0;
	static var lastY = -1.0;
	static var bloomMipmaps: Array<RenderTarget>;
	static var bloomCurrentMip = 0;
	static var bloomSampleScale: Float;
	#if rp_voxels
	static var voxelsCreated = false;
	#end

	public static function init(_path: RenderPath) {
		path = _path;
		superSample = Config.raw.rp_supersample;
	}

	#if rp_voxels
	public static function initVoxels(targetName = "voxels") {
		if (Config.raw.rp_gi != true || voxelsCreated) return;
		voxelsCreated = true;

		{
			var t = new RenderTargetRaw();
			t.name = targetName;
			t.format = "R8";
			t.width = 256;
			t.height = 256;
			t.depth = 256;
			t.is_image = true;
			t.mipmaps = true;
			path.createRenderTarget(t);
		}
	}
	#end

	public static function applyConfig() {
		if (superSample != Config.raw.rp_supersample) {
			superSample = Config.raw.rp_supersample;
			for (rt in path.renderTargets) {
				if (rt.raw.width == 0 && rt.raw.scale != null) {
					rt.raw.scale = superSample;
				}
			}
			path.resize();
		}
		#if rp_voxels
		if (!voxelsCreated) initVoxels();
		#end
	}

	public static inline function getSuperSampling(): Float {
		return superSample;
	}

	public static function drawCompass(currentG: kha.graphics4.Graphics) {
		if (Context.showCompass) {
			var scene = Scene.active;
			var cam = scene.camera;
			var compass: MeshObject = cast scene.getChild(".Compass");

			var _visible = compass.visible;
			var _parent = compass.parent;
			var _loc = compass.transform.loc;
			var _rot = compass.transform.rot;
			var crot = cam.transform.rot;
			var ratio = iron.App.w() / iron.App.h();
			var _P = cam.P;
			cam.P = Mat4.ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);
			compass.visible = true;
			compass.parent = cam;
			compass.transform.loc = new Vec4(7.4 * ratio, 7.0, -1);
			compass.transform.rot = new Quat(-crot.x, -crot.y, -crot.z, crot.w);
			compass.transform.scale.set(0.4, 0.4, 0.4);
			compass.transform.buildMatrix();
			compass.frustumCulling = false;
			compass.render(currentG, "overlay", []);

			cam.P = _P;
			compass.visible = _visible;
			compass.parent = _parent;
			compass.transform.loc = _loc;
			compass.transform.rot = _rot;
			compass.transform.buildMatrix();
		}
	}

	public static function begin() {
		// Begin split
		if (Context.splitView && !Context.paint2dView) {
			if (Context.viewIndexLast == -1 && Context.viewIndex == -1) {
				// Begin split, draw right viewport first
				Context.viewIndex = 1;
			}
			else {
				// Set current viewport
				Context.viewIndex = Input.getMouse().viewX > arm.App.w() / 2 ? 1 : 0;
			}

			var cam = Scene.active.camera;
			if (Context.viewIndexLast > -1) {
				// Save current viewport camera
				arm.Camera.inst.views[Context.viewIndexLast].setFrom(cam.transform.local);
			}

			var decal = Context.tool == ToolDecal || Context.tool == ToolText;

			if (Context.viewIndexLast != Context.viewIndex || decal || !Config.raw.brush_3d) {
				// Redraw on current viewport change
				Context.ddirty = 1;
			}

			cam.transform.setMatrix(arm.Camera.inst.views[Context.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();
		}
	}

	public static function end() {
		// End split
		Context.viewIndexLast = Context.viewIndex;
		Context.viewIndex = -1;

		if (Context.foregroundEvent && !iron.system.Input.getMouse().down()) {
			Context.foregroundEvent = false;
			Context.pdirty = 0;
		}
	}

	public static inline function ssaa4(): Bool {
		return Config.raw.rp_supersample == 4;
	}

	public static function isCached(): Bool {
		var mouse = Input.getMouse();
		var mx = lastX;
		var my = lastY;
		lastX = mouse.viewX;
		lastY = mouse.viewY;

		if (Config.raw.brush_live && Context.pdirty <= 0) {
			var inViewport = Context.paintVec.x < 1 && Context.paintVec.x > 0 &&
							 Context.paintVec.y < 1 && Context.paintVec.y > 0;
			var in2dView = UIView2D.inst.show && UIView2D.inst.type == View2DLayer &&
						   mx > UIView2D.inst.wx && mx < UIView2D.inst.wx + UIView2D.inst.ww &&
						   my > UIView2D.inst.wy && my < UIView2D.inst.wy + UIView2D.inst.wh;
			var moved = (mx != lastX || my != lastY) && (inViewport || in2dView);
			if (moved || Context.brushLocked) {
				Context.rdirty = 2;
			}
		}

		if (Context.ddirty <= 0 && Context.rdirty <= 0 && Context.pdirty <= 0) {
			if (mx != lastX || my != lastY || mouse.locked) Context.ddirty = 0;
			#if (kha_metal || krom_android)
			if (Context.ddirty > -6) {
			#else
			if (Context.ddirty > -2) {
			#end
				path.setTarget("");
				path.bindTarget("taa", "tex");
				ssaa4() ?
					path.drawShader("shader_datas/supersample_resolve/supersample_resolve") :
					path.drawShader("shader_datas/copy_pass/copy_pass");
				if (Config.raw.brush_3d) RenderPathPaint.commandsCursor();
				if (Context.ddirty <= 0) Context.ddirty--;
			}
			end();
			if (!RenderPathPaint.dilated) {
				RenderPathPaint.dilate(Config.raw.dilate == DilateDelayed, true);
				RenderPathPaint.dilated = true;
			}
			return true;
		}
		return false;
	}

	public static function commandsBloom(tex = "tex") {
		if (Config.raw.rp_bloom != false) {
			if (bloomMipmaps == null) {
				bloomMipmaps = [];

				var prevScale = 1.0;
				for (i in 0...10) {
					var t = new RenderTargetRaw();
					t.name = "bloom_mip_" + i;
					t.width = 0;
					t.height = 0;
					t.scale = (prevScale *= 0.5);
					t.format = "RGBA64";
					bloomMipmaps.push(path.createRenderTarget(t));
				}

				path.loadShader("shader_datas/bloom_pass/bloom_downsample_pass");
				path.loadShader("shader_datas/bloom_pass/bloom_upsample_pass");

				iron.object.Uniforms.externalIntLinks.push(function(_, _, link: String) {
					if (link == "_bloomCurrentMip") return bloomCurrentMip;
					return null;
				});
				iron.object.Uniforms.externalFloatLinks.push(function(_, _, link: String) {
					if (link == "_bloomSampleScale") return bloomSampleScale;
					return null;
				});
			}

			var bloomRadius = 6.5;
			var minDim = Math.min(path.currentW, path.currentH);
			var logMinDim = Math.max(1.0, js.lib.Math.log2(minDim) + (bloomRadius - 8.0));
			var numMips = Std.int(logMinDim);
			bloomSampleScale = 0.5 + logMinDim - numMips;

			for (i in 0...numMips) {
				bloomCurrentMip = i;
				path.setTarget(bloomMipmaps[i].raw.name);
				path.clearTarget();
				path.bindTarget(i == 0 ? tex : bloomMipmaps[i - 1].raw.name, "tex");
				path.drawShader("shader_datas/bloom_pass/bloom_downsample_pass");
			}
			for (i in 0...numMips) {
				var mipLevel = numMips - 1 - i;
				bloomCurrentMip = mipLevel;
				path.setTarget(mipLevel == 0 ? tex : bloomMipmaps[mipLevel - 1].raw.name);
				path.bindTarget(bloomMipmaps[mipLevel].raw.name, "tex");
				path.drawShader("shader_datas/bloom_pass/bloom_upsample_pass");
			}
		}
	}
}
