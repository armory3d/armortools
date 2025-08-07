
function context_ext_init(c: context_t) {
    c.tool = workspace_tool_t.ERASER;
	c.color_picker_previous_tool = workspace_tool_t.ERASER;
	c.brush_radius = 0.25;
	c.brush_radius_handle.value = 0.25;
	c.brush_hardness = 0.8;

	c.coords = vec4_create();
	c.start_x = 0.0;
	c.start_y = 0.0;

	c.lock_begin = false;
	c.lock_x = false;
	c.lock_y = false;
	c.lock_start_x = 0.0;
	c.lock_start_y = 0.0;
	c.registered = false;
}

function context_ext_select_paint_object(o: mesh_object_t) {
	context_raw.paint_object = o;
}
