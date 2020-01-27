package arm.ui;

import zui.Zui;
import zui.Id;
import arm.util.MeshUtil;

class TabMeshes {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "Meshes")) {
			ui.row([1 / 4, 1 / 4]);

			if (ui.button("Import")) Project.importMesh();
			if (ui.isHovered) ui.tooltip("Import mesh file (" + Config.keymap.file_import_assets  + ")");

			if (ui.button("Tools...")) {
				UIMenu.draw(function(ui: Zui) {
					ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 6, ui.t.SEPARATOR_COL);
					ui.text("Tools", Right, ui.t.HIGHLIGHT_COL);
					if (ui.button("Flip Normals", Left)) {
						MeshUtil.flipNormals();
						Context.ddirty = 2;
					}
					if (ui.button("Calculate Normals", Left)) {
						MeshUtil.calcNormals();
						Context.ddirty = 2;
					}
					if (ui.button("Rotate X", Left)) {
						MeshUtil.swapAxis(1, 2);
						Context.ddirty = 2;
					}
					if (ui.button("Rotate Y", Left)) {
						MeshUtil.swapAxis(2, 0);
						Context.ddirty = 2;
					}
					if (ui.button("Rotate Z", Left)) {
						MeshUtil.swapAxis(0, 1);
						Context.ddirty = 2;
					}
				});
			}

			if (ui.panel(Id.handle({selected: false}), "Scene", true)) {
				ui.indent();
				for (o in Project.paintObjects) {
					ui.text(o.name);
				}
				ui.unindent();
			}
		}
	}
}
