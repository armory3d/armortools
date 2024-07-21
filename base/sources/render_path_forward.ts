
function render_path_forward_init() {
}

function render_path_forward_commands() {
	///if is_paint
	render_path_paint_live_brush_dirty();
	///end
	render_path_base_commands(render_path_forward_draw_forward);
}

function render_path_forward_draw_forward() {
	render_path_set_depth_from("gbuffer1", "gbuffer0");
	render_path_set_target("gbuffer1");
	render_path_draw_skydome("shader_datas/world_pass/world_pass");
	render_path_set_depth_from("gbuffer1", "gbuffer2");

	render_path_set_target("buf");
	render_path_bind_target("gbuffer1", "tex");
	render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");

	render_path_set_target("buf");
	render_path_base_draw_compass();
	render_path_draw_meshes("overlay");

	render_path_base_draw_taa();
}
