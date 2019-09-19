package arm.ui;

import zui.Zui;
import zui.Id;
import iron.object.Object;
import iron.object.MeshObject;
import iron.Scene;

class TabOutliner {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab, "Outliner")) {
			ui.row([1/4]);
			if (ui.button("Import")) Context.importMesh();

			var i = 0;
			function drawList(h:Handle, o:Object) {
				if (o.name.charAt(0) == '.') return; // Hidden
				if (Std.is(o, MeshObject) && cast(o, MeshObject).force_context != null) return; // Show mesh context only
				var b = false;
				if (Context.object == o) {
					ui.fill(0, 0, ui._windowW, ui.t.ELEMENT_H, ui.t.HIGHLIGHT_COL);
				}
				if (o.children.length > 0) {
					b = ui.panel(h.nest(i, {selected: true}), o.name, 0, true);
				}
				else {
					ui._x += 18; // Sign offset
					ui.text(o.name);
					ui._x -= 18;
				}
				if (ui.isReleased) {
					Context.selectObject(o);
					Context.ddirty = 2;
				}
				i++;
				if (b) {
					for (c in o.children) {
						ui.indent();
						drawList(h, c);
						ui.unindent();
					}
				}
			}
			for (c in Scene.active.root.children) {
				drawList(Id.handle(), c);
			}
		}
	}
}
