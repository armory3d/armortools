
let ui_menubar_default_w: i32 = 330;
let ui_menubar_workspace_handle: ui_handle_t = ui_handle_create();
let ui_menubar_menu_handle: ui_handle_t = ui_handle_create();
let ui_menubar_w: i32 = ui_menubar_default_w;

let _ui_menubar_saved_camera: mat4_t = mat4_nan();
let _ui_menubar_plane: mesh_object_t = null;

function ui_menubar_init() {
	ui_menubar_workspace_handle.layout = ui_layout_t.HORIZONTAL;
	ui_menubar_menu_handle.layout = ui_layout_t.HORIZONTAL;
}

function ui_menubar_render_ui() {
	let ui: ui_t = ui_base_ui;

	let item_w: i32 = ui_toolbar_get_w();
	let panel_x: i32 = app_x();

	///if (is_paint || is_sculpt)
	if (config_raw.layout[layout_size_t.HEADER] == 1) {
		panel_x = app_x() - item_w;
	}
	///end

	if (ui_window(ui_menubar_menu_handle, panel_x, 0, ui_menubar_w, ui_header_h)) {
		ui._x += 1; // Prevent "File" button highlight on startup

		ui_begin_menu();

		if (config_raw.touch_ui) {

			///if (is_paint || is_sculpt)
			ui._w = item_w;
			///end
			///if is_lab
			ui._w = 36;
			///end

			if (ui_menubar_icon_button(ui, 0, 2)) box_preferences_show();
			if (ui_menubar_icon_button(ui, 0, 3)) {
				///if (arm_android || arm_ios)
				console_toast(tr("Saving project"));
				project_save();
				///end
				app_notify_on_next_frame(function () {
					box_projects_show();
				});
			}
			if (ui_menubar_icon_button(ui, 4, 2)) {
				project_import_asset();
			}
			///if (is_paint || is_lab)
			if (ui_menubar_icon_button(ui, 5, 2)) {
				box_export_show_textures();
			}
			///end
			let size: i32 = math_floor(ui._w / ui_SCALE(ui));
			if (ui_menu_show && ui_menu_category == menu_category_t.VIEWPORT) {
				ui_fill(0, -6, size, size - 4, ui.ops.theme.HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(ui, 8, 2)) {
				ui_menubar_show_menu(ui, menu_category_t.VIEWPORT);
			}
			if (ui_menu_show && ui_menu_category == menu_category_t.MODE) {
				ui_fill(0, -6, size, size - 4, ui.ops.theme.HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(ui, 9, 2)) {
				ui_menubar_show_menu(ui, menu_category_t.MODE);
			}
			if (ui_menu_show && ui_menu_category == menu_category_t.CAMERA) {
				ui_fill(0, -6, size, size - 4, ui.ops.theme.HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(ui, 10, 2)) {
				ui_menubar_show_menu(ui, menu_category_t.CAMERA);
			}
			if (ui_menu_show && ui_menu_category == menu_category_t.HELP) {
				ui_fill(0, -6, size, size - 4, ui.ops.theme.HIGHLIGHT_COL);
			}
			if (ui_menubar_icon_button(ui, 11, 2)) {
				ui_menubar_show_menu(ui, menu_category_t.HELP);
			}
			ui.enabled = history_undos > 0;
			if (ui_menubar_icon_button(ui, 6, 2)) {
				history_undo();
			}
			ui.enabled = history_redos > 0;
			if (ui_menubar_icon_button(ui, 7, 2)) {
				history_redo();
			}
			ui.enabled = true;
		}
		else {
			let categories: string[] = [tr("File"), tr("Edit"), tr("Viewport"), tr("Mode"), tr("Camera"), tr("Help")];
			for (let i: i32 = 0; i < categories.length; ++i) {
				if (_ui_menu_button(categories[i]) || (ui_menu_show && ui_menu_commands == null && ui.is_hovered)) {
					ui_menubar_show_menu(ui, i);
				}
			}
		}

		if (ui_menubar_w < ui._x + 10) {
			ui_menubar_w = math_floor(ui._x + 10);

			///if (is_paint || is_sculpt)
			ui_toolbar_handle.redraws = 2;
			///end
		}

		ui_end_menu();
	}

	if (config_raw.layout[layout_size_t.HEADER] == 1) {
		// Non-floating header
		ui_menubar_draw_tab_header();
	}
}

function ui_menubar_draw_tab_header() {
	let ui: ui_t = ui_base_ui;

	let item_w: i32 = ui_toolbar_get_w();
	let panel_x: i32 = app_x();

	let nodesw: i32 = (ui_nodes_show || ui_view2d_show) ? config_raw.layout[layout_size_t.NODES_W] : 0;
	///if (is_paint || is_sculpt)
	let ww: i32 = sys_width() - config_raw.layout[layout_size_t.SIDEBAR_W] - ui_menubar_w - nodesw;
	panel_x = (app_x() - item_w) + ui_menubar_w;
	///else
	let ww: i32 = sys_width() - ui_menubar_w - nodesw;
	panel_x = (app_x()) + ui_menubar_w;
	///end

	if (ui_window(ui_menubar_workspace_handle, panel_x, 0, ww, ui_header_h)) {

		if (!config_raw.touch_ui) {
			ui_tab(ui_header_worktab, tr("3D View"));
		}
		else {
			ui_fill(0, 0, ui._window_w, ui._window_h + 4, ui.ops.theme.SEPARATOR_COL);
		}

		///if is_lab
		ui_tab(ui_header_worktab, tr("2D View"));
		if (ui_header_worktab.changed) {
			context_raw.ddirty = 2;
			context_raw.brush_blend_dirty = true;
			ui_header_handle.redraws = 2;
			context_main_object().skip_context = null;

			if (ui_header_worktab.position == space_type_t.SPACE3D) {
				if (!mat4_isnan(_ui_menubar_saved_camera)) {
					transform_set_matrix(scene_camera.base.transform, _ui_menubar_saved_camera);
					_ui_menubar_saved_camera = mat4_nan();
				}
				scene_meshes = [context_main_object()];
			}
			else { // Space2D
				if (_ui_menubar_plane == null) {
					let mesh: raw_mesh_t = geom_make_plane(1, 1, 2, 2);
					let raw: mesh_data_t = {
						name: "2DView",
						vertex_arrays: [
							{
								values: mesh.posa,
								attrib: "pos",
								data: "short4norm"
							},
							{
								values: mesh.nora,
								attrib: "nor",
								data: "short2norm"
							},
							{
								values: mesh.texa,
								attrib: "tex",
								data: "short2norm"
							}
						],
						index_arrays: [
							{
								values: mesh.inda,
								material: 0
							}
						],
						scale_pos: mesh.scale_pos,
						scale_tex: mesh.scale_tex
					};
					let md: mesh_data_t = mesh_data_create(raw);
					let dot_plane: mesh_object_t = scene_get_child(".Plane").ext;
					_ui_menubar_plane = mesh_object_create(md, dot_plane.materials);
					array_remove(scene_meshes, _ui_menubar_plane);
				}

				if (mat4_isnan(_ui_menubar_saved_camera)) {
					_ui_menubar_saved_camera = mat4_clone(scene_camera.base.transform.local);
				}
				scene_meshes = [_ui_menubar_plane];
				let m: mat4_t = mat4_identity();
				m = mat4_translate(m, 0, 0, 1.6);
				transform_set_matrix(scene_camera.base.transform, m);
			}
			render_path_raytrace_ready = false;
		}
		///end
	}
}

function ui_menubar_show_menu(ui: ui_t, category: i32) {
	if (ui_menu_show && ui_menu_category == category) {
		return;
	}

	ui_menu_show = true;
	ui_menu_show_first = true;
	ui_menu_commands = null;
	ui_menu_category = category;
	ui_menu_x = math_floor(ui._x - ui._w);
	ui_menu_y = math_floor(ui_MENUBAR_H(ui));
	if (config_raw.touch_ui) {
		let menu_w: i32 = math_floor(base_default_element_w * ui_SCALE(base_ui_menu) * 2.0);
		ui_menu_x -= math_floor((menu_w - ui._w) / 2) + math_floor(ui_header_h / 2);
		ui_menu_x += math_floor(2 * ui_SCALE(base_ui_menu));
		ui_menu_y -= math_floor(2 * ui_SCALE(base_ui_menu));
		ui_menu_keep_open = true;
	}
}

function ui_menubar_icon_button(ui: ui_t, i: i32, j: i32): bool {
	let col: u32 = ui.ops.theme.WINDOW_BG_COL;
	let light: bool = col > 0xff666666 ;
	let icon_accent: i32 = light ? 0xff666666 : 0xffaaaaaa;
	let img: image_t = resource_get("icons.k");
	let rect: rect_t = resource_tile50(img, i, j);
	return _ui_image(img, icon_accent, -1.0, rect.x, rect.y, rect.w, rect.h) == ui_state_t.RELEASED;
}
