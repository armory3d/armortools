package arm.ui;

import zui.Zui;
import zui.Id;
import iron.object.Object;
import iron.object.MeshObject;
import iron.Scene;

class TabOutliner {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab, tr("Outliner"))) {
			ui.row([1 / 4]);
			if (ui.button(tr("Import"))) Project.importMesh();

			var i = 0;
			function drawList(h: Handle, o: Object) {
				if (o.name.charAt(0) == ".") return; // Hidden
				if (Std.is(o, MeshObject) && cast(o, MeshObject).force_context != null) return; // Show mesh context only
				var b = false;
				if (i % 2 == 0) {
					ui.fill(-ui._x, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.WINDOW_BG_COL - 0x00040404);
				}
				if (Context.object == o) {
					ui.fill(-ui._x, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}
				if (o.children.length > 0) {
					ui.row([1 / 13, 12 / 13]);
					b = ui.panel(h.nest(i, {selected: true}), "", true, false, false);
					ui.text(o.name);
				}
				else {
					ui._x += 18; // Sign offset

					// Draw line that shows parent relations
					ui.fill(-15 / ui.SCALE(), ui.t.ELEMENT_H / 2, 15, 1, ui.t.ACCENT_COL);

					ui.text(o.name);
					ui._x -= 18;
				}
				if (ui.isReleased) {
					Context.selectObject(o);
					Context.ddirty = 2;
					UISidebar.inst.hwnd1.redraws = 2;
				}
				i++;
				if (b) {
					var currentY = ui._y;
					for (c in o.children) {
						ui.indent();
						drawList(h, c);
						ui.unindent();
					}

					// Draw line that shows parent relations
					var h = (ui._y - currentY - ui.ELEMENT_H() / 2) / ui.SCALE();
					ui.fill(15, -h - ui.t.ELEMENT_H / 2, 1, h, ui.t.ACCENT_COL);
				}
			}
			var _ELEMENT_OFFSET = ui.t.ELEMENT_OFFSET;
			ui.t.ELEMENT_OFFSET = 0;
			for (c in Scene.active.root.children) {
				drawList(Id.handle(), c);
			}
			ui.t.ELEMENT_OFFSET = _ELEMENT_OFFSET;
		}
	}
}
