package arm.render;

import kha.System;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UISidebar;

class RenderPathForward {

	public static var path: RenderPath;

	public static function init(_path: RenderPath) {
		path = _path;
	}

	@:access(iron.RenderPath)
	public static function commands() {

		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		#if arm_painter
		Inc.beginSplit();

		if (Inc.isCached()) return;

		// Match projection matrix jitter
		var skipTaa = Context.splitView;
		if (!skipTaa) {
			@:privateAccess Scene.active.camera.frame = RenderPathDeferred.taaFrame;
			@:privateAccess Scene.active.camera.projectionJitter();
		}
		Scene.active.camera.buildMatrix();

		RenderPathPaint.begin();

		drawSplit();
		#end // arm_painter

		drawGbuffer();

		#if arm_painter
		RenderPathPaint.draw();
		#end

		#if (kha_direct3d12 || kha_vulkan)
		if (Context.viewportMode == ViewPathTrace) {
			RenderPathRaytrace.draw();
			return;
		}
		#end

		drawForward();

		#if arm_painter
		RenderPathPaint.end();
		Inc.endSplit();
		#end

		RenderPathDeferred.taaFrame++;
	}

	static function drawGbuffer() {
		path.setTarget("gbuffer0");
		#if kha_metal
		path.clearTarget(0x00000000, 1.0);
		#else
		path.clearTarget(null, 1.0);
		#end
		path.setTarget("gbuffer2");
		path.clearTarget(0xff000000);
		path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		var currentG = path.currentG;
		#if arm_painter
		RenderPathPaint.bindLayers();
		#end
		path.drawMeshes("mesh");
		#if arm_painter
		RenderPathPaint.unbindLayers();
		#end
		LineDraw.render(path.currentG);
	}

	static function drawForward() {
		path.setDepthFrom("gbuffer1", "gbuffer0");
		path.setTarget("gbuffer1");
		path.drawSkydome("world_pass/world_pass/world_pass");
		path.setDepthFrom("gbuffer1", "gbuffer2");

		path.setTarget("buf");
		path.bindTarget("gbuffer1", "tex");
		path.drawShader("shader_datas/compositor_pass/compositor_pass");

		path.setTarget("buf");
		var currentG = path.currentG;
		path.drawMeshes("overlay");
		#if arm_painter
		Inc.drawCompass(currentG);
		#end

		var taaFrame = RenderPathDeferred.taaFrame;
		var current = taaFrame % 2 == 0 ? "bufa" : "taa2";
		var last = taaFrame % 2 == 0 ? "taa2" : "bufa";

		path.setTarget(current);
		path.clearTarget(0x00000000);
		path.bindTarget("buf", "colorTex");
		path.drawShader("shader_datas/smaa_edge_detect/smaa_edge_detect");

		path.setTarget("taa");
		path.clearTarget(0x00000000);
		path.bindTarget(current, "edgesTex");
		path.drawShader("shader_datas/smaa_blend_weight/smaa_blend_weight");

		path.setTarget(current);
		path.bindTarget("buf", "colorTex");
		path.bindTarget("taa", "blendTex");
		path.bindTarget("gbuffer2", "sveloc");
		path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

		#if arm_painter
		var skipTaa = Context.splitView;
		#else
		var skipTaa = false;
		#end

		if (skipTaa) {
			path.setTarget("taa");
			path.bindTarget(current, "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		else {
			path.setTarget("taa");
			path.bindTarget(current, "tex");
			path.bindTarget(last, "tex2");
			path.bindTarget("gbuffer2", "sveloc");
			path.drawShader("shader_datas/taa_pass/taa_pass");
		}

		if (!Inc.ssaa4()) {
			path.setTarget("");
			path.bindTarget(taaFrame == 0 ? current : "taa", "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}

		if (Inc.ssaa4()) {
			path.setTarget("");
			path.bindTarget(taaFrame % 2 == 0 ? "taa2" : "taa", "tex");
			path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
		}
	}

	static function drawSplit() {
		if (Context.splitView) {
			if (Context.pdirty > 0) {
				var cam = Scene.active.camera;

				Context.viewIndex = Context.viewIndex == 0 ? 1 : 0;
				cam.transform.setMatrix(arm.plugin.Camera.inst.views[Context.viewIndex]);
				cam.buildMatrix();
				cam.buildProjection();

				drawGbuffer();

				#if (kha_direct3d12 || kha_vulkan)
				Context.viewportMode == ViewPathTrace ? RenderPathRaytrace.draw() : drawForward();
				#else
				drawForward();
				#end

				Context.viewIndex = Context.viewIndex == 0 ? 1 : 0;
				cam.transform.setMatrix(arm.plugin.Camera.inst.views[Context.viewIndex]);
				cam.buildMatrix();
				cam.buildProjection();
			}
		}
	}
}
