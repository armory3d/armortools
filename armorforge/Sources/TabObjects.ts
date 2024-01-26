
class TabObjects {

	static materialId = 0;

	static roundfp = (f: f32, precision = 2): f32 => {
		f *= Math.pow(10, precision);
		return Math.round(f) / Math.pow(10, precision);
	}

	static draw = (htab: Handle) => {
		let ui = UIBase.ui;
		if (ui.tab(htab, tr("Objects"))) {
			ui.beginSticky();
			ui.row([1 / 4]);
			if (ui.button("Import")) {
				Project.importMesh(false, () => {
					Project.paintObjects.pop().setParent(null);
				});
			}
			ui.endSticky();

			if (ui.panel(Zui.handle("tabobjects_0", {selected: true}), "Outliner")) {
				// ui.indent();
				ui._y -= ui.ELEMENT_OFFSET();

				let listX = ui._x;
				let listW = ui._w;

				let lineCounter = 0;
				let drawList = (listHandle: Handle, currentObject: BaseObject) => {
					if (currentObject.name.charAt(0) == ".") return; // Hidden
					let b = false;

					// Highlight every other line
					if (lineCounter % 2 == 0) {
						ui.g.color = ui.t.SEPARATOR_COL;
						ui.g.fillRect(0, ui._y, ui._windowW, ui.ELEMENT_H());
						ui.g.color = 0xffffffff;
					}

					// Highlight selected line
					if (currentObject == Context.raw.selectedObject) {
						ui.g.color = 0xff205d9c;
						ui.g.fillRect(0, ui._y, ui._windowW, ui.ELEMENT_H());
						ui.g.color = 0xffffffff;
					}

					if (currentObject.children.length > 0) {
						ui.row([1 / 13, 12 / 13]);
						b = ui.panel(listHandle.nest(lineCounter, {selected: true}), "", true, false, false);
						ui.text(currentObject.name);
					}
					else {
						ui._x += 18; // Sign offset

						// Draw line that shows parent relations
						ui.g.color = ui.t.ACCENT_COL;
						ui.g.drawLine(ui._x - 10, ui._y + ui.ELEMENT_H() / 2, ui._x, ui._y + ui.ELEMENT_H() / 2);
						ui.g.color = 0xffffffff;

						ui.text(currentObject.name);
						ui._x -= 18;
					}

					lineCounter++;
					// Undo applied offset for row drawing caused by endElement() in Zui.hx
					ui._y -= ui.ELEMENT_OFFSET();

					if (ui.isReleased) {
						Context.raw.selectedObject = currentObject;
					}

					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw((ui: Zui) => {
							if (UIMenu.menuButton(ui, "Assign Material")) {
								TabObjects.materialId++;

								for (let sh of Scene.active.raw.shader_datas) {
									if (sh.name == "Material_data") {
										let s: TShaderData = JSON.parse(JSON.stringify(sh));
										s.name = "TempMaterial_data" + TabObjects.materialId;
										Scene.active.raw.shader_datas.push(s);
										break;
									}
								}

								for (let mat of Scene.active.raw.material_datas) {
									if (mat.name == "Material") {
										let m: TMaterialData = JSON.parse(JSON.stringify(mat));
										m.name = "TempMaterial" + TabObjects.materialId;
										m.shader = "TempMaterial_data" + TabObjects.materialId;
										Scene.active.raw.material_datas.push(m);
										break;
									}
								}

								Data.getMaterial("Scene", "TempMaterial" + TabObjects.materialId, (md: MaterialData) => {
									let mo: MeshObject = currentObject as MeshObject;
									mo.materials = [md];
									MakeMaterial.parseMeshPreviewMaterial(md);
								});
							}
						}, 1);
					}

					if (b) {
						let currentY = ui._y;
						for (let child of currentObject.children) {
							// ui.indent();
							drawList(listHandle, child);
							// ui.unindent();
						}

						// Draw line that shows parent relations
						ui.g.color = ui.t.ACCENT_COL;
						ui.g.drawLine(ui._x + 14, currentY, ui._x + 14, ui._y - ui.ELEMENT_H() / 2);
						ui.g.color = 0xffffffff;
					}
				}
				for (let c of Scene.active.root.children) {
					drawList(Zui.handle("tabobjects_1"), c);
				}

				// ui.unindent();
			}

			if (ui.panel(Zui.handle("tabobjects_2", {selected: true}), 'Properties')) {
				// ui.indent();

				if (Context.raw.selectedObject != null) {
					let h = Zui.handle("tabobjects_3");
					h.selected = Context.raw.selectedObject.visible;
					Context.raw.selectedObject.visible = ui.check(h, "Visible");

					let t = Context.raw.selectedObject.transform;
					let localPos = t.loc;
					let worldPos = new Vec4(t.worldx(), t.worldy(), t.worldz(), 1.0);
					let scale = t.scale;
					let rot = t.rot.getEuler();
					let dim = t.dim;
					rot.mult(180 / 3.141592);
					let f = 0.0;

					ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					ui.text("Loc");

					h = Zui.handle("tabobjects_4");
					h.text = TabObjects.roundfp(localPos.x) + "";
					f = parseFloat(ui.textInput(h, "X"));
					if (h.changed) localPos.x = f;

					h = Zui.handle("tabobjects_5");
					h.text = TabObjects.roundfp(localPos.y) + "";
					f = parseFloat(ui.textInput(h, "Y"));
					if (h.changed) localPos.y = f;

					h = Zui.handle("tabobjects_6");
					h.text = TabObjects.roundfp(localPos.z) + "";
					f = parseFloat(ui.textInput(h, "Z"));
					if (h.changed) localPos.z = f;

					ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					ui.text("Rotation");

					h = Zui.handle("tabobjects_7");
					h.text = TabObjects.roundfp(rot.x) + "";
					f = parseFloat(ui.textInput(h, "X"));
					let changed = false;
					if (h.changed) { changed = true; rot.x = f; }

					h = Zui.handle("tabobjects_8");
					h.text = TabObjects.roundfp(rot.y) + "";
					f = parseFloat(ui.textInput(h, "Y"));
					if (h.changed) { changed = true; rot.y = f; }

					h = Zui.handle("tabobjects_9");
					h.text = TabObjects.roundfp(rot.z) + "";
					f = parseFloat(ui.textInput(h, "Z"));
					if (h.changed) { changed = true; rot.z = f; }

					if (changed && Context.raw.selectedObject.name != "Scene") {
						rot.mult(3.141592 / 180);
						Context.raw.selectedObject.transform.rot.fromEuler(rot.x, rot.y, rot.z);
						Context.raw.selectedObject.transform.buildMatrix();
						// ///if arm_physics
						// let rb = Context.raw.selectedObject.getTrait(RigidBody);
						// if (rb != null) rb.syncTransform();
						// ///end
					}

					ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					ui.text("Scale");

					h = Zui.handle("tabobjects_10");
					h.text = TabObjects.roundfp(scale.x) + "";
					f = parseFloat(ui.textInput(h, "X"));
					if (h.changed) scale.x = f;

					h = Zui.handle("tabobjects_11");
					h.text = TabObjects.roundfp(scale.y) + "";
					f = parseFloat(ui.textInput(h, "Y"));
					if (h.changed) scale.y = f;

					h = Zui.handle("tabobjects_12");
					h.text = TabObjects.roundfp(scale.z) + "";
					f = parseFloat(ui.textInput(h, "Z"));
					if (h.changed) scale.z = f;

					ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					ui.text("Dimensions");

					h = Zui.handle("tabobjects_13");
					h.text = TabObjects.roundfp(dim.x) + "";
					f = parseFloat(ui.textInput(h, "X"));
					if (h.changed) dim.x = f;

					h = Zui.handle("tabobjects_14");
					h.text = TabObjects.roundfp(dim.y) + "";
					f = parseFloat(ui.textInput(h, "Y"));
					if (h.changed) dim.y = f;

					h = Zui.handle("tabobjects_15");
					h.text = TabObjects.roundfp(dim.z) + "";
					f = parseFloat(ui.textInput(h, "Z"));
					if (h.changed) dim.z = f;

					Context.raw.selectedObject.transform.dirty = true;

					if (Context.raw.selectedObject.name == "Scene") {
						let p = Scene.active.world.probe;
						p.raw.strength = ui.slider(Zui.handle("tabobjects_16", {value: p.raw.strength}), "Environment", 0.0, 5.0, true);
					}
					else if (Context.raw.selectedObject.constructor == LightObject) {
						let light = (Context.raw.selectedObject as LightObject);
						let lightHandle = Zui.handle("tabobjects_17");
						lightHandle.value = light.data.raw.strength / 10;
						light.data.raw.strength = ui.slider(lightHandle, "Strength", 0.0, 5.0, true) * 10;
					}
					else if (Context.raw.selectedObject.constructor == CameraObject) {
						let cam = (Context.raw.selectedObject as CameraObject);
						let fovHandle = Zui.handle("tabobjects_18");
						fovHandle.value = Math.floor(cam.data.raw.fov * 100) / 100;
						cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
						if (fovHandle.changed) {
							cam.buildProjection();
						}
					}
				}

				// ui.unindent();
			}
		}
	}
}
