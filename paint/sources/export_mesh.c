
#include "global.h"

void export_mesh_run(char *path, mesh_object_t_array_t *paint_objects, bool apply_disp, bool merge_vertices) {
	if (paint_objects == NULL) {
		paint_objects = project_paint_objects;
	}
	if (g_context->export_mesh_format == MESH_FORMAT_OBJ) {
		merge_vertices ? export_obj_run(path, paint_objects, apply_disp) : export_obj_run_fast(path, paint_objects);
	}
	else {
		export_arm_run_mesh(path, paint_objects);
	}
}
