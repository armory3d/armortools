package arm.render;

import iron.math.Vec4;
import iron.math.Mat4;
import iron.math.Quat;
import iron.object.MeshObject;
import iron.system.Input;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UIHeader;
import arm.ui.UIView2D;
import arm.Enums;

class Inc {

	static var path: RenderPath;
	public static var superSample = 1.0;

	static var lastX = -1.0;
	static var lastY = -1.0;

	#if rp_voxels
	static var voxelsCreated = false;
	#end

	public static function init(_path: RenderPath) {
		path = _path;
		var config = Config.raw;
		superSample = config.rp_supersample;
	}

	public static function applyConfig() {
		var config = Config.raw;
		if (superSample != config.rp_supersample) {
			superSample = config.rp_supersample;
			for (rt in path.renderTargets) {
				if (rt.raw.width == 0 && rt.raw.scale != null) {
					rt.raw.scale = getSuperSampling();
				}
			}
			path.resize();
		}
		#if rp_voxels
		if (!voxelsCreated) initGI();
		#end
	}

	#if rp_voxels
	public static function initGI(tname = "voxels") {
		var config = Config.raw;
		if (config.rp_gi != true || voxelsCreated) return;
		voxelsCreated = true;

		var t = new RenderTargetRaw();
		t.name = tname;
		t.format = "R8";
		var res = 256;
		var resZ =  1.0;
		t.width = res;
		t.height = res;
		t.depth = Std.int(res * resZ);
		t.is_image = true;
		t.mipmaps = true;
		path.createRenderTarget(t);

		#if arm_voxelgi_temporal
		{
			var tB = new RenderTargetRaw();
			tB.name = t.name + "B";
			tB.format = t.format;
			tB.width = t.width;
			tB.height = t.height;
			tB.depth = t.depth;
			tB.is_image = t.is_image;
			tB.mipmaps = t.mipmaps;
			path.createRenderTarget(tB);
		}
		#end
	}
	#end

	public static inline function getSuperSampling(): Float {
		return superSample;
	}

	public static function drawCompass(currentG: kha.graphics4.Graphics) {
		if (Context.showCompass) {
			var scene = Scene.active;
			var cam = Scene.active.camera;
			var compass: MeshObject = cast scene.getChild(".Compass");

			var visible = compass.visible;
			var parent = compass.parent;
			var loc = compass.transform.loc;
			var rot = compass.transform.rot;
			var crot = cam.transform.rot;
			var ratio = iron.App.w() / iron.App.h();
			var P = cam.P;
			cam.P = Mat4.ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);
			compass.visible = true;
			compass.parent = cam;
			compass.transform.loc = new Vec4(7.4 * ratio, 7.0, -1);
			compass.transform.rot = new Quat(-crot.x, -crot.y, -crot.z, crot.w);
			compass.transform.scale.set(0.4, 0.4, 0.4);
			compass.transform.buildMatrix();

			compass.frustumCulling = false;
			compass.render(currentG, "overlay", []);

			cam.P = P;
			compass.visible = visible;
			compass.parent = parent;
			compass.transform.loc = loc;
			compass.transform.rot = rot;
			compass.transform.buildMatrix();
		}
	}

	public static function beginSplit() {
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
		endSplit();

		if (Context.foregroundEvent && !iron.system.Input.getMouse().down()) {
			Context.foregroundEvent = false;
			Context.pdirty = 0;
		}
	}

	static function endSplit() {
		Context.viewIndexLast = Context.viewIndex;
		Context.viewIndex = -1;
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

	static var bloomMipmaps: Array<RenderTarget>;
	static var bloomCurrentMip = 0;
	static var bloomSampleScale: Float;

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

				path.loadShader("shader_datas/bloom_pass/downsample_pass");
				path.loadShader("shader_datas/bloom_pass/upsample_pass");

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
				path.drawShader("shader_datas/bloom_pass/downsample_pass");
			}
			for (i in 0...numMips) {
				var mipLevel = numMips - 1 - i;
				bloomCurrentMip = mipLevel;
				path.setTarget(mipLevel == 0 ? tex : bloomMipmaps[mipLevel - 1].raw.name);
				path.bindTarget(bloomMipmaps[mipLevel].raw.name, "tex");
				path.drawShader("shader_datas/bloom_pass/upsample_pass");
			}
		}
	}
}
