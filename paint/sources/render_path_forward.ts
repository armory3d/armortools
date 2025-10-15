
function render_path_forward_init() {}

function render_path_forward_commands() {
	render_path_paint_live_brush_dirty();
	render_path_base_commands(render_path_forward_draw_forward);
}

function render_path_forward_draw_forward() {
	render_path_set_target("gbuffer1", null, "main");
	render_path_draw_skydome("Scene/world_pass/world_pass");

	render_path_set_target("buf");
	render_path_bind_target("gbuffer1", "tex");
	render_path_draw_shader("Scene/compositor_pass/compositor_pass");

	render_path_set_target("buf");
	render_path_base_draw_compass();
	render_path_draw_meshes("overlay");

	render_path_base_draw_taa("buf", "gbuffer1");
}
