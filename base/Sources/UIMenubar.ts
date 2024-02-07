
class UIMenubar {

	static defaultMenubarW = 330;
	static workspaceHandle = zui_handle_create({ layout: Layout.Horizontal });
	static menuHandle = zui_handle_create({ layout: Layout.Horizontal });
	static menubarw = UIMenubar.defaultMenubarW;

	///if is_lab
	static _savedCamera: mat4_t = null;
	static _plane: mesh_object_t = null;
	///end

	constructor() {
	}

	static renderUI = () => {
		let ui = UIBase.ui;

		///if (is_paint || is_sculpt)
		let panelx = app_x() - UIToolbar.toolbarw;
		///end
		///if is_lab
		let panelx = app_x();
		///end

		if (zui_window(UIMenubar.menuHandle, panelx, 0, UIMenubar.menubarw, UIHeader.headerh)) {
			ui._x += 1; // Prevent "File" button highlight on startup

			zui_begin_menu();

			if (Config.raw.touch_ui) {

				///if (is_paint || is_sculpt)
				ui._w = UIToolbar.toolbarw;
				///end
				///if is_lab
				ui._w = 36;
				///end

				if (UIMenubar.iconButton(ui, 0, 2)) BoxPreferences.show();
				if (UIMenubar.iconButton(ui, 0, 3)) {
					///if (krom_android || krom_ios)
					Console.toast(tr("Saving project"));
					Project.projectSave();
					///end
					Base.notifyOnNextFrame(() => {
						BoxProjects.show();
					});
				}
				if (UIMenubar.iconButton(ui, 4, 2)) Project.importAsset();
				///if (is_paint || is_lab)
				if (UIMenubar.iconButton(ui, 5, 2)) BoxExport.showTextures();
				///end
				let size = Math.floor(ui._w / zui_SCALE(ui));
				if (UIMenu.show && UIMenu.menuCategory == MenuCategory.MenuViewport) zui_fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (UIMenubar.iconButton(ui, 8, 2)) UIMenubar.showMenu(ui, MenuCategory.MenuViewport);
				if (UIMenu.show && UIMenu.menuCategory == MenuCategory.MenuMode) zui_fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (UIMenubar.iconButton(ui, 9, 2)) UIMenubar.showMenu(ui, MenuCategory.MenuMode);
				if (UIMenu.show && UIMenu.menuCategory == MenuCategory.MenuCamera) zui_fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (UIMenubar.iconButton(ui, 10, 2)) UIMenubar.showMenu(ui, MenuCategory.MenuCamera);
				if (UIMenu.show && UIMenu.menuCategory == MenuCategory.MenuHelp) zui_fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (UIMenubar.iconButton(ui, 11, 2)) UIMenubar.showMenu(ui, MenuCategory.MenuHelp);
				ui.enabled = History.undos > 0;
				if (UIMenubar.iconButton(ui, 6, 2)) History.undo();
				ui.enabled = History.redos > 0;
				if (UIMenubar.iconButton(ui, 7, 2)) History.redo();
				ui.enabled = true;
			}
			else {
				let categories = [tr("File"), tr("Edit"), tr("Viewport"), tr("Mode"), tr("Camera"), tr("Help")];
				for (let i = 0; i < categories.length; ++i) {
					if (zui_menu_button(categories[i]) || (UIMenu.show && UIMenu.menuCommands == null && ui.is_hovered)) {
						UIMenubar.showMenu(ui, i);
					}
				}
			}

			if (UIMenubar.menubarw < ui._x + 10) {
				UIMenubar.menubarw = Math.floor(ui._x + 10);

				///if (is_paint || is_sculpt)
				UIToolbar.toolbarHandle.redraws = 2;
				///end
			}

			zui_end_menu();
		}

		let nodesw = (UINodes.show || UIView2D.show) ? Config.raw.layout[LayoutSize.LayoutNodesW] : 0;
		///if (is_paint || is_sculpt)
		let ww = sys_width() - Config.raw.layout[LayoutSize.LayoutSidebarW] - UIMenubar.menubarw - nodesw;
		panelx = (app_x() - UIToolbar.toolbarw) + UIMenubar.menubarw;
		///else
		let ww = sys_width() - UIMenubar.menubarw - nodesw;
		panelx = (app_x()) + UIMenubar.menubarw;
		///end

		if (zui_window(UIMenubar.workspaceHandle, panelx, 0, ww, UIHeader.headerh)) {

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
				Context.raw.brushBlendDirty = true;
				UIHeader.headerHandle.redraws = 2;
				Context.mainObject().skip_context = null;

				if (UIHeader.worktab.position == SpaceType.Space3D) {
					if (UIMenubar._savedCamera != null) {
						transform_set_matrix(scene_camera.base.transform, UIMenubar._savedCamera);
						UIMenubar._savedCamera = null;
					}
					scene_meshes = [Context.mainObject()];
				}
				else { // Space2D
					if (UIMenubar._plane == null) {
						let mesh: any = Geom.make_plane(1, 1, 2, 2);
						let raw = {
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
						let md: mesh_data_t;
						mesh_data_create(raw, (_md: mesh_data_t) => { md = _md; });
						let dotPlane: mesh_object_t = scene_get_child(".Plane").ext;
						UIMenubar._plane = mesh_object_create(md, dotPlane.materials);
						array_remove(scene_meshes, UIMenubar._plane);
					}

					if (UIMenubar._savedCamera == null) {
						UIMenubar._savedCamera = mat4_clone(scene_camera.base.transform.local);
					}
					scene_meshes = [UIMenubar._plane];
					let m = mat4_identity();
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

	static showMenu = (ui: zui_t, category: i32) => {
		UIMenu.show = true;
		UIMenu.menuCommands = null;
		UIMenu.menuCategory = category;
		UIMenu.menuCategoryW = ui._w;
		UIMenu.menuCategoryH = Math.floor(zui_MENUBAR_H(ui));
		UIMenu.menuX = Math.floor(ui._x - ui._w);
		UIMenu.menuY = Math.floor(zui_MENUBAR_H(ui));
		if (Config.raw.touch_ui) {
			let menuW = Math.floor(Base.defaultElementW * zui_SCALE(Base.uiMenu) * 2.0);
			UIMenu.menuX -= Math.floor((menuW - ui._w) / 2) + Math.floor(UIHeader.headerh / 2);
			UIMenu.menuX += Math.floor(2 * zui_SCALE(Base.uiMenu));
			UIMenu.menuY -= Math.floor(2 * zui_SCALE(Base.uiMenu));
			UIMenu.keepOpen = true;
		}
	}

	static iconButton = (ui: zui_t, i: i32, j: i32): bool => {
		let col = ui.t.WINDOW_BG_COL;
		if (col < 0) col += 4294967296;
		let light = col > 0xff666666 + 4294967296;
		let iconAccent = light ? 0xff666666 : 0xffaaaaaa;
		let img = Res.get("icons.k");
		let rect = Res.tile50(img, i, j);
		return zui_image(img, iconAccent, null, rect.x, rect.y, rect.w, rect.h) == State.Released;
	}
}
