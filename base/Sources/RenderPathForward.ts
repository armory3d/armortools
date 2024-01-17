
class RenderPathForward {

	static path: RenderPath;

	static init = (_path: RenderPath) => {
		RenderPathForward.path = _path;
	}

	static commands = () => {
		///if is_paint
		RenderPathPaint.liveBrushDirty();
		///end
		RenderPathBase.commands(RenderPathForward.drawForward);
	}

	static drawForward = () => {
		RenderPathForward.path.setDepthFrom("gbuffer1", "gbuffer0");
		RenderPathForward.path.setTarget("gbuffer1");
		RenderPathForward.path.drawSkydome("shader_datas/world_pass/world_pass");
		RenderPathForward.path.setDepthFrom("gbuffer1", "gbuffer2");

		RenderPathForward.path.setTarget("buf");
		RenderPathForward.path.bindTarget("gbuffer1", "tex");
		RenderPathForward.path.drawShader("shader_datas/compositor_pass/compositor_pass");

		RenderPathForward.path.setTarget("buf");
		RenderPathBase.drawCompass(RenderPathForward.path.currentG);
		RenderPathForward.path.drawMeshes("overlay");

		RenderPathBase.drawTAA();
	}
}
