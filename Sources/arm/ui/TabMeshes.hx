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
			if (ui.isHovered) ui.tooltip("Import mesh file (" + Config.keymap.import_assets  + ")");

			UITrait.inst.splitBy = ui.combo(Id.handle({position: 0}), ["Object", "Group"], "Split By", true);
			if (ui.isHovered) ui.tooltip("Split .obj mesh into objects");

			UITrait.inst.isUdim = ui.check(Id.handle({selected: UITrait.inst.isUdim}), "UDIM Import");
			if (ui.isHovered) ui.tooltip("Split mesh per UDIM tile");

			UITrait.inst.parseTransform = ui.check(Id.handle({selected: UITrait.inst.parseTransform}), "Parse Transforms");
			if (ui.isHovered) ui.tooltip("Load per-object transforms from .fbx");

			if (ui.panel(Id.handle({selected: false}), "Scene", 0, true)) {
				ui.indent();
				for (o in Project.paintObjects) {
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
			}
		}
	}
}
