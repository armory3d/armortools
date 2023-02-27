package arm.render;

import iron.RenderPath;

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
		if (Context.raw.viewportMode == ViewLit) {
			path.drawShader("shader_datas/compositor_pass/compositor_pass");
		}
		else {
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}

		path.setTarget("buf");
		RenderPathBase.drawCompass(path.currentG);
		path.drawMeshes("overlay");

		RenderPathBase.drawTAA();
	}
}
