package arm.ui;

import kha.System;
import zui.Zui;
import arm.Enums;

class UIToolbar {

	public static var inst: UIToolbar;

	public static inline var defaultToolbarW = 36;

	public var toolbarHandle = new Handle();
	public var toolbarw = defaultToolbarW;

	public var toolNames = [
		tr("Brush"),
		tr("Eraser"),
		tr("Fill"),
		tr("Decal"),
		tr("Text"),
		tr("Clone"),
		tr("Blur"),
		tr("Particle"),
		tr("ColorID"),
		tr("Picker"),
		tr("Gizmo"),
		tr("Bake")
	];
	var toolCount = [10, 2, 1, 1];

	public function new() {
		inst = this;
	}

	@:access(zui.Zui)
	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UISidebar.inst.ui;

		if (ui.window(toolbarHandle, 0, UIHeader.inst.headerh, toolbarw, System.windowHeight() - UIHeader.inst.headerh)) {
			ui._y += 1;

			ui.imageScrollAlign = false;
			var img = Res.get("icons.k");
			var imgw = ui.SCALE() > 1 ? 100 : 50;

			if (UIHeader.inst.worktab.position == SpacePaint) {
				var keys = [
					"(" + Config.keymap.tool_brush + ")",
					"(" + Config.keymap.tool_eraser + ")",
					"(" + Config.keymap.tool_fill + ")",
					"(" + Config.keymap.tool_decal + ")",
					"(" + Config.keymap.tool_text + ")",
					"(" + Config.keymap.tool_clone + ") - " + tr("Hold {key} to set source", ["key" => Config.keymap.set_clone_source]),
					"(" + Config.keymap.tool_blur + ")",
					"(" + Config.keymap.tool_particle + ")",
					"(" + Config.keymap.tool_colorid + ")",
					"(" + Config.keymap.tool_picker + ")"
				];

				for (i in 0...toolCount[SpacePaint]) {
					ui._x += 2;
					if (Context.tool == i) ui.fill(-1, 2, 32 + 2, 32 + 2, ui.t.HIGHLIGHT_COL);
					var rect = Res.tile50(img, i, 0);
					if (ui.image(img, -1, null, rect.x, rect.y, rect.w, rect.h) == State.Started) Context.selectTool(i);
					if (ui.isHovered) ui.tooltip(tr(toolNames[i]) + " " + keys[i]);
					ui._x -= 2;
					ui._y += 2;
				}
			}
			else if (UIHeader.inst.worktab.position == SpaceMaterial) {
				ui._x += 2;
				if (Context.tool == ToolGizmo) ui.fill(-1, 2, 32 + 2, 32 + 2, ui.t.HIGHLIGHT_COL);
				if (ui.image(img, -1, null, imgw * 10, 0, imgw, imgw) == State.Started) Context.selectTool(ToolGizmo);
				if (ui.isHovered) ui.tooltip(tr("Gizmo") + " (G)");
				ui._x -= 2;
				ui._y += 2;

				ui._x += 2;
				if (Context.tool == ToolPicker) ui.fill(-1, 2, 32 + 2, 32 + 2, ui.t.HIGHLIGHT_COL);
				if (ui.image(img, -1, null, imgw * 9, 0, imgw, imgw) == State.Started) Context.selectTool(ToolPicker);
				if (ui.isHovered) ui.tooltip(tr("Picker") + " (V)");
				ui._x -= 2;
				ui._y += 2;
			}
			else if (UIHeader.inst.worktab.position == SpaceBake) {
				ui._x += 2;
				if (Context.tool == ToolBake) ui.fill(-1, 2, 32 + 2, 32 + 2, ui.t.HIGHLIGHT_COL);
				if (ui.image(img, -1, null, imgw * 11, 0, imgw, imgw) == State.Started) Context.selectTool(ToolBake);
				if (ui.isHovered) ui.tooltip(tr("Bake") + " (K)");
				ui._x -= 2;
				ui._y += 2;
			}
			else if (UIHeader.inst.worktab.position == SpaceRender) {
				ui._x += 2;
				if (Context.tool == ToolGizmo) ui.fill(-1, 2, 32 + 2, 32 + 2, ui.t.HIGHLIGHT_COL);
				if (ui.image(img, -1, null, imgw * 10, 0, imgw, imgw) == State.Started) Context.selectTool(ToolGizmo);
				if (ui.isHovered) ui.tooltip(tr("Gizmo") + " (G)");
				ui._x -= 2;
				ui._y += 2;
			}

			ui.imageScrollAlign = true;
		}
	}
}
