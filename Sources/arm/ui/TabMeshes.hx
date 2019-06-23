package arm.ui;

import zui.Id;
import arm.util.MeshUtil;
import arm.nodes.MaterialParser;

class TabMeshes {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "Meshes")) {
			ui.row([1/4]);

			if (ui.button("Import")) Context.importMesh();
			if (ui.isHovered) ui.tooltip("Import mesh file (Ctrl + Shift + I)");

			UITrait.inst.isUdim = ui.check(Id.handle({selected: UITrait.inst.isUdim}), "UDIM Import");
			if (ui.isHovered) ui.tooltip("Split mesh per UDIM tile");

			UITrait.inst.parseTransform = ui.check(Id.handle({selected: UITrait.inst.parseTransform}), "Parse Transforms");

			if (ui.panel(Id.handle({selected: false}), "Scene", 0, true)) {
				ui.indent();
				for (o in Context.paintObjects) {
					ui.text(o.name);
				}
				ui.unindent();
			}

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

				var dispHandle = Id.handle({value: UITrait.inst.displaceStrength});
				UITrait.inst.displaceStrength = ui.slider(dispHandle, "Displace", 0.0, 2.0, true);
				if (dispHandle.changed) {
					MaterialParser.parseMeshMaterial();
				}
			}
		}
	}
}
