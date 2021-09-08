package arm.render;

import kha.System;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UISidebar;
import arm.node.MakeMesh;
import arm.Enums;

class RenderPathForward {

	public static var path: RenderPath;

	public static function init(_path: RenderPath) {
		path = _path;
	}

	@:access(iron.RenderPath)
	public static function commands() {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		Inc.beginSplit();
		if (Inc.isCached()) return;

		// Match projection matrix jitter
		var skipTaa = Context.splitView || ((Context.tool == ToolClone || Context.tool == ToolBlur) && Context.pdirty > 0);
		@:privateAccess Scene.active.camera.frame = skipTaa ? 0 : RenderPathDeferred.taaFrame;
		@:privateAccess Scene.active.camera.projectionJitter();
		Scene.active.camera.buildMatrix();

		RenderPathPaint.begin();
		drawSplit();
		drawGbuffer();
		RenderPathPaint.draw();

		#if (kha_direct3d12 || kha_vulkan)
		if (Context.viewportMode == ViewPathTrace) {
			var useLiveLayer = arm.ui.UIHeader.inst.worktab.position == SpaceMaterial;
			RenderPathRaytrace.draw(useLiveLayer);
			return;
		}
		#end

		drawForward();
		RenderPathPaint.end();
		Inc.end();
		RenderPathDeferred.taaFrame++;
	}

	public static function drawGbuffer(gbuffer0 = "gbuffer0", gbuffer1 = "gbuffer1", gbuffer2 = "gbuffer2") {
		path.setTarget(gbuffer0);
		#if kha_metal
		path.clearTarget(0x00000000, 1.0);
		#else
		path.clearTarget(null, 1.0);
		#end
		path.setTarget(gbuffer2);
		path.clearTarget(0xff000000);
		path.setTarget(gbuffer0, [gbuffer1, gbuffer2]);
		var currentG = path.currentG;
		RenderPathPaint.bindLayers();
		path.drawMeshes("mesh");
		RenderPathPaint.unbindLayers();
		if (MakeMesh.layerPassCount > 1) {
			RenderPathDeferred.makeGbufferCopyTextures();
			for (i in 1...MakeMesh.layerPassCount) {
				var ping = i % 2 == 1 ? "_copy" : "";
				var pong = i % 2 == 1 ? "" : "_copy";
				path.setTarget("gbuffer0" + ping, ["gbuffer1" + ping, "gbuffer2" + ping]);
				path.bindTarget("gbuffer0" + pong, "gbuffer0");
				path.bindTarget("gbuffer1" + pong, "gbuffer1");
				path.bindTarget("gbuffer2" + pong, "gbuffer2");
				RenderPathPaint.bindLayers();
				path.drawMeshes("mesh" + i);
				RenderPathPaint.unbindLayers();
			}
			if (MakeMesh.layerPassCount % 2 == 0) {
				RenderPathDeferred.copyToGbuffer();
			}
		}
		LineDraw.render(currentG);
	}

	public static function drawForward(eye = false, output = "", gbuffer0 = "gbuffer0", gbuffer1 = "gbuffer1", gbuffer2 = "gbuffer2", buf = "buf", bufa = "bufa", taa = "taa", taa2 = "taa2") {
		path.setDepthFrom(gbuffer1, gbuffer0);
		path.setTarget(gbuffer1);
		path.drawSkydome("shader_datas/world_pass/world_pass");
		path.setDepthFrom(gbuffer1, gbuffer2);

		path.setTarget(buf);
		path.bindTarget(gbuffer1, "tex");
		if (Context.viewportMode == ViewLit) {
			path.drawShader("shader_datas/compositor_pass/compositor_pass");
		}
		else {
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}

		if (output == "") {
			path.setTarget(buf);
			var currentG = path.currentG;
			path.drawMeshes("overlay");
			Inc.drawCompass(currentG);
		}

		var taaFrame = RenderPathDeferred.taaFrame;
		var current = taaFrame % 2 == 0 ? bufa : taa2;
		var last = taaFrame % 2 == 0 ? taa2 : bufa;

		path.setTarget(current);
		path.clearTarget(0x00000000);
		path.bindTarget(buf, "colorTex");
		path.drawShader("shader_datas/smaa_edge_detect/smaa_edge_detect");

		path.setTarget(taa);
		path.clearTarget(0x00000000);
		path.bindTarget(current, "edgesTex");
		path.drawShader("shader_datas/smaa_blend_weight/smaa_blend_weight");

		path.setTarget(current);
		path.bindTarget(buf, "colorTex");
		path.bindTarget(taa, "blendTex");
		path.bindTarget(gbuffer2, "sveloc");
		path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

		var skipTaa = Context.splitView || eye;
		if (skipTaa) {
			path.setTarget(taa);
			path.bindTarget(current, "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		else {
			path.setTarget(taa);
			path.bindTarget(current, "tex");
			path.bindTarget(last, "tex2");
			path.bindTarget(gbuffer2, "sveloc");
			path.drawShader("shader_datas/taa_pass/taa_pass");
		}

		if (!Inc.ssaa4()) {
			path.setTarget(output);
			path.bindTarget(taaFrame % 2 == 0 ? current : taa, "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}

		if (Inc.ssaa4()) {
			path.setTarget(output);
			path.bindTarget(taaFrame % 2 == 0 ? taa2 : taa, "tex");
			path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
		}
	}

	static function drawSplit() {
		if (Context.splitView && !Context.paint2dView) {
			#if (kha_metal || krom_android)
			Context.ddirty = 2;
			#else
			Context.ddirty = 1;
			#end
			var cam = Scene.active.camera;

			Context.viewIndex = Context.viewIndex == 0 ? 1 : 0;
			cam.transform.setMatrix(arm.Camera.inst.views[Context.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();

			drawGbuffer();

			#if (kha_direct3d12 || kha_vulkan)
			var useLiveLayer = arm.ui.UIHeader.inst.worktab.position == SpaceMaterial;
			Context.viewportMode == ViewPathTrace ? RenderPathRaytrace.draw(useLiveLayer) : drawForward();
			#else
			drawForward();
			#end

			Context.viewIndex = Context.viewIndex == 0 ? 1 : 0;
			cam.transform.setMatrix(arm.Camera.inst.views[Context.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();
		}
	}
}
