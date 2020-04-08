package arm.ui;

import kha.System;
import zui.Zui;
import arm.Enums;

class UIToolbar {

	public static var inst: UIToolbar;

	public static inline var defaultToolbarW = 54;

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
		tr("Bake"),
		tr("ColorID"),
		tr("Picker")
	];

	public function new() {
		inst = this;
	}

	@:access(zui.Zui)
	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UISidebar.inst.ui;

		if (ui.window(toolbarHandle, 0, UIHeader.inst.headerh, toolbarw, System.windowHeight() - UIHeader.inst.headerh)) {
			ui._y += 2;

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
					"(" + Config.keymap.tool_clone + ") - " + tr("Hold") + " (" + Config.keymap.set_clone_source + ") " + tr("to set source"),
					"(" + Config.keymap.tool_blur + ")",
					"(" + Config.keymap.tool_particle + ")",
					"(" + Config.keymap.tool_bake + ")",
					"(" + Config.keymap.tool_colorid + ")",
					"(" + Config.keymap.tool_picker + ")"
				];

				for (i in 0...toolNames.length) {
					ui._x += 2;
					if (Context.tool == i) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
					if (ui.image(img, -1, null, i * imgw, 0, imgw, imgw) == State.Started) Context.selectTool(i);
					if (ui.isHovered) ui.tooltip(toolNames[i] + " " + keys[i]);
					ui._x -= 2;
					ui._y += 2;
				}
			}
			else if (UIHeader.inst.worktab.position == SpaceMaterial) {
				ui._x += 2;
				if (Context.tool == ToolGizmo) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
				if (ui.image(img, -1, null, imgw * 11, 0, imgw, imgw) == State.Started) Context.selectTool(ToolGizmo);
				if (ui.isHovered) ui.tooltip(tr("Gizmo") + " (G)");
				ui._x -= 2;
				ui._y += 2;

				ui._x += 2;
				if (Context.tool == ToolPicker) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
				if (ui.image(img, -1, null, imgw * 10, 0, imgw, imgw) == State.Started) Context.selectTool(ToolPicker);
				if (ui.isHovered) ui.tooltip(tr("Picker") + " (V)");
				ui._x -= 2;
				ui._y += 2;
			}
			else if (UIHeader.inst.worktab.position == SpaceRender) {
				ui._x += 2;
				if (Context.tool == ToolGizmo) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
				if (ui.image(img, -1, null, imgw * 11, 0, imgw, imgw) == State.Started) Context.selectTool(ToolGizmo);
				if (ui.isHovered) ui.tooltip(tr("Gizmo") + " (G)");
				ui._x -= 2;
				ui._y += 2;
			}

			ui.imageScrollAlign = true;
		}
	}
}
