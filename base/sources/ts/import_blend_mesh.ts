
function import_blend_mesh_ui() {
	if (config_raw.blender == null) {
		config_raw.blender = "";
	}

	ui_text(tr("Blender Executable"));

	let ar: f32[] = [ 7 / 8, 1 / 8];
	ui_row(ar);

	let h: ui_handle_t = ui_handle(__ID__);
	h.text = config_raw.blender;
	config_raw.blender = ui_text_input(h);

	if (ui_button("...")) {
		ui_files_show("", false, false, function (path: string) {
			///if arm_windows
			path = string_replace_all(path, "\\", "/");
			///end
			config_raw.blender = path;
			config_save();
		});
	}
}

function import_blend_mesh_run(path: string, replace_existing: bool = true) {

	if (config_raw.blender == null || config_raw.blender == "") {
		console_error(tr("Blender executable path not set"));
		return;
	}

	let save: string = "tmp.obj";
	if (path_is_protected()) {
		save = iron_internal_save_path() + save;
	}

	///if arm_windows
	path = string_replace_all(path, "\\", "\\\\");
	save = string_replace_all(save, "\\", "\\\\");
	///end

	// Have to use ; instead of \n on windows
	let py: string = "\
import bpy;\
bpy.ops.wm.obj_export(filepath='" + save + "',export_triangulated_mesh=True,export_materials=False,check_existing=False)";

	let bl: string = string_replace_all(config_raw.blender, " ", "\\ ");
	iron_sys_command(bl + " \"" + path + "\" -b --python-expr \"" + py + "\"");
	import_obj_run(save, replace_existing);
}
