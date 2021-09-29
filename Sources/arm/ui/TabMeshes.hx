package arm.ui;

import zui.Zui;
import zui.Id;
import iron.object.MeshObject;
import arm.util.MeshUtil;

class TabMeshes {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UIStatus.inst.statustab, tr("Meshes"))) {

			ui.beginSticky();
			ui.row([1 / 14, 1 / 9, 1 / 9, 1 / 9, 1 / 9, 1 / 14, 1 / 14, 1 / 14]);

			if (ui.button(tr("Import"))) {
				UIMenu.draw(function(ui: Zui) {
					ui.text(tr("Import"), Right, ui.t.HIGHLIGHT_COL);
					if (ui.button(tr("Replace Existing"), Left, '${Config.keymap.file_import_assets}')) {
						Project.importMesh(true);
					}
					if (ui.button(tr("Append"), Left)) {
						Project.importMesh(false);
					}
				}, 3);
			}
			if (ui.isHovered) ui.tooltip(tr("Import mesh file"));

			if (ui.button(tr("Flip Normals"))) {
				MeshUtil.flipNormals();
				Context.ddirty = 2;
			}

			if (ui.button(tr("Calculate Normals"))) {
				MeshUtil.calcNormals();
				Context.ddirty = 2;
			}

			if (ui.button(tr("Geometry to Origin"))) {
				MeshUtil.toOrigin();
				Context.ddirty = 2;
			}

			if (ui.button(tr("Apply Displacement"))) {
				MeshUtil.applyDisplacement();
				MeshUtil.calcNormals();
				Context.ddirty = 2;
			}

			if (ui.button(tr("Rotate X"))) {
				MeshUtil.swapAxis(1, 2);
				Context.ddirty = 2;
			}

			if (ui.button(tr("Rotate Y"))) {
				MeshUtil.swapAxis(2, 0);
				Context.ddirty = 2;
			}

			if (ui.button(tr("Rotate Z"))) {
				MeshUtil.swapAxis(0, 1);
				Context.ddirty = 2;
			}

			ui.endSticky();

			for (i in 0...Project.paintObjects.length) {
				var o = Project.paintObjects[i];
				var h = Id.handle();
				h.selected = o.visible;
				o.visible = ui.check(h, o.name);
				if (ui.isHovered && ui.inputReleasedR) {
					UIMenu.draw(function(ui: Zui) {
						ui.text(o.name, Right, ui.t.HIGHLIGHT_COL);
						if (ui.button(tr("Export"), Left)) {
							Context.exportMeshIndex = i + 1;
							BoxExport.showMesh();
						}
						if (Project.paintObjects.length > 1 && ui.button(tr("Delete"), Left)) {
							Project.paintObjects.remove(o);
							while (o.children.length > 0) {
								var child = o.children[0];
								o.removeChild(child);
								if (Project.paintObjects[0] != child) {
									Project.paintObjects[0].addChild(child);
								}
								if (o.children.length == 0) {
									Project.paintObjects[0].transform.scale.setFrom(o.transform.scale);
									Project.paintObjects[0].transform.buildMatrix();
								}
							}
							iron.data.Data.deleteMesh(o.data.handle);
							o.remove();
							Context.paintObject = Context.mainObject();
							MeshUtil.mergeMesh();
							Context.ddirty = 2;
						}
					}, Project.paintObjects.length > 1 ? 3 : 2);
				}
				if (h.changed) {
					var visibles: Array<MeshObject> = [];
					for (p in Project.paintObjects) if (p.visible) visibles.push(p);
					MeshUtil.mergeMesh(visibles);
					Context.ddirty = 2;
				}
			}
		}
	}
}
