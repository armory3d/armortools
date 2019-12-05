package arm.render;

import kha.System;
import iron.system.Input;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UITrait;
import arm.Tool;

class RenderPathForward {

	public static var path:RenderPath;
	static var lastX = -1.0;
	static var lastY = -1.0;

	public static function init(_path:RenderPath) {
		path = _path;

		// Already loaded in deferred

		// var t = new RenderTargetRaw();
		// t.name = "lbuffer0";
		// t.width = 0;
		// t.height = 0;
		// t.format = "RGBA64";
		// t.scale = Inc.getSuperSampling();
		// t.depth_buffer = "main";
		// path.createRenderTarget(t);

		// path.loadShader("world_pass/world_pass/world_pass");

		// path.createDepthBuffer("main", "DEPTH24");
		// {
		// 	var t = new RenderTargetRaw();
		// 	t.name = "taa2";
		// 	t.width = 0;
		// 	t.height = 0;
		// 	t.format = "RGBA32";
		// 	t.scale = Inc.getSuperSampling();
		// 	path.createRenderTarget(t);
		// }
		// {
		// 	var t = new RenderTargetRaw();
		// 	t.name = "taa";
		// 	t.width = 0;
		// 	t.height = 0;
		// 	t.format = "RGBA32";
		// 	t.scale = Inc.getSuperSampling();
		// 	path.createRenderTarget(t);
		// }

		// path.loadShader("shader_datas/compositor_pass/compositor_pass");
		// path.loadShader("shader_datas/copy_pass/copy_pass");
		// path.loadShader("shader_datas/supersample_resolve/supersample_resolve");

		// {
		// 	var t = new RenderTargetRaw();
		// 	t.name = "buf";
		// 	t.width = 0;
		// 	t.height = 0;
		// 	t.format = 'RGBA32';
		// 	t.scale = Inc.getSuperSampling();
		// 	t.depth_buffer = "main";
		// 	path.createRenderTarget(t);
		// }
		// {
		// 	var t = new RenderTargetRaw();
		// 	t.name = "bufa";
		// 	t.width = 0;
		// 	t.height = 0;
		// 	t.format = "RGBA32";
		// 	t.scale = Inc.getSuperSampling();
		// 	path.createRenderTarget(t);
		// }
		// {
		// 	var t = new RenderTargetRaw();
		// 	t.name = "bufb";
		// 	t.width = 0;
		// 	t.height = 0;
		// 	t.format = "RGBA32";
		// 	t.scale = Inc.getSuperSampling();
		// 	path.createRenderTarget(t);
		// }

		// path.loadShader("shader_datas/smaa_edge_detect/smaa_edge_detect");
		// path.loadShader("shader_datas/smaa_blend_weight/smaa_blend_weight");
		// path.loadShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		// path.loadShader("shader_datas/taa_pass/taa_pass");

		// #if arm_painter
		// RenderPathPaint.init(path);
		// RenderPathPreview.init(path);
		// #end

		// #if kha_direct3d12
		// RenderPathRaytrace.init(path);
		// #end
	}

	@:access(iron.RenderPath)
	public static function commands() {

		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		#if arm_painter
		Inc.beginSplit();

		#if (!arm_creator)
		if (Context.ddirty <= 0 && Context.rdirty <= 0 && (Context.pdirty <= 0 || UITrait.inst.worktab.position == SpaceScene)) {
			var mouse = Input.getMouse();
			var mx = lastX;
			var my = lastY;
			lastX = mouse.viewX;
			lastY = mouse.viewY;
			if (mx != lastX || my != lastY || mouse.locked) Context.ddirty = 0;
			if (Context.ddirty > -2) {
				path.setTarget("");
				path.bindTarget("taa", "tex");
				Inc.ssaa4() ?
					path.drawShader("shader_datas/supersample_resolve/supersample_resolve") :
					path.drawShader("shader_datas/copy_pass/copy_pass");
				if (UITrait.inst.brush3d) RenderPathPaint.commandsCursor();
				if (Context.ddirty <= 0) Context.ddirty--;
			}
			Inc.endSplit();
			return;
		}
		#end

		// Match projection matrix jitter
		var skipTaa = UITrait.inst.splitView;
		if (!skipTaa) {
			@:privateAccess Scene.active.camera.frame = RenderPathDeferred.taaFrame;
			@:privateAccess Scene.active.camera.projectionJitter();
		}
		Scene.active.camera.buildMatrix();

		RenderPathPaint.begin();
		drawSplit();

		RenderPathPaint.draw();

		#end // arm_painter

		drawForward();

		#if arm_painter
		RenderPathPaint.end();
		Inc.endSplit();
		#end

		RenderPathDeferred.taaFrame++;
	}

	static function drawForward() {
		path.setTarget("gbuffer0");
		path.clearTarget(null, 1.0);
		path.setTarget("gbuffer2");
		path.clearTarget(0xff000000);
		path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		#if arm_painter
		RenderPathPaint.bindLayers();
		#end
		path.drawMeshes("mesh");

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

		path.setTarget("bufb");
		path.clearTarget(0x00000000);
		path.bindTarget(current, "edgesTex");
		path.drawShader("shader_datas/smaa_blend_weight/smaa_blend_weight");

		path.setTarget(current);
		path.bindTarget("buf", "colorTex");
		path.bindTarget("bufb", "blendTex");
		path.bindTarget("gbuffer2", "sveloc");
		path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

		#if arm_painter
		var skipTaa = UITrait.inst.splitView;
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
		if (UITrait.inst.splitView) {
			if (Context.pdirty > 0) {
				var cam = Scene.active.camera;

				UITrait.inst.viewIndex = UITrait.inst.viewIndex == 0 ? 1 : 0;
				cam.transform.setMatrix(arm.plugin.Camera.inst.views[UITrait.inst.viewIndex]);
				cam.buildMatrix();
				cam.buildProjection();

				#if kha_direct3d12
				UITrait.inst.viewportMode == 10 ? RenderPathRaytrace.draw() : drawForward();
				#else
				drawForward();
				#end

				UITrait.inst.viewIndex = UITrait.inst.viewIndex == 0 ? 1 : 0;
				cam.transform.setMatrix(arm.plugin.Camera.inst.views[UITrait.inst.viewIndex]);
				cam.buildMatrix();
				cam.buildProjection();
			}
		}
	}
}
