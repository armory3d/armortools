
function context_ext_init(c: context_t) {
    c.tool = workspace_tool_t.BRUSH;
    c.color_picker_previous_tool = workspace_tool_t.BRUSH;
    c.brush_radius = 0.5;
    c.brush_radius_handle.value = 0.5;
	c.brush_hardness = 0.8;
	///if is_sculpt
	c.brush_hardness = 0.05;
	///end
}

function context_ext_select_paint_object(o: mesh_object_t) {
	ui_header_handle.redraws = 2;
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		p.skip_context = "paint";
	}
	context_raw.paint_object = o;

	let mask: i32 = slot_layer_get_object_mask(context_raw.layer);
	if (context_layer_filter_used()) {
		mask = context_raw.layer_filter;
	}

	if (context_raw.merged_object == null || mask > 0) {
		context_raw.paint_object.skip_context = "";
	}
	util_uv_uvmap_cached = false;
	util_uv_trianglemap_cached = false;
	util_uv_dilatemap_cached = false;
}
