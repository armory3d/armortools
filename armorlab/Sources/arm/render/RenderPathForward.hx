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

		if (Inc.isCached()) return;

		// Match projection matrix jitter
		var skipTaa = (Context.tool == ToolClone || Context.tool == ToolBlur) && Context.pdirty > 0;
		@:privateAccess Scene.active.camera.frame = skipTaa ? 0 : RenderPathDeferred.taaFrame;
		@:privateAccess Scene.active.camera.projectionJitter();
		Scene.active.camera.buildMatrix();

		RenderPathPaint.begin();
		RenderPathDeferred.drawGbuffer();
		RenderPathPaint.draw();

		#if (kha_direct3d12 || kha_vulkan)
		if (Context.viewportMode == ViewPathTrace) {
			RenderPathRaytrace.draw(false);
			return;
		}
		#end

		drawForward();
		RenderPathPaint.end();
		Inc.end();
		RenderPathDeferred.taaFrame++;
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

		var skipTaa = eye;
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

		path.setTarget(output);
		path.bindTarget(taaFrame % 2 == 0 ? current : taa, "tex");
		path.drawShader("shader_datas/copy_pass/copy_pass");
	}
}
