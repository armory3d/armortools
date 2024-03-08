
class UIMenubar {

	static default_menubar_w: i32 = 330;
	static workspace_handle: zui_handle_t = zui_handle_create({ layout: zui_layout_t.HORIZONTAL });
	static menu_handle: zui_handle_t = zui_handle_create({ layout: zui_layout_t.HORIZONTAL });
	static menubarw: i32 = UIMenubar.default_menubar_w;

	///if is_lab
	static _saved_camera: mat4_t = null;
	static _plane: mesh_object_t = null;
	///end

	constructor() {
	}

	static render_ui = () => {
		let ui: zui_t = UIBase.ui;

		///if (is_paint || is_sculpt)
		let panelx: i32 = app_x() - UIToolbar.toolbar_w;
		///end
		///if is_lab
		let panelx: i32 = app_x();
		///end

		if (zui_window(UIMenubar.menu_handle, panelx, 0, UIMenubar.menubarw, UIHeader.headerh)) {
			ui._x += 1; // Prevent "File" button highlight on startup

			zui_begin_menu();

			if (Config.raw.touch_ui) {

				///if (is_paint || is_sculpt)
				ui._w = UIToolbar.toolbar_w;
				///end
				///if is_lab
				ui._w = 36;
				///end

				if (UIMenubar.icon_button(ui, 0, 2)) BoxPreferences.show();
				if (UIMenubar.icon_button(ui, 0, 3)) {
					///if (krom_android || krom_ios)
					Console.toast(tr("Saving project"));
					Project.project_save();
					///end
					base_notify_on_next_frame(() => {
						BoxProjects.show();
					});
				}
				if (UIMenubar.icon_button(ui, 4, 2)) Project.import_asset();
				///if (is_paint || is_lab)
				if (UIMenubar.icon_button(ui, 5, 2)) BoxExport.show_textures();
				///end
				let size: i32 = Math.floor(ui._w / zui_SCALE(ui));
				if (UIMenu.show && UIMenu.menu_category == menu_category_t.VIEWPORT) zui_fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (UIMenubar.icon_button(ui, 8, 2)) UIMenubar.show_menu(ui, menu_category_t.VIEWPORT);
				if (UIMenu.show && UIMenu.menu_category == menu_category_t.MODE) zui_fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (UIMenubar.icon_button(ui, 9, 2)) UIMenubar.show_menu(ui, menu_category_t.MODE);
				if (UIMenu.show && UIMenu.menu_category == menu_category_t.CAMERA) zui_fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (UIMenubar.icon_button(ui, 10, 2)) UIMenubar.show_menu(ui, menu_category_t.CAMERA);
				if (UIMenu.show && UIMenu.menu_category == menu_category_t.HELP) zui_fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (UIMenubar.icon_button(ui, 11, 2)) UIMenubar.show_menu(ui, menu_category_t.HELP);
				ui.enabled = History.undos > 0;
				if (UIMenubar.icon_button(ui, 6, 2)) History.undo();
				ui.enabled = History.redos > 0;
				if (UIMenubar.icon_button(ui, 7, 2)) History.redo();
				ui.enabled = true;
			}
			else {
				let categories: string[] = [tr("File"), tr("Edit"), tr("Viewport"), tr("Mode"), tr("Camera"), tr("Help")];
				for (let i: i32 = 0; i < categories.length; ++i) {
					if (zui_menu_button(categories[i]) || (UIMenu.show && UIMenu.menu_commands == null && ui.is_hovered)) {
						UIMenubar.show_menu(ui, i);
					}
				}
			}

			if (UIMenubar.menubarw < ui._x + 10) {
				UIMenubar.menubarw = Math.floor(ui._x + 10);

				///if (is_paint || is_sculpt)
				UIToolbar.toolbar_handle.redraws = 2;
				///end
			}

			zui_end_menu();
		}

		let nodesw: i32 = (UINodes.show || UIView2D.show) ? Config.raw.layout[layout_size_t.NODES_W] : 0;
		///if (is_paint || is_sculpt)
		let ww: i32 = sys_width() - Config.raw.layout[layout_size_t.SIDEBAR_W] - UIMenubar.menubarw - nodesw;
		panelx = (app_x() - UIToolbar.toolbar_w) + UIMenubar.menubarw;
		///else
		let ww: i32 = sys_width() - UIMenubar.menubarw - nodesw;
		panelx = (app_x()) + UIMenubar.menubarw;
		///end

		if (zui_window(UIMenubar.workspace_handle, panelx, 0, ww, UIHeader.headerh)) {

			if (!Config.raw.touch_ui) {
				zui_tab(UIHeader.worktab, tr("3D View"));
			}
			else {
				zui_fill(0, 0, ui._window_w, ui._window_h + 4, ui.t.SEPARATOR_COL);
			}

			///if is_lab
			zui_tab(UIHeader.worktab, tr("2D View"));
			if (UIHeader.worktab.changed) {
				Context.raw.ddirty = 2;
				Context.raw.brush_blend_dirty = true;
				UIHeader.header_handle.redraws = 2;
				Context.main_object().skip_context = null;

				if (UIHeader.worktab.position == space_type_t.SPACE3D) {
					if (UIMenubar._saved_camera != null) {
						transform_set_matrix(scene_camera.base.transform, UIMenubar._saved_camera);
						UIMenubar._saved_camera = null;
					}
					scene_meshes = [Context.main_object()];
				}
				else { // Space2D
					if (UIMenubar._plane == null) {
						let mesh: any = Geom.make_plane(1, 1, 2, 2);
						let raw: any = {
							name: "2DView",
							vertex_arrays: [
								{ values: mesh.posa, attrib: "pos", data: "short4norm" },
								{ values: mesh.nora, attrib: "nor", data: "short2norm" },
								{ values: mesh.texa, attrib: "tex", data: "short2norm" }
							],
							index_arrays: [
								{ values: mesh.inda, material: 0 }
							],
							scale_pos: mesh.scalePos,
							scale_tex: mesh.scaleTex
						};
						let md: mesh_data_t = mesh_data_create(raw);
						let dot_plane: mesh_object_t = scene_get_child(".Plane").ext;
						UIMenubar._plane = mesh_object_create(md, dot_plane.materials);
						array_remove(scene_meshes, UIMenubar._plane);
					}

					if (UIMenubar._saved_camera == null) {
						UIMenubar._saved_camera = mat4_clone(scene_camera.base.transform.local);
					}
					scene_meshes = [UIMenubar._plane];
					let m: mat4_t = mat4_identity();
					mat4_translate(m, 0, 0, 1.6);
					transform_set_matrix(scene_camera.base.transform, m);
				}
				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				RenderPathRaytrace.ready = false;
				///end
			}
			///end
		}
	}

	static show_menu = (ui: zui_t, category: i32) => {
		UIMenu.show = true;
		UIMenu.menu_commands = null;
		UIMenu.menu_category = category;
		UIMenu.menu_category_w = ui._w;
		UIMenu.menu_category_h = Math.floor(zui_MENUBAR_H(ui));
		UIMenu.menu_x = Math.floor(ui._x - ui._w);
		UIMenu.menu_y = Math.floor(zui_MENUBAR_H(ui));
		if (Config.raw.touch_ui) {
			let menuW: i32 = Math.floor(base_default_element_w * zui_SCALE(base_ui_menu) * 2.0);
			UIMenu.menu_x -= Math.floor((menuW - ui._w) / 2) + Math.floor(UIHeader.headerh / 2);
			UIMenu.menu_x += Math.floor(2 * zui_SCALE(base_ui_menu));
			UIMenu.menu_y -= Math.floor(2 * zui_SCALE(base_ui_menu));
			UIMenu.keep_open = true;
		}
	}

	static icon_button = (ui: zui_t, i: i32, j: i32): bool => {
		let col: i32 = ui.t.WINDOW_BG_COL;
		if (col < 0) col += 4294967296;
		let light: bool = col > (0xff666666 + 4294967296);
		let icon_accent: i32 = light ? 0xff666666 : 0xffaaaaaa;
		let img: image_t = Res.get("icons.k");
		let rect: rect_t = Res.tile50(img, i, j);
		return zui_image(img, icon_accent, -1.0, rect.x, rect.y, rect.w, rect.h) == zui_state_t.RELEASED;
	}
}
