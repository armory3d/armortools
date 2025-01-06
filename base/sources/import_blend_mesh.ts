
function import_blend_mesh_ui() {
	if (config_raw.blender == null) {
		config_raw.blender = "";
	}

	let ar: f32[] = [ 7 / 8, 1 / 8];
	ui_row(ar);

	let h: ui_handle_t = ui_handle(__ID__);
	h.text = config_raw.blender;
	config_raw.blender = ui_text_input(h, tr("Blender Executable"));

	if (ui_button("...")) {
		ui_files_show("", false, false, function (path: string) {
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

	let save: string;
	if (path_is_protected()) {
		save = iron_save_path();
	}
	else {
		save = path_data();
	}
	save += "tmp.obj";

	let py: string = "\
import bpy\n\
bpy.ops.wm.obj_export(filepath='" + save + "',export_triangulated_mesh=True,export_materials=False,check_existing=False)";

	iron_sys_command("\"" + config_raw.blender + "\" \"" + path + "\" -b --python-expr \"" + py + "\"");
	import_obj_run(save, replace_existing);
}
