
class TabObjects {

	static materialId = 0;

	static roundfp = (f: f32, precision = 2): f32 => {
		f *= Math.pow(10, precision);
		return Math.round(f) / Math.pow(10, precision);
	}

	static draw = (htab: HandleRaw) => {
		let ui = UIBase.ui;
		if (Zui.tab(htab, tr("Objects"))) {
			Zui.beginSticky();
			Zui.row([1 / 4]);
			if (Zui.button("Import")) {
				Project.importMesh(false, () => {
					BaseObject.setParent(Project.paintObjects.pop().base, null);
				});
			}
			Zui.endSticky();

			if (Zui.panel(Zui.handle("tabobjects_0", {selected: true}), "Outliner")) {
				// ui.indent();
				ui._y -= Zui.ELEMENT_OFFSET(ui);

				let listX = ui._x;
				let listW = ui._w;

				let lineCounter = 0;
				let drawList = (listHandle: HandleRaw, currentObject: TBaseObject) => {
					if (currentObject.name.charAt(0) == ".") return; // Hidden
					let b = false;

					// Highlight every other line
					if (lineCounter % 2 == 0) {
						ui.g.color = ui.t.SEPARATOR_COL;
						g2_fill_rect(0, ui._y, ui._windowW, Zui.ELEMENT_H(ui));
						ui.g.color = 0xffffffff;
					}

					// Highlight selected line
					if (currentObject == Context.raw.selectedObject) {
						ui.g.color = 0xff205d9c;
						g2_fill_rect(0, ui._y, ui._windowW, Zui.ELEMENT_H(ui));
						ui.g.color = 0xffffffff;
					}

					if (currentObject.children.length > 0) {
						Zui.row([1 / 13, 12 / 13]);
						b = Zui.panel(Zui.nest(listHandle, lineCounter, {selected: true}), "", true, false, false);
						Zui.text(currentObject.name);
					}
					else {
						ui._x += 18; // Sign offset

						// Draw line that shows parent relations
						ui.g.color = ui.t.ACCENT_COL;
						g2_draw_line(ui._x - 10, ui._y + Zui.ELEMENT_H(ui) / 2, ui._x, ui._y + Zui.ELEMENT_H(ui) / 2);
						ui.g.color = 0xffffffff;

						Zui.text(currentObject.name);
						ui._x -= 18;
					}

					lineCounter++;
					// Undo applied offset for row drawing caused by endElement() in Zui.hx
					ui._y -= Zui.ELEMENT_OFFSET(ui);

					if (ui.isReleased) {
						Context.raw.selectedObject = currentObject;
					}

					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw((ui: ZuiRaw) => {
							if (UIMenu.menuButton(ui, "Assign Material")) {
								TabObjects.materialId++;

								for (let sh of _scene_raw.shader_datas) {
									if (sh.name == "Material_data") {
										let s: shader_data_t = JSON.parse(JSON.stringify(sh));
										s.name = "TempMaterial_data" + TabObjects.materialId;
										_scene_raw.shader_datas.push(s);
										break;
									}
								}

								for (let mat of _scene_raw.material_datas) {
									if (mat.name == "Material") {
										let m: material_data_t = JSON.parse(JSON.stringify(mat));
										m.name = "TempMaterial" + TabObjects.materialId;
										m.shader = "TempMaterial_data" + TabObjects.materialId;
										_scene_raw.material_datas.push(m);
										break;
									}
								}

								Data.getMaterial("Scene", "TempMaterial" + TabObjects.materialId, (md: material_data_t) => {
									let mo: TMeshObject = currentObject.ext;
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
						g2_draw_line(ui._x + 14, currentY, ui._x + 14, ui._y - Zui.ELEMENT_H(ui) / 2);
						ui.g.color = 0xffffffff;
					}
				}
				for (let c of _scene_root.children) {
					drawList(Zui.handle("tabobjects_1"), c);
				}

				// ui.unindent();
			}

			if (Zui.panel(Zui.handle("tabobjects_2", {selected: true}), 'Properties')) {
				// ui.indent();

				if (Context.raw.selectedObject != null) {
					let h = Zui.handle("tabobjects_3");
					h.selected = Context.raw.selectedObject.visible;
					Context.raw.selectedObject.visible = Zui.check(h, "Visible");

					let t = Context.raw.selectedObject.transform;
					let localPos = t.loc;
					let worldPos = vec4_create(transform_world_x(t), transform_world_y(t), transform_world_z(t), 1.0);
					let scale = t.scale;
					let rot = quat_get_euler(t.rot);
					let dim = t.dim;
					vec4_mult(rot, 180 / 3.141592);
					let f = 0.0;

					Zui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					Zui.text("Loc");

					h = Zui.handle("tabobjects_4");
					h.text = TabObjects.roundfp(localPos.x) + "";
					f = parseFloat(Zui.textInput(h, "X"));
					if (h.changed) localPos.x = f;

					h = Zui.handle("tabobjects_5");
					h.text = TabObjects.roundfp(localPos.y) + "";
					f = parseFloat(Zui.textInput(h, "Y"));
					if (h.changed) localPos.y = f;

					h = Zui.handle("tabobjects_6");
					h.text = TabObjects.roundfp(localPos.z) + "";
					f = parseFloat(Zui.textInput(h, "Z"));
					if (h.changed) localPos.z = f;

					Zui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					Zui.text("Rotation");

					h = Zui.handle("tabobjects_7");
					h.text = TabObjects.roundfp(rot.x) + "";
					f = parseFloat(Zui.textInput(h, "X"));
					let changed = false;
					if (h.changed) { changed = true; rot.x = f; }

					h = Zui.handle("tabobjects_8");
					h.text = TabObjects.roundfp(rot.y) + "";
					f = parseFloat(Zui.textInput(h, "Y"));
					if (h.changed) { changed = true; rot.y = f; }

					h = Zui.handle("tabobjects_9");
					h.text = TabObjects.roundfp(rot.z) + "";
					f = parseFloat(Zui.textInput(h, "Z"));
					if (h.changed) { changed = true; rot.z = f; }

					if (changed && Context.raw.selectedObject.name != "Scene") {
						vec4_mult(rot, 3.141592 / 180);
						quat_from_euler(Context.raw.selectedObject.transform.rot, rot.x, rot.y, rot.z);
						transform_build_matrix(Context.raw.selectedObject.transform);
						// ///if arm_physics
						// if (rb != null) rb.syncTransform();
						// ///end
					}

					Zui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					Zui.text("Scale");

					h = Zui.handle("tabobjects_10");
					h.text = TabObjects.roundfp(scale.x) + "";
					f = parseFloat(Zui.textInput(h, "X"));
					if (h.changed) scale.x = f;

					h = Zui.handle("tabobjects_11");
					h.text = TabObjects.roundfp(scale.y) + "";
					f = parseFloat(Zui.textInput(h, "Y"));
					if (h.changed) scale.y = f;

					h = Zui.handle("tabobjects_12");
					h.text = TabObjects.roundfp(scale.z) + "";
					f = parseFloat(Zui.textInput(h, "Z"));
					if (h.changed) scale.z = f;

					Zui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					Zui.text("Dimensions");

					h = Zui.handle("tabobjects_13");
					h.text = TabObjects.roundfp(dim.x) + "";
					f = parseFloat(Zui.textInput(h, "X"));
					if (h.changed) dim.x = f;

					h = Zui.handle("tabobjects_14");
					h.text = TabObjects.roundfp(dim.y) + "";
					f = parseFloat(Zui.textInput(h, "Y"));
					if (h.changed) dim.y = f;

					h = Zui.handle("tabobjects_15");
					h.text = TabObjects.roundfp(dim.z) + "";
					f = parseFloat(Zui.textInput(h, "Z"));
					if (h.changed) dim.z = f;

					Context.raw.selectedObject.transform.dirty = true;

					if (Context.raw.selectedObject.name == "Scene") {
						let p = scene_world;
						p.strength = Zui.slider(Zui.handle("tabobjects_16", {value: p.strength}), "Environment", 0.0, 5.0, true);
					}
					else if (Context.raw.selectedObject.ext.constructor == TLightObject) {
						let light = Context.raw.selectedObject.ext;
						let lightHandle = Zui.handle("tabobjects_17");
						lightHandle.value = light.data.strength / 10;
						light.data.strength = Zui.slider(lightHandle, "Strength", 0.0, 5.0, true) * 10;
					}
					else if (Context.raw.selectedObject.ext.constructor == TCameraObject) {
						let cam = Context.raw.selectedObject.ext;
						let fovHandle = Zui.handle("tabobjects_18");
						fovHandle.value = Math.floor(cam.data.fov * 100) / 100;
						cam.data.fov = Zui.slider(fovHandle, "FoV", 0.3, 2.0, true);
						if (fovHandle.changed) {
							CameraObject.buildProjection(cam);
						}
					}
				}

				// ui.unindent();
			}
		}
	}
}
