package arm.render;

import iron.RenderPath;
import iron.Scene;
import arm.Enums;

class RenderPathForward {

	public static var path: RenderPath;

	public static function init(_path: RenderPath) {
		path = _path;
	}

	public static function commands() {
		RenderPathBase.commands(drawForward);
	}

	public static function drawForward() {
		path.setDepthFrom("gbuffer1", "gbuffer0");
		path.setTarget("gbuffer1");
		path.drawSkydome("shader_datas/world_pass/world_pass");
		path.setDepthFrom("gbuffer1", "gbuffer2");

		path.setTarget("buf");
		path.bindTarget("gbuffer1", "tex");
		if (Context.viewportMode == ViewLit) {
			path.drawShader("shader_datas/compositor_pass/compositor_pass");
		}
		else {
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}

		path.setTarget("buf");
		var currentG = path.currentG;
		path.drawMeshes("overlay");
		RenderPathBase.drawCompass(currentG);

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

		var skipTaa = Context.splitView;
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

		if (RenderPathBase.ssaa4()) {
			path.setTarget("");
			path.bindTarget(taaFrame % 2 == 0 ? "taa2" : "taa", "tex");
			path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
		}
		else {
			path.setTarget("");
			path.bindTarget(taaFrame % 2 == 0 ? current : "taa", "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
	}
}
