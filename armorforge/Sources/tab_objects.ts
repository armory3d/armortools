
let tab_objects_material_id: i32 = 0;

function tab_objects_roundfp(f: f32, precision: i32 = 2): f32 {
	f *= math_pow(10, precision);
	return math_round(f) / math_pow(10, precision);
}

function tab_objects_draw(htab: zui_handle_t) {
	let ui = ui_base_ui;
	if (zui_tab(htab, tr("Objects"))) {
		zui_begin_sticky();
		zui_row([1 / 4]);
		if (zui_button("Import")) {
			project_import_mesh(false, function () {
				object_set_parent(project_paint_objects.pop().base, null);
			});
		}
		zui_end_sticky();

		if (zui_panel(zui_handle("tabobjects_0", {selected: true}), "Outliner")) {
			// ui.indent();
			ui._y -= zui_ELEMENT_OFFSET(ui);

			let line_counter = 0;
			let draw_list = function (list_handle: zui_handle_t, current_object: object_t) {
				if (char_at(current_object.name, 0) == ".") {
					return; // Hidden
				}
				let b: bool = false;

				// Highlight every other line
				if (line_counter % 2 == 0) {
					g2_set_color(ui.t.SEPARATOR_COL);
					g2_fill_rect(0, ui._y, ui._window_w, zui_ELEMENT_H(ui));
					g2_set_color(0xffffffff);
				}

				// Highlight selected line
				if (current_object == context_context_raw.selected_object) {
					g2_set_color(0xff205d9c);
					g2_fill_rect(0, ui._y, ui._window_w, zui_ELEMENT_H(ui));
					g2_set_color(0xffffffff);
				}

				if (current_object.children.length > 0) {
					zui_row([1 / 13, 12 / 13]);
					b = zui_panel(zui_nest(list_handle, line_counter, {selected: true}), "", true, false, false);
					zui_text(current_object.name);
				}
				else {
					ui._x += 18; // Sign offset

					// Draw line that shows parent relations
					g2_set_color(ui.t.ACCENT_COL);
					g2_draw_line(ui._x - 10, ui._y + zui_ELEMENT_H(ui) / 2, ui._x, ui._y + zui_ELEMENT_H(ui) / 2);
					g2_set_color(0xffffffff);

					zui_text(current_object.name);
					ui._x -= 18;
				}

				line_counter++;
				// Undo applied offset for row drawing caused by endElement() in Zui.hx
				ui._y -= zui_ELEMENT_OFFSET(ui);

				if (ui.is_released) {
					context_context_raw.selected_object = current_object;
				}

				if (ui.is_hovered && ui.input_released_r) {
					ui_menu_draw(function (ui: zui_t) {
						if (ui_menu_button(ui, "Assign Material")) {
							tab_objects_material_id++;

							for (let sh of _scene_raw.shader_datas) {
								if (sh.name == "Material_data") {
									let s: shader_data_t = json_parse(json_stringify(sh));
									s.name = "TempMaterial_data" + tab_objects_material_id;
									array_push(_scene_raw.shader_datas, s);
									break;
								}
							}

							for (let mat of _scene_raw.material_datas) {
								if (mat.name == "Material") {
									let m: material_data_t = json_parse(json_stringify(mat));
									m.name = "TempMaterial" + tab_objects_material_id;
									m.shader = "TempMaterial_data" + tab_objects_material_id;
									array_push(_scene_raw.material_datas, m);
									break;
								}
							}

							let md: material_data_t = data_get_material("Scene", "TempMaterial" + tab_objects_material_id);
							let mo: mesh_object_t = current_object.ext;
							mo.materials = [md];
							make_material_parse_mesh_preview_material(md);
						}
					}, 1);
				}

				if (b) {
					let current_y = ui._y;
					for (let child of current_object.children) {
						// ui.indent();
						draw_list(list_handle, child);
						// ui.unindent();
					}

					// Draw line that shows parent relations
					g2_set_color(ui.t.ACCENT_COL);
					g2_draw_line(ui._x + 14, current_y, ui._x + 14, ui._y - zui_ELEMENT_H(ui) / 2);
					g2_set_color(0xffffffff);
				}
			}
			for (let c of _scene_root.children) {
				draw_list(zui_handle("tabobjects_1"), c);
			}

			// ui.unindent();
		}

		if (zui_panel(zui_handle("tabobjects_2", {selected: true}), 'Properties')) {
			// ui.indent();

			if (context_context_raw.selected_object != null) {
				let h = zui_handle("tabobjects_3");
				h.selected = context_context_raw.selected_object.visible;
				context_context_raw.selected_object.visible = zui_check(h, "Visible");

				let t = context_context_raw.selected_object.transform;
				let local_pos = t.loc;
				let scale = t.scale;
				let rot = quat_get_euler(t.rot);
				let dim = t.dim;
				vec4_mult(rot, 180 / 3.141592);
				let f: f32 = 0.0;

				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
				zui_text("Loc");

				h = zui_handle("tabobjects_4");
				h.text = roundfp(local_pos.x) + "";
				f = parseFloat(zui_text_input(h, "X"));
				if (h.changed) {
					local_pos.x = f;
				}

				h = zui_handle("tabobjects_5");
				h.text = roundfp(local_pos.y) + "";
				f = parseFloat(zui_text_input(h, "Y"));
				if (h.changed) {
					local_pos.y = f;
				}

				h = zui_handle("tabobjects_6");
				h.text = roundfp(local_pos.z) + "";
				f = parseFloat(zui_text_input(h, "Z"));
				if (h.changed) {
					local_pos.z = f;
				}

				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
				zui_text("Rotation");

				h = zui_handle("tabobjects_7");
				h.text = roundfp(rot.x) + "";
				f = parseFloat(zui_text_input(h, "X"));
				let changed = false;
				if (h.changed) {
					changed = true;
					rot.x = f;
				}

				h = zui_handle("tabobjects_8");
				h.text = roundfp(rot.y) + "";
				f = parseFloat(zui_text_input(h, "Y"));
				if (h.changed) {
					changed = true;
					rot.y = f;
				}

				h = zui_handle("tabobjects_9");
				h.text = roundfp(rot.z) + "";
				f = parseFloat(zui_text_input(h, "Z"));
				if (h.changed) {
					changed = true;
					rot.z = f;
				}

				if (changed && context_context_raw.selected_object.name != "Scene") {
					vec4_mult(rot, 3.141592 / 180);
					quat_from_euler(context_context_raw.selected_object.transform.rot, rot.x, rot.y, rot.z);
					transform_build_matrix(context_context_raw.selected_object.transform);
					// ///if arm_physics
					// if (rb != null) rb.syncTransform();
					// ///end
				}

				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
				zui_text("Scale");

				h = zui_handle("tabobjects_10");
				h.text = roundfp(scale.x) + "";
				f = parseFloat(zui_text_input(h, "X"));
				if (h.changed) {
					scale.x = f;
				}

				h = zui_handle("tabobjects_11");
				h.text = roundfp(scale.y) + "";
				f = parseFloat(zui_text_input(h, "Y"));
				if (h.changed) {
					scale.y = f;
				}

				h = zui_handle("tabobjects_12");
				h.text = roundfp(scale.z) + "";
				f = parseFloat(zui_text_input(h, "Z"));
				if (h.changed) {
					scale.z = f;
				}

				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
				zui_text("Dimensions");

				h = zui_handle("tabobjects_13");
				h.text = roundfp(dim.x) + "";
				f = parseFloat(zui_text_input(h, "X"));
				if (h.changed) {
					dim.x = f;
				}

				h = zui_handle("tabobjects_14");
				h.text = roundfp(dim.y) + "";
				f = parseFloat(zui_text_input(h, "Y"));
				if (h.changed) {
					dim.y = f;
				}

				h = zui_handle("tabobjects_15");
				h.text = roundfp(dim.z) + "";
				f = parseFloat(zui_text_input(h, "Z"));
				if (h.changed) {
					dim.z = f;
				}

				context_context_raw.selected_object.transform.dirty = true;

				if (context_context_raw.selected_object.name == "Scene") {
					let p = scene_world;
					p.strength = zui_slider(zui_handle("tabobjects_16", {value: p.strength}), "Environment", 0.0, 5.0, true);
				}
				else if (context_context_raw.selected_object.ext_type == "light_object_t") {
					let light = context_context_raw.selected_object.ext;
					let light_handle = zui_handle("tabobjects_17");
					light_handle.value = light.data.strength / 10;
					light.data.strength = zui_slider(light_handle, "Strength", 0.0, 5.0, true) * 10;
				}
				else if (context_context_raw.selected_object.ext_type == "camera_object_t") {
					let cam = context_context_raw.selected_object.ext;
					let fov_handle = zui_handle("tabobjects_18");
					fov_handle.value = math_floor(cam.data.fov * 100) / 100;
					cam.data.fov = zui_slider(fov_handle, "FoV", 0.3, 2.0, true);
					if (fov_handle.changed) {
						camera_object_build_proj(cam);
					}
				}
			}

			// ui.unindent();
		}
	}
}
