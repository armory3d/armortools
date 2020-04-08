package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import iron.RenderPath;
import arm.node.MaterialParser;
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
		var WINDOW_BG_COL = ui.t.WINDOW_BG_COL;
		ui.t.WINDOW_BG_COL = ui.t.SEPARATOR_COL;
		if (ui.window(menuHandle, panelx, 0, menubarw, Std.int(UIHeader.defaultHeaderH * ui.SCALE()))) {
			var _w = ui._w;
			ui._x += 1; // Prevent "File" button highlight on startup

			var ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;
			var BUTTON_COL = ui.t.BUTTON_COL;
			ui.t.BUTTON_COL = ui.t.SEPARATOR_COL;

			menuButton(tr("File"), MenuFile);
			menuButton(tr("Edit"), MenuEdit);
			menuButton(tr("Viewport"), MenuViewport);
			menuButton(tr("Mode"), MenuMode);
			menuButton(tr("Camera"), MenuCamera);
			menuButton(tr("Help"), MenuHelp);

			if (menubarw < ui._x + 10) {
				menubarw = Std.int(ui._x + 10);
				UIToolbar.inst.toolbarHandle.redraws = 2;
			}

			ui._w = _w;
			ui.t.ELEMENT_OFFSET = ELEMENT_OFFSET;
			ui.t.BUTTON_COL = BUTTON_COL;
		}
		ui.t.WINDOW_BG_COL = WINDOW_BG_COL;

		var panelx = (iron.App.x() - UIToolbar.inst.toolbarw) + menubarw;
		if (ui.window(workspaceHandle, panelx, 0, System.windowWidth() - UISidebar.inst.windowW - menubarw, Std.int(UIHeader.defaultHeaderH * ui.SCALE()))) {
			ui.tab(UIHeader.inst.worktab, tr("Paint"));
			ui.tab(UIHeader.inst.worktab, tr("Material"));
			ui.tab(UIHeader.inst.worktab, tr("Render"));
			if (UIHeader.inst.worktab.changed) {
				Context.ddirty = 2;
				UIToolbar.inst.toolbarHandle.redraws = 2;
				UIHeader.inst.headerHandle.redraws = 2;
				UISidebar.inst.hwnd.redraws = 2;
				UISidebar.inst.hwnd1.redraws = 2;
				UISidebar.inst.hwnd2.redraws = 2;

				if (UIHeader.inst.worktab.position == SpacePaint) {
					Context.selectTool(ToolBrush);
				}
				else {
					Context.selectTool(ToolGizmo);
				}

				if (UIHeader.inst.worktab.position == SpaceMaterial) {
					Layers.updateFillLayers();
				}

				MaterialParser.parsePaintMaterial();
				MaterialParser.parseMeshMaterial();
				Context.mainObject().skip_context = null;
			}
		}
	}

	@:access(zui.Zui)
	function menuButton(name: String, category: Int) {
		var ui = UISidebar.inst.ui;
		ui._w = Std.int(ui.ops.font.width(ui.fontSize, name) + 25);
		if (ui.button(name) || (UIMenu.show && UIMenu.menuCommands == null && ui.isHovered)) {
			UIMenu.show = true;
			UIMenu.menuCategory = category;
			UIMenu.menuX = Std.int(ui._x - ui._w);
			UIMenu.menuY = UIHeader.inst.headerh;
		}
	}
}
