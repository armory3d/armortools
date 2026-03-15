
#include "global.h"

void import_blend_mesh_ui_blender_folder_picked(char *path) {
#ifdef IRON_WINDOWS
	path = string_copy(string_replace_all(path, "\\", "/"));
#endif
	config_raw->blender = string_copy(path);
	config_save();
}

void import_blend_mesh_ui() {
	if (config_raw->blender == NULL) {
		config_raw->blender = "";
	}

	ui_text(tr("Blender Executable"), UI_ALIGN_LEFT, 0x00000000);

	f32_array_t *ar = f32_array_create_from_raw(
	    (f32[]){
	        7 / 8.0,
	        1 / 8.0,
	    },
	    2);
	ui_row(ar);

	ui_handle_t *h      = ui_handle(__ID__);
	h->text             = string_copy(config_raw->blender);
	config_raw->blender = string_copy(ui_text_input(h, "", UI_ALIGN_LEFT, true, false));
	if (ui_icon_button("", ICON_FOLDER_OPEN, UI_ALIGN_CENTER)) {
		ui_files_show("", false, false, &import_blend_mesh_ui_blender_folder_picked);
	}
}

void import_blend_mesh_run(char *path, bool replace_existing) {
	if (config_raw->blender == NULL || string_equals(config_raw->blender, "")) {
		console_error(tr("Blender executable path not set"));
		return;
	}

	char *save       = "tmp.obj";
	char *bpy_folder = "data/";
	if (path_is_protected()) {
		save       = string("%s%s", iron_internal_save_path(), save);
		bpy_folder = "";
	}

	// Have to use ; instead of \n on windows
	char *py = string("import bpy;bpy.ops.wm.obj_export(filepath='%s%s',export_triangulated_mesh=True,export_materials=False,check_existing=False)", bpy_folder,
	                  string_replace_all(save, "\\", "/"));
#ifdef IRON_WINDOWS
	char *bl = string("\"%s\"", string_replace_all(config_raw->blender, "/", "\\"));
#else
	char *bl = string_replace_all(config_raw->blender, " ", "\\ ");
#endif
	iron_sys_command(string("%s \"%s\" -b --python-expr \"%s\"", bl, path, py));
	import_obj_run(save, replace_existing);
}
