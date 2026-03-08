void import_blend_material_run(char *path) {
	gc_unroot(_import_blend_material_path);
	_import_blend_material_path = string_copy(path);
	gc_root(_import_blend_material_path);

	ui_box_show_custom(&import_blend_material_run_43663, 400, 200, NULL, true, "");
}

void import_blend_material_run_43663() {
	if (ui_tab(ui_handle(__ID__), tr("Import Material", NULL), false, -1, false)) {
		import_blend_mesh_ui();

		ui_row2();
		if (ui_icon_button(tr("Cancel", NULL), ICON_CLOSE, UI_ALIGN_CENTER)) {
			ui_box_hide();
		}
		if (ui_icon_button(tr("Import", NULL), ICON_CHECK, UI_ALIGN_CENTER) || ui->is_return_down) {

			ui_box_hide();

			if (config_raw->blender == NULL || string_equals(config_raw->blender, "")) {
				console_error(tr("Blender executable path not set", NULL));
				return;
			}
			_import_blend_material();
		}
	}
}

void _import_blend_material() {
	console_toast(tr("Baking material", NULL));

	sys_notify_on_next_frame(&_import_blend_material_43775, NULL);
}

void _import_blend_material_43775(void * _) {
	char *save;
	if (path_is_protected()) {
		save = string_copy(iron_internal_save_path());
	}
	else {
		save = string_copy(path_data());
	}
	save = string_join(save, "blender");
	iron_create_directory(save);

	char *py = string_join(string_join(string_join(string_join(string_join(string_join("\
import bpy;\n\
bpy.context.scene.render.engine = 'CYCLES'\n\
bpy.context.scene.cycles.samples = 4\n\
bpy.ops.mesh.primitive_plane_add()\n\
plane = bpy.context.selected_objects[0]\n\
img = bpy.data.images.new('bake', 2048, 2048)\n\
img.file_format = 'JPEG'\n\
for mat in bpy.data.materials:\n\
    if mat.name == 'Dots Stroke':\n\
        continue\n\
    plane.data.materials.append(mat)\n\
    node = mat.node_tree.nodes.new(type='ShaderNodeTexImage')\n\
    node.image = img\n\
    mat.node_tree.nodes.active = node\n\
    bpy.ops.object.bake(type='DIFFUSE')\n\
    img.filepath_raw = '",
	                                                                                       save),
	                                                                           "/blender_base.jpg'\n\
    img.save()\n\
    bpy.ops.object.bake(type='ROUGHNESS')\n\
    img.filepath_raw = '"),
	                                                               save),
	                                                   "/blender_rough.jpg'\n\
    img.save()\n\
    bpy.ops.object.bake(type='NORMAL')\n\
    img.filepath_raw = '"),
	                                       save),
	                           "/blender_nor.jpg'\n\
    img.save()\n\
    break\n\
");

	iron_sys_command(string_join(string_join(string_join(string_join(string_join(string_join("\"", config_raw->blender), "\" \""), _import_blend_material_path),
	                                                     "\" -b --python-expr \""),
	                                         py),
	                             "\""));
	import_folder_run(save);
}
