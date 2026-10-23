
#include "../global.h"

char *_project_import_mesh_box_path;
bool  _project_import_mesh_box_replace_existing;
bool  _project_import_mesh_box_clear_layers;
bool  _project_import_mesh_box_keep_camera;
void (*_project_import_mesh_box_done)(void);

extern int plugins_skinning_frame;
extern int plugins_split_by;
extern bool io_fbx_material_uvs;

static i32 import_axis_mode    = 0;
static i32 import_axis_up      = 2; // +Y
static i32 import_axis_forward = 4; // +Z

static void project_import_mesh_box_run(bool replace_existing) {
	import_mesh_run_with_axes(_project_import_mesh_box_path, _project_import_mesh_box_clear_layers, replace_existing, _project_import_mesh_box_keep_camera,
	                          import_axis_mode == 0 ? -1 : import_axis_up, import_axis_mode == 0 ? -1 : import_axis_forward);
}

void project_import_mesh_box_menu() {
	if (ui_menu_button(tr("Append"), "", ICON_CHECK)) {
		ui_box_hide();
		i32 first = g_project->_->paint_objects->length;
		project_import_mesh_box_run(false);
		if (g_project->_->paint_objects->length > first) {
			import_mesh_finish_import(NULL);
		}
	}
	if (ui_menu_button(tr("Help"), "", ICON_HELP)) {
		iron_load_url("https://github.com/armory3d/armorpaint_web/blob/main/manual.md#faq");
	}
}

void project_import_mesh_box_draw() {
	char *path             = _project_import_mesh_box_path;
	bool  replace_existing = _project_import_mesh_box_replace_existing;
	void (*done)(void)     = _project_import_mesh_box_done;
	char *lower            = to_lower_case(path);
	bool  is_obj           = ends_with(lower, ".obj");
	bool  is_fbx           = ends_with(lower, ".fbx");
	bool  is_blend         = ends_with(lower, ".blend");
	bool  is_gltf          = ends_with(lower, ".gltf") || ends_with(lower, ".glb");
	free(lower);

	if (is_obj || is_fbx) {
		string_array_t *split_by_combo = any_array_create_from_raw_tmp(
		    (void *[]){
		        tr("Object"),
		        tr("Material"),
		        tr("UDIM Tile"),
		    },
		    3);
		ui_text(tr("Split By"), UI_ALIGN_LEFT, 0);
		ui_inline_radio((int *)&g_context->split_by, split_by_combo, UI_ALIGN_LEFT);
		plugins_split_by = g_context->split_by;
		if (g_ui->is_hovered) {
			ui_tooltip(tr("Split mesh into objects"));
		}
	}

	if (is_blend) {
		import_blend_mesh_ui();
	}
	if (is_fbx && g_context->split_by == SPLIT_TYPE_OBJECT) {
		ui_check(&io_fbx_material_uvs, tr("Separate Material UVs"), "");
		if (g_ui->is_hovered)
			ui_tooltip(tr("Keep material UVs separate in the paint texture and preserve source objects and LODs"));
	}

	if (is_fbx || is_gltf) {
		static bool skinning = false;
		static f32  frame    = 0.0;
		ui_row2();
		ui_check(&skinning, tr("Apply Skinning"), "");
		g_ui->enabled          = skinning;
		plugins_skinning_frame = ui_slider(&frame, tr("Frame"), 1, 99, false, 1, true, UI_ALIGN_RIGHT, true);
		g_ui->enabled          = true;
		if (!skinning) {
			plugins_skinning_frame = -1;
		}
	}

	ui_text(tr("Orientation"), UI_ALIGN_LEFT, 0);
	string_array_t *modes = any_array_create_from_raw_tmp((void *[]){tr("Automatic"), tr("Manual")}, 2);
	ui_inline_radio(&import_axis_mode, modes, UI_ALIGN_LEFT);
	if (g_ui->is_hovered) {
		ui_tooltip(tr("Automatic uses file axis information when available, otherwise the format's default orientation"));
	}
	if (import_axis_mode == 1) {
		char           *labels[] = {"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
		string_array_t *axes     = any_array_create_from_raw_tmp((void **)labels, 6);
		ui_row2();
		ui_combo(&import_axis_up, axes, tr("Up Axis"), true, UI_ALIGN_LEFT, true);
		if (import_axis_up / 2 == import_axis_forward / 2)
			import_axis_forward = (import_axis_up + 2) % 6;
		// Only perpendicular axes are selectable; parallel/opposite pairs are invalid.
		char *forward_labels[4];
		i32   forward_axes[4], selected = 0, count = 0;
		for (i32 i = 0; i < 6; ++i) {
			if (i / 2 == import_axis_up / 2)
				continue;
			if (i == import_axis_forward)
				selected = count;
			forward_labels[count] = labels[i];
			forward_axes[count++] = i;
		}
		string_array_t *forward_choices = any_array_create_from_raw_tmp((void **)forward_labels, 4);
		ui_set_next_id((ui_id_t)&import_axis_forward);
		ui_combo(&selected, forward_choices, tr("Forward Axis"), true, UI_ALIGN_LEFT, true);
		import_axis_forward = forward_axes[selected];
		ui_text(tr("Select the source file's axes"), UI_ALIGN_LEFT, 0);
		if (g_ui->is_hovered)
			ui_tooltip(tr("Converted to Z up and -Y forward. All imported parts are oriented together."));
	}

	f32_array_t *row = f32_array_create_from_raw_tmp(
	    (f32[]){
	        0.45,
	        0.45,
	        0.1,
	    },
	    3);

	ui_end_element();
	ui_row(row);
	if (ui_icon_button(tr("Cancel"), ICON_CLOSE, UI_ALIGN_CENTER) && !ui_menu_show) {
		ui_box_hide();
	}
	if ((ui_icon_button(tr("Import"), ICON_CHECK, UI_ALIGN_CENTER) || g_ui->is_return_down) && !ui_menu_show) {
		ui_box_hide();

#if defined(IRON_ANDROID) || defined(IRON_IOS)
		console_toast(tr("Importing mesh"));
#endif

		project_import_mesh_box_run(replace_existing);
		if (done != NULL) {
			done();
		}
	}
	if (ui_icon_button("", ICON_CHEVRON_DOWN, UI_ALIGN_CENTER)) {
		ui_menu_draw(&project_import_mesh_box_menu, -1, -1);
	}
}

void project_import_mesh_box(char *path, bool replace_existing, bool clear_layers, bool keep_camera, void (*done)(void)) {
	_project_import_mesh_box_path             = string_copy(path);
	_project_import_mesh_box_replace_existing = replace_existing;
	_project_import_mesh_box_clear_layers     = clear_layers;
	_project_import_mesh_box_keep_camera      = keep_camera;
	_project_import_mesh_box_done             = done;
	ui_box_show_custom(&project_import_mesh_box_draw, 460, 360, NULL, true, tr("Import Mesh"));
	ui_box_click_to_hide = false; // Prevent closing when going back to window from file browser
}
