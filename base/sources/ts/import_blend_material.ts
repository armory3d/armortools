
let _import_blend_material_path: string;

function import_blend_material_run(path: string) {

	_import_blend_material_path = path;

	ui_box_show_custom(function (ui: ui_t) {
		if (ui_tab(ui_handle(__ID__), tr("Import Material"))) {
			import_blend_mesh_ui();

			ui_row2();
			if (ui_button(tr("Cancel"))) {
				ui_box_hide();
			}
			if (ui_button(tr("Import")) || ui.is_return_down) {

				ui_box_hide();

				if (config_raw.blender == null || config_raw.blender == "") {
					console_error(tr("Blender executable path not set"));
					return;
				}

				_import_blend_material();
			}
		}
	});
}

function _import_blend_material() {

	let current: iron_gpu_texture_t = _draw_current;
	let g2_in_use: bool = _draw_in_use;
	if (g2_in_use) draw_end();

	console_toast(tr("Baking material"));

	if (g2_in_use) draw_begin(current);

	app_notify_on_init(function () {

		let save: string;
		if (path_is_protected()) {
			save = iron_internal_save_path();
		}
		else {
			save = path_data();
		}
		save += "blender";
		file_create_directory(save);

		let py: string = "\
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
    img.filepath_raw = '" + save + "/blender_base.jpg'\n\
    img.save()\n\
    bpy.ops.object.bake(type='ROUGHNESS')\n\
    img.filepath_raw = '" + save + "/blender_rough.jpg'\n\
    img.save()\n\
    bpy.ops.object.bake(type='NORMAL')\n\
    img.filepath_raw = '" + save + "/blender_nor.jpg'\n\
    img.save()\n\
    break\n\
";

		iron_sys_command("\"" + config_raw.blender + "\" \"" + _import_blend_material_path + "\" -b --python-expr \"" + py + "\"");
		import_folder_run(save);
	});
}
