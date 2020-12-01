package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import zui.Ext;
import iron.RenderPath;
import arm.node.MakeMaterial;
import arm.render.RenderPathPaint;
import arm.Enums;

class UIMenubar {

	public static var inst: UIMenubar;
	public static inline var defaultMenubarW = 330;
	public var workspaceHandle = new Handle({layout: Horizontal});
	public var menuHandle = new Handle({layout: Horizontal});
	public var menubarw = defaultMenubarW;

	public function new() {
		inst = this;
	}

	@:access(zui.Zui)
	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UISidebar.inst.ui;

		var panelx = iron.App.x() - UIToolbar.inst.toolbarw;
		if (ui.window(menuHandle, panelx, 0, menubarw, Std.int(UIHeader.defaultHeaderH * ui.SCALE()))) {
			ui._x += 1; // Prevent "File" button highlight on startup

			Ext.beginMenu(ui);

			var menuCategories = 6;
			for (i in 0...menuCategories) {
				var categories = [tr("File"), tr("Edit"), tr("Viewport"), tr("Mode"), tr("Camera"), tr("Help")];
				if (Ext.menuButton(ui, categories[i]) || (UIMenu.show && UIMenu.menuCommands == null && ui.isHovered)) {
					UIMenu.show = true;
					UIMenu.menuCategory = i;
					UIMenu.menuX = Std.int(ui._x - ui._w);
					UIMenu.menuY = Std.int(Ext.MENUBAR_H(ui));
				}
			}

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
			// ui.tab(UIHeader.inst.worktab, tr("Render"));
			if (UIHeader.inst.worktab.changed) {
				Context.ddirty = 2;
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
				}
				else {
					Context.selectTool(ToolGizmo);
				}

				if (UIHeader.inst.worktab.position == SpaceMaterial) {
					Layers.updateFillLayers();
				}

				Context.mainObject().skip_context = null;
			}
		}
	}
}
