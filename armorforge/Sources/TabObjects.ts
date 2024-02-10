
class TabObjects {

	static materialId = 0;

	static roundfp = (f: f32, precision = 2): f32 => {
		f *= Math.pow(10, precision);
		return Math.round(f) / Math.pow(10, precision);
	}

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		if (zui_tab(htab, tr("Objects"))) {
			zui_begin_sticky();
			zui_row([1 / 4]);
			if (zui_button("Import")) {
				Project.importMesh(false, () => {
					object_set_parent(Project.paintObjects.pop().base, null);
				});
			}
			zui_end_sticky();

			if (zui_panel(zui_handle("tabobjects_0", {selected: true}), "Outliner")) {
				// ui.indent();
				ui._y -= zui_ELEMENT_OFFSET(ui);

				let listX = ui._x;
				let listW = ui._w;

				let lineCounter = 0;
				let drawList = (listHandle: zui_handle_t, currentObject: object_t) => {
					if (currentObject.name.charAt(0) == ".") return; // Hidden
					let b = false;

					// Highlight every other line
					if (lineCounter % 2 == 0) {
						g2_set_color(ui.t.SEPARATOR_COL);
						g2_fill_rect(0, ui._y, ui._window_w, zui_ELEMENT_H(ui));
						g2_set_color(0xffffffff);
					}

					// Highlight selected line
					if (currentObject == Context.raw.selectedObject) {
						g2_set_color(0xff205d9c);
						g2_fill_rect(0, ui._y, ui._window_w, zui_ELEMENT_H(ui));
						g2_set_color(0xffffffff);
					}

					if (currentObject.children.length > 0) {
						zui_row([1 / 13, 12 / 13]);
						b = zui_panel(zui_nest(listHandle, lineCounter, {selected: true}), "", true, false, false);
						zui_text(currentObject.name);
					}
					else {
						ui._x += 18; // Sign offset

						// Draw line that shows parent relations
						g2_set_color(ui.t.ACCENT_COL);
						g2_draw_line(ui._x - 10, ui._y + zui_ELEMENT_H(ui) / 2, ui._x, ui._y + zui_ELEMENT_H(ui) / 2);
						g2_set_color(0xffffffff);

						zui_text(currentObject.name);
						ui._x -= 18;
					}

					lineCounter++;
					// Undo applied offset for row drawing caused by endElement() in Zui.hx
					ui._y -= zui_ELEMENT_OFFSET(ui);

					if (ui.is_released) {
						Context.raw.selectedObject = currentObject;
					}

					if (ui.is_hovered && ui.input_released_r) {
						UIMenu.draw((ui: zui_t) => {
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

								data_get_material("Scene", "TempMaterial" + TabObjects.materialId, (md: material_data_t) => {
									let mo: mesh_object_t = currentObject.ext;
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
						g2_set_color(ui.t.ACCENT_COL);
						g2_draw_line(ui._x + 14, currentY, ui._x + 14, ui._y - zui_ELEMENT_H(ui) / 2);
						g2_set_color(0xffffffff);
					}
				}
				for (let c of _scene_root.children) {
					drawList(zui_handle("tabobjects_1"), c);
				}

				// ui.unindent();
			}

			if (zui_panel(zui_handle("tabobjects_2", {selected: true}), 'Properties')) {
				// ui.indent();

				if (Context.raw.selectedObject != null) {
					let h = zui_handle("tabobjects_3");
					h.selected = Context.raw.selectedObject.visible;
					Context.raw.selectedObject.visible = zui_check(h, "Visible");

					let t = Context.raw.selectedObject.transform;
					let localPos = t.loc;
					let worldPos = vec4_create(transform_world_x(t), transform_world_y(t), transform_world_z(t), 1.0);
					let scale = t.scale;
					let rot = quat_get_euler(t.rot);
					let dim = t.dim;
					vec4_mult(rot, 180 / 3.141592);
					let f = 0.0;

					zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					zui_text("Loc");

					h = zui_handle("tabobjects_4");
					h.text = TabObjects.roundfp(localPos.x) + "";
					f = parseFloat(zui_text_input(h, "X"));
					if (h.changed) localPos.x = f;

					h = zui_handle("tabobjects_5");
					h.text = TabObjects.roundfp(localPos.y) + "";
					f = parseFloat(zui_text_input(h, "Y"));
					if (h.changed) localPos.y = f;

					h = zui_handle("tabobjects_6");
					h.text = TabObjects.roundfp(localPos.z) + "";
					f = parseFloat(zui_text_input(h, "Z"));
					if (h.changed) localPos.z = f;

					zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					zui_text("Rotation");

					h = zui_handle("tabobjects_7");
					h.text = TabObjects.roundfp(rot.x) + "";
					f = parseFloat(zui_text_input(h, "X"));
					let changed = false;
					if (h.changed) { changed = true; rot.x = f; }

					h = zui_handle("tabobjects_8");
					h.text = TabObjects.roundfp(rot.y) + "";
					f = parseFloat(zui_text_input(h, "Y"));
					if (h.changed) { changed = true; rot.y = f; }

					h = zui_handle("tabobjects_9");
					h.text = TabObjects.roundfp(rot.z) + "";
					f = parseFloat(zui_text_input(h, "Z"));
					if (h.changed) { changed = true; rot.z = f; }

					if (changed && Context.raw.selectedObject.name != "Scene") {
						vec4_mult(rot, 3.141592 / 180);
						quat_from_euler(Context.raw.selectedObject.transform.rot, rot.x, rot.y, rot.z);
						transform_build_matrix(Context.raw.selectedObject.transform);
						// ///if arm_physics
						// if (rb != null) rb.syncTransform();
						// ///end
					}

					zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					zui_text("Scale");

					h = zui_handle("tabobjects_10");
					h.text = TabObjects.roundfp(scale.x) + "";
					f = parseFloat(zui_text_input(h, "X"));
					if (h.changed) scale.x = f;

					h = zui_handle("tabobjects_11");
					h.text = TabObjects.roundfp(scale.y) + "";
					f = parseFloat(zui_text_input(h, "Y"));
					if (h.changed) scale.y = f;

					h = zui_handle("tabobjects_12");
					h.text = TabObjects.roundfp(scale.z) + "";
					f = parseFloat(zui_text_input(h, "Z"));
					if (h.changed) scale.z = f;

					zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
					zui_text("Dimensions");

					h = zui_handle("tabobjects_13");
					h.text = TabObjects.roundfp(dim.x) + "";
					f = parseFloat(zui_text_input(h, "X"));
					if (h.changed) dim.x = f;

					h = zui_handle("tabobjects_14");
					h.text = TabObjects.roundfp(dim.y) + "";
					f = parseFloat(zui_text_input(h, "Y"));
					if (h.changed) dim.y = f;

					h = zui_handle("tabobjects_15");
					h.text = TabObjects.roundfp(dim.z) + "";
					f = parseFloat(zui_text_input(h, "Z"));
					if (h.changed) dim.z = f;

					Context.raw.selectedObject.transform.dirty = true;

					if (Context.raw.selectedObject.name == "Scene") {
						let p = scene_world;
						p.strength = zui_slider(zui_handle("tabobjects_16", {value: p.strength}), "Environment", 0.0, 5.0, true);
					}
					else if (Context.raw.selectedObject.ext.constructor == light_object_t) {
						let light = Context.raw.selectedObject.ext;
						let lightHandle = zui_handle("tabobjects_17");
						lightHandle.value = light.data.strength / 10;
						light.data.strength = zui_slider(lightHandle, "Strength", 0.0, 5.0, true) * 10;
					}
					else if (Context.raw.selectedObject.ext.constructor == camera_object_t) {
						let cam = Context.raw.selectedObject.ext;
						let fovHandle = zui_handle("tabobjects_18");
						fovHandle.value = Math.floor(cam.data.fov * 100) / 100;
						cam.data.fov = zui_slider(fovHandle, "FoV", 0.3, 2.0, true);
						if (fovHandle.changed) {
							camera_object_build_proj(cam);
						}
					}
				}

				// ui.unindent();
			}
		}
	}
}
