package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import zui.Ext;
import iron.RenderPath;
import iron.Scene;
import iron.object.MeshObject;
import arm.node.MakeMaterial;
import arm.render.RenderPathPaint;
import arm.Enums;

@:access(zui.Zui)
class UIMenubar {

	public static var inst: UIMenubar;
	public static inline var defaultMenubarW = 330;
	public var workspaceHandle = new Handle({ layout: Horizontal });
	public var menuHandle = new Handle({ layout: Horizontal });
	public var menubarw = defaultMenubarW;

	static var _meshes: Array<MeshObject> = null;
	static var _savedCamera = iron.math.Mat4.identity();

	public function new() {
		inst = this;
	}

	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UISidebar.inst.ui;

		var panelx = iron.App.x();
		if (ui.window(menuHandle, panelx, 0, menubarw, Std.int(UIHeader.defaultHeaderH * ui.SCALE()))) {
			ui._x += 1; // Prevent "File" button highlight on startup

			Ext.beginMenu(ui);

			if (Config.raw.touch_ui) {
				ui._y += 4;

				#if (krom_android || krom_ios)
				var defaultToolbarW = 36 + 4;
				#else
				var defaultToolbarW = 36;
				#end

				ui._w = Std.int(defaultToolbarW * ui.SCALE());
				if (iconButton(ui, 0, 2)) BoxPreferences.show();
				if (iconButton(ui, 0, 3)) {
					#if (krom_android || krom_ios)
					Console.toast(tr("Saving project"));
					Project.projectSave();
					#end
					App.notifyOnNextFrame(function() {
						BoxProjects.show();
					});
				}
				if (iconButton(ui, 4, 2)) Project.importAsset();
				if (iconButton(ui, 5, 2)) BoxExport.showTextures();
				var size = defaultToolbarW;
				if (UIMenu.show && UIMenu.menuCategory == MenuViewport) ui.fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (iconButton(ui, 8, 2)) showMenu(ui, MenuViewport);
				if (UIMenu.show && UIMenu.menuCategory == MenuMode) ui.fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (iconButton(ui, 9, 2)) showMenu(ui, MenuMode);
				if (UIMenu.show && UIMenu.menuCategory == MenuCamera) ui.fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (iconButton(ui, 10, 2)) showMenu(ui, MenuCamera);
				if (UIMenu.show && UIMenu.menuCategory == MenuHelp) ui.fill(0, -6, size, size - 4, ui.t.HIGHLIGHT_COL);
				if (iconButton(ui, 11, 2)) showMenu(ui, MenuHelp);
				// ui.enabled = History.undos > 0;
				if (iconButton(ui, 6, 2)) History.undo();
				// ui.enabled = History.redos > 0;
				if (iconButton(ui, 7, 2)) History.redo();
				// ui.enabled = true;
			}
			else {
				var categories = [tr("File"), tr("Edit"), tr("Viewport"), tr("Mode"), tr("Camera"), tr("Help")];
				for (i in 0...categories.length) {
					if (Ext.menuButton(ui, categories[i]) || (UIMenu.show && UIMenu.menuCommands == null && ui.isHovered)) {
						showMenu(ui, i);
					}
				}
			}

			if (menubarw < ui._x + 10) {
				menubarw = Std.int(ui._x + 10);
			}

			Ext.endMenu(ui);
		}

		var panelx = (iron.App.x()) + menubarw;
		if (ui.window(workspaceHandle, panelx, 0, System.windowWidth() - menubarw, Std.int(UIHeader.defaultHeaderH * ui.SCALE()))) {
			ui.tab(UIHeader.inst.worktab, tr("3D"));
			ui.tab(UIHeader.inst.worktab, tr("2D"));
			if (UIHeader.inst.worktab.changed) {
				Context.ddirty = 2;
				Context.brushBlendDirty = true;
				UIHeader.inst.headerHandle.redraws = 2;

				Context.mainObject().skip_context = null;

				if (UIHeader.inst.worktab.position == Space3D) {
					if (_meshes != null) {
						Scene.active.meshes = _meshes;
						Scene.active.camera.transform.setMatrix(_savedCamera);
						_meshes = null;
					}
				}
				else { // Space2D
					var plane: MeshObject = cast Scene.active.getChild(".Plane");
					plane.transform.scale.set(1, 1, 1);
					plane.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
					plane.transform.buildMatrix();
					plane.visible = true;
					if (_meshes == null) {
						_meshes = Scene.active.meshes;
						_savedCamera.setFrom(Scene.active.camera.transform.local);
					}
					Scene.active.meshes = [plane];
					var m = iron.math.Mat4.identity();
					m.translate(0, 0, 1.5);
					Scene.active.camera.transform.setMatrix(m);
				}
			}
		}
	}

	function showMenu(ui: Zui, category: Int) {
		UIMenu.show = true;
		UIMenu.menuCategory = category;
		UIMenu.menuX = Std.int(ui._x - ui._w);
		UIMenu.menuY = Std.int(Ext.MENUBAR_H(ui));
		if (Config.raw.touch_ui) {
			var menuW = Std.int(App.defaultElementW * App.uiMenu.SCALE() * 2.0);
			UIMenu.menuX -= Std.int((menuW - ui._w) / 2) + Std.int(UIHeader.inst.headerh / 2);
			// UIMenu.menuY += 4;
			UIMenu.keepOpen = true;
		}
	}

	function iconButton(ui: Zui, i: Int, j: Int): Bool {
		var col = ui.t.WINDOW_BG_COL;
		if (col < 0) col += untyped 4294967296;
		var light = col > 0xff666666 + 4294967296;
		var iconAccent = light ? 0xff666666 : 0xff999999;
		var img = Res.get("icons.k");
		var rect = Res.tile50(img, i, j);
		return ui.image(img, iconAccent, null, rect.x, rect.y, rect.w, rect.h) == State.Released;
	}
}
