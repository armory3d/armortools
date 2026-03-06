void import_blend_mesh_ui() {
	if (config_raw->blender == null) {
		config_raw->blender = "";
	}

	ui_text(tr("Blender Executable", null), UI_ALIGN_LEFT, 0x00000000);

	f32_array_t *ar = f32_array_create_from_raw(
	    (f32[]){
	        7 / (float)8,
	        1 / (float)8,
	    },
	    2);
	ui_row(ar);

	ui_handle_t *h      = ui_handle(__ID__);
	h->text             = string_copy(config_raw->blender);
	config_raw->blender = string_copy(ui_text_input(h, "", UI_ALIGN_LEFT, true, false));
	if (ui_icon_button("", ICON_FOLDER_OPEN, UI_ALIGN_CENTER)) {
		ui_files_show("", false, false, &import_blend_mesh_ui_37327);
	}
}

void import_blend_mesh_ui_37327(string_t *path) {
	#ifdef IRON_WINDOWS
	path                = string_copy(string_replace_all(path, "\\", "/"));
	#endif
	config_raw->blender = string_copy(path);
	config_save();
}

void import_blend_mesh_run(string_t *path, bool replace_existing) {
	if (config_raw->blender == null || string_equals(config_raw->blender, "")) {
		console_error(tr("Blender executable path not set", null));
		return;
	}

	string_t *save       = "tmp.obj";
	string_t *bpy_folder = "data/";
	if (path_is_protected()) {
		save       = string_join(iron_internal_save_path(), save);
		bpy_folder = "";
	}

	// Have to use ; instead of \n on windows
	string_t *py = string_join(string_join(string_join("\
import bpy;\
bpy.ops.wm.obj_export(filepath='",
	                                                   bpy_folder),
	                                       string_replace_all(save, "\\", "/")),
	                           "',export_triangulated_mesh=True,export_materials=False,check_existing=False)");
	#ifdef IRON_WINDOWS
	string_t *bl = string_join(string_join("\"", string_replace_all(config_raw->blender, "/", "\\")), "\"");
	#else
	string_t *bl = string_replace_all(config_raw->blender, " ", "\\ ");
	#endif
	iron_sys_command(string_join(string_join(string_join(string_join(string_join(bl, " \""), path), "\" -b --python-expr \""), py), "\""));
	import_obj_run(save, replace_existing);
}
