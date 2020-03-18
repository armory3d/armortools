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

			if (UISidebar.inst.worktab.position == SpacePaint) {
				var keys = ["(B)", "(E)", "(G)", "(D)", "(T)", "(L) - " + tr("Hold ALT to set source"), "(U)", "(P)", "(K)", "(C)", "(V)"];
				var img = Res.get("icons.k");
				var imgw = ui.SCALE() > 1 ? 100 : 50;
				for (i in 0...toolNames.length) {
					ui._x += 2;
					if (Context.tool == i) ui.rect(-1, -1, 50 + 2, 50 + 2, ui.t.HIGHLIGHT_COL, 2);
					if (ui.image(img, -1, null, i * imgw, 0, imgw, imgw) == State.Started) Context.selectTool(i);
					if (ui.isHovered) ui.tooltip(toolNames[i] + " " + keys[i]);
					ui._x -= 2;
					ui._y += 2;
				}
			}
			else if (UISidebar.inst.worktab.position == SpaceScene) {
				var img = Res.get("icons.k");
				var imgw = ui.SCALE() > 1 ? 100 : 50;
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
