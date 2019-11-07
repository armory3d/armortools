package arm.ui;

import zui.Id;
import arm.util.MeshUtil;
import arm.nodes.MaterialParser;

class TabMeshes {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "Meshes")) {
			ui.row([1/4]);

			if (ui.button("Import")) Project.importMesh();
			if (ui.isHovered) ui.tooltip("Import mesh file (" + Config.keymap.file_import_assets  + ")");

			if (ui.panel(Id.handle({selected: false}), "Geometry")) {

				ui.row([1/2,1/2]);
				if (ui.button("Flip Normals")) {
					MeshUtil.flipNormals();
					Context.ddirty = 2;
				}
				if (ui.button("Calculate Normals")) {
					MeshUtil.calcNormals();
					Context.ddirty = 2;
				}

				ui.row([1/3, 1/3, 1/3]);
				if (ui.button("Rotate X")) {
					MeshUtil.swapAxis(1, 2);
					Context.ddirty = 2;
				}
				if (ui.button("Rotate Y")) {
					MeshUtil.swapAxis(2, 0);
					Context.ddirty = 2;
				}
				if (ui.button("Rotate Z")) {
					MeshUtil.swapAxis(0, 1);
					Context.ddirty = 2;
				}
			}
		}
	}
}
