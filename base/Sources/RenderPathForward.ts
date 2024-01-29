
class RenderPathForward {

	static init = () => {
	}

	static commands = () => {
		///if is_paint
		RenderPathPaint.liveBrushDirty();
		///end
		RenderPathBase.commands(RenderPathForward.drawForward);
	}

	static drawForward = () => {
		RenderPath.setDepthFrom("gbuffer1", "gbuffer0");
		RenderPath.setTarget("gbuffer1");
		RenderPath.drawSkydome("shader_datas/world_pass/world_pass");
		RenderPath.setDepthFrom("gbuffer1", "gbuffer2");

		RenderPath.setTarget("buf");
		RenderPath.bindTarget("gbuffer1", "tex");
		RenderPath.drawShader("shader_datas/compositor_pass/compositor_pass");

		RenderPath.setTarget("buf");
		RenderPathBase.drawCompass(RenderPath.currentG);
		RenderPath.drawMeshes("overlay");

		RenderPathBase.drawTAA();
	}
}
