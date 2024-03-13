
function export_mesh_run(path: string, paint_objects: mesh_object_t[] = null, apply_disp: bool = false) {
	if (paint_objects == null) {
		paint_objects = project_paint_objects;
	}
	if (context_raw.export_mesh_format == mesh_format_t.OBJ) {
		export_obj_run(path, paint_objects, apply_disp);
	}
	else {
		export_arm_run_mesh(path, paint_objects);
	}
}
