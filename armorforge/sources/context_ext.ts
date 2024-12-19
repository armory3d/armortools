
function context_ext_init(c: context_t) {
    c.tool = workspace_tool_t.GIZMO;
    c.brush_radius = 1.0;
}

function context_ext_select_paint_object(o: mesh_object_t) {
    for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		p.skip_context = "paint";
	}
    context_raw.paint_object.skip_context = "";
    context_raw.paint_object = o;
}
