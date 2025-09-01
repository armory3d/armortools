
function export_mesh_run(path: string, paint_objects: mesh_object_t[] = null, apply_disp: bool = false, merge_vertices: bool = true) {
	if (paint_objects == null) {
		paint_objects = project_paint_objects;
	}
	if (context_raw.export_mesh_format == mesh_format_t.OBJ) {
		merge_vertices ?
			export_obj_run(path, paint_objects, apply_disp) :
			export_obj_run_fast(path, paint_objects);
	}
	else {
		export_arm_run_mesh(path, paint_objects);
	}
}
