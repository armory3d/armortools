
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
		render_path_set_depth_from("gbuffer1", "gbuffer0");
		render_path_set_target("gbuffer1");
		render_path_draw_skydome("shader_datas/world_pass/world_pass");
		render_path_set_depth_from("gbuffer1", "gbuffer2");

		render_path_set_target("buf");
		render_path_bind_target("gbuffer1", "tex");
		render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");

		render_path_set_target("buf");
		RenderPathBase.drawCompass();
		render_path_draw_meshes("overlay");

		RenderPathBase.drawTAA();
	}
}
