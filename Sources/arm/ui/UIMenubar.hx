package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import zui.Ext;
import iron.RenderPath;
import arm.node.MakeMaterial;
import arm.render.RenderPathPaint;
import arm.Enums;

@:access(zui.Zui)
class UIMenubar {

	public static var inst: UIMenubar;
	public static inline var defaultMenubarW = 330;
	public var workspaceHandle = new Handle({layout: Horizontal});
	public var menuHandle = new Handle({layout: Horizontal});
	public var menubarw = defaultMenubarW;

	public function new() {
		inst = this;
	}

	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UISidebar.inst.ui;

		var panelx = iron.App.x() - UIToolbar.inst.toolbarw;
		if (ui.window(menuHandle, panelx, 0, menubarw, Std.int(UIHeader.defaultHeaderH * ui.SCALE()))) {
			ui._x += 1; // Prevent "File" button highlight on startup

			Ext.beginMenu(ui);

			#if (krom_android || krom_ios)
			ui._w = Std.int(UIToolbar.defaultToolbarW * ui.SCALE());
			if (iconButton(ui, 0)) BoxPreferences.show();
			if (iconButton(ui, 1)) Project.projectNewBox();
			if (iconButton(ui, 2)) Project.projectOpen();
			if (iconButton(ui, 3)) Project.projectSave();
			if (iconButton(ui, 4)) Project.importAsset();
			if (iconButton(ui, 5)) BoxExport.showTextures();
			ui.enabled = History.undos > 0;
			if (iconButton(ui, 6)) History.undo();
			ui.enabled = History.redos > 0;
			if (iconButton(ui, 7)) History.redo();
			ui.enabled = true;
			if (UIMenu.show && UIMenu.menuCategory == MenuViewport) ui.fill(0, 2, 32 + 4, 32, ui.t.HIGHLIGHT_COL);
			if (iconButton(ui, 8)) showMenu(ui, MenuViewport);
			if (UIMenu.show && UIMenu.menuCategory == MenuMode) ui.fill(0, 2, 32 + 4, 32, ui.t.HIGHLIGHT_COL);
			if (iconButton(ui, 9)) showMenu(ui, MenuMode);
			if (UIMenu.show && UIMenu.menuCategory == MenuCamera) ui.fill(0, 2, 32 + 4, 32, ui.t.HIGHLIGHT_COL);
			if (iconButton(ui, 10)) showMenu(ui, MenuCamera);
			if (UIMenu.show && UIMenu.menuCategory == MenuHelp) ui.fill(0, 2, 32 + 4, 32, ui.t.HIGHLIGHT_COL);
			if (iconButton(ui, 11)) showMenu(ui, MenuHelp);
			#else
			var categories = [tr("File"), tr("Edit"), tr("Viewport"), tr("Mode"), tr("Camera"), tr("Help")];
			for (i in 0...categories.length) {
				if (Ext.menuButton(ui, categories[i]) || (UIMenu.show && UIMenu.menuCommands == null && ui.isHovered)) {
					showMenu(ui, i);
				}
			}
			#end

			if (menubarw < ui._x + 10) {
				menubarw = Std.int(ui._x + 10);
				UIToolbar.inst.toolbarHandle.redraws = 2;
			}

			Ext.endMenu(ui);
		}

		var panelx = (iron.App.x() - UIToolbar.inst.toolbarw) + menubarw;
		if (ui.window(workspaceHandle, panelx, 0, System.windowWidth() - Config.raw.layout[LayoutSidebarW] - menubarw, Std.int(UIHeader.defaultHeaderH * ui.SCALE()))) {
			ui.tab(UIHeader.inst.worktab, tr("Paint"));
			ui.tab(UIHeader.inst.worktab, tr("Material"));
			ui.tab(UIHeader.inst.worktab, tr("Bake"));
			if (UIHeader.inst.worktab.changed) {
				Context.ddirty = 2;
				Context.brushBlendDirty = true;
				UIToolbar.inst.toolbarHandle.redraws = 2;
				UIHeader.inst.headerHandle.redraws = 2;
				UISidebar.inst.hwnd0.redraws = 2;
				UISidebar.inst.hwnd1.redraws = 2;
				UISidebar.inst.hwnd2.redraws = 2;

				if (UIHeader.inst.worktab.position == SpacePaint) {
					Context.selectTool(ToolBrush);
				}
				else if (UIHeader.inst.worktab.position == SpaceBake) {
					Context.selectTool(ToolBake);
					#if (kha_direct3d12 || kha_vulkan)
					// Bake in lit mode for now
					if (Context.viewportMode == ViewPathTrace) {
						Context.viewportMode = ViewLit;
					}
					#end
				}
				else if (UIHeader.inst.worktab.position == SpaceMaterial) {
					Context.selectTool(ToolPicker);
					Layers.updateFillLayers();
				}

				Context.mainObject().skip_context = null;
			}
		}
	}

	function showMenu(ui: Zui, category: Int) {
		UIMenu.show = true;
		UIMenu.menuCategory = category;
		UIMenu.menuX = Std.int(ui._x - ui._w);
		UIMenu.menuY = Std.int(Ext.MENUBAR_H(ui));
		#if (krom_android || krom_ios)
		var menuW = Std.int(App.defaultElementW * App.uiMenu.SCALE() * 2.0);
		UIMenu.menuX -= Std.int((menuW - ui._w) / 2);
		UIMenu.menuY += 4;
		#end
	}

	#if (krom_android || krom_ios)
	function iconButton(ui: Zui, i: Int): Bool {
		var col = ui.t.WINDOW_BG_COL;
		if (col < 0) col += untyped 4294967296;
		var light = col > 0xff666666 + 4294967296;
		var iconAccent = light ? 0xff666666 : 0xffbbbbbb;
		var img = Res.get("icons.k");
		var rect = Res.tile50(img, i, 2);
		return ui.image(img, iconAccent, null, rect.x, rect.y, rect.w, rect.h) == State.Released;
	}
	#end
}
