
class RenderPathForward {

	static init = () => {
	}

	static commands = () => {
		///if is_paint
		RenderPathPaint.live_brush_dirty();
		///end
		RenderPathBase.commands(RenderPathForward.draw_forward);
	}

	static draw_forward = () => {
		render_path_set_depth_from("gbuffer1", "gbuffer0");
		render_path_set_target("gbuffer1");
		render_path_draw_skydome("shader_datas/world_pass/world_pass");
		render_path_set_depth_from("gbuffer1", "gbuffer2");

		render_path_set_target("buf");
		render_path_bind_target("gbuffer1", "tex");
		render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");

		render_path_set_target("buf");
		RenderPathBase.draw_compass();
		render_path_draw_meshes("overlay");

		RenderPathBase.draw_taa();
	}
}
