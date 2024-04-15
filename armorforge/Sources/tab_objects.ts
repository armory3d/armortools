
let tab_objects_material_id: i32 = 0;
let tab_objects_line_counter: i32 = 0;
let tab_objects_current_object: object_t;

function tab_objects_roundfp(f: f32, precision: i32 = 2): f32 {
	f *= math_pow(10, precision);
	return math_round(f) / math_pow(10, precision);
}

function tab_objects_import_mesh_done() {
	object_set_parent(project_paint_objects.pop().base, null);
}

function tab_objects_draw_menu(ui: zui_t) {
	if (ui_menu_button(ui, "Assign Material")) {
		tab_objects_material_id++;

		for (let i: i32 = 0; i < _scene_raw.shader_datas.length; ++i) {
			let sh: shader_data_t = _scene_raw.shader_datas[i];
			if (sh.name == "Material_data") {
				let s: shader_data_t = json_parse(json_stringify(sh));
				s.name = "TempMaterial_data" + tab_objects_material_id;
				array_push(_scene_raw.shader_datas, s);
				break;
			}
		}

		for (let i: i32 = 0; i < _scene_raw.material_datas.length; ++i) {
			let mat: material_data_t = _scene_raw.material_datas[i];
			if (mat.name == "Material") {
				let m: material_data_t = json_parse(json_stringify(mat));
				m.name = "TempMaterial" + tab_objects_material_id;
				m.shader = "TempMaterial_data" + tab_objects_material_id;
				array_push(_scene_raw.material_datas, m);
				break;
			}
		}

		let md: material_data_t = data_get_material("Scene", "TempMaterial" + tab_objects_material_id);
		let mo: mesh_object_t = tab_objects_current_object.ext;
		mo.materials = [md];
		make_material_parse_mesh_preview_material(md);
	}
}

function tab_objects_draw_list(ui: zui_t, list_handle: zui_handle_t, current_object: object_t) {
	if (char_at(current_object.name, 0) == ".") {
		return; // Hidden
	}
	let b: bool = false;

	// Highlight every other line
	if (tab_objects_line_counter % 2 == 0) {
		g2_set_color(ui.ops.theme.SEPARATOR_COL);
		g2_fill_rect(0, ui._y, ui._window_w, zui_ELEMENT_H(ui));
		g2_set_color(0xffffffff);
	}

	// Highlight selected line
	if (current_object == context_raw.selected_object) {
		g2_set_color(0xff205d9c);
		g2_fill_rect(0, ui._y, ui._window_w, zui_ELEMENT_H(ui));
		g2_set_color(0xffffffff);
	}

	if (current_object.children.length > 0) {
		zui_row([1 / 13, 12 / 13]);
		b = zui_panel(zui_nest(list_handle, tab_objects_line_counter, {selected: true}), "", true, false, false);
		zui_text(current_object.name);
	}
	else {
		ui._x += 18; // Sign offset

		// Draw line that shows parent relations
		g2_set_color(ui.ops.theme.ACCENT_COL);
		g2_draw_line(ui._x - 10, ui._y + zui_ELEMENT_H(ui) / 2, ui._x, ui._y + zui_ELEMENT_H(ui) / 2);
		g2_set_color(0xffffffff);

		zui_text(current_object.name);
		ui._x -= 18;
	}

	tab_objects_line_counter++;
	// Undo applied offset for row drawing caused by end_element()
	ui._y -= zui_ELEMENT_OFFSET(ui);

	if (ui.is_released) {
		context_raw.selected_object = current_object;
	}

	if (ui.is_hovered && ui.input_released_r) {
		tab_objects_current_object = current_object;
		ui_menu_draw(tab_objects_draw_menu, 1);
	}

	if (b) {
		let current_y = ui._y;
		for (let i: i32 = 0; i < current_object.children.length; ++i) {
			let child: object_t = current_object.children[i];
			// ui.indent();
			tab_objects_draw_list(ui, list_handle, child);
			// ui.unindent();
		}

		// Draw line that shows parent relations
		g2_set_color(ui.ops.theme.ACCENT_COL);
		g2_draw_line(ui._x + 14, current_y, ui._x + 14, ui._y - zui_ELEMENT_H(ui) / 2);
		g2_set_color(0xffffffff);
	}
}

function tab_objects_draw(htab: zui_handle_t) {
	let ui = ui_base_ui;
	if (zui_tab(htab, tr("Objects"))) {
		zui_begin_sticky();
		zui_row([1 / 4]);
		if (zui_button("Import")) {
			project_import_mesh(false, tab_objects_import_mesh_done);
		}
		zui_end_sticky();

		if (zui_panel(zui_handle(__ID__, {selected: true}), "Outliner")) {
			// ui.indent();
			ui._y -= zui_ELEMENT_OFFSET(ui);

			tab_objects_line_counter = 0;

			for (let i: i32 = 0; i < _scene_root.children.length; ++i) {
				let c: object_t = _scene_root.children[i];
				tab_objects_draw_list(ui, zui_handle(__ID__), c);
			}

			// ui.unindent();
		}

		if (zui_panel(zui_handle(__ID__, {selected: true}), "Properties")) {
			// ui.indent();

			if (context_raw.selected_object != null) {
				let h = zui_handle(__ID__);
				h.selected = context_raw.selected_object.visible;
				context_raw.selected_object.visible = zui_check(h, "Visible");

				let t = context_raw.selected_object.transform;
				let local_pos = t.loc;
				let scale = t.scale;
				let rot = quat_get_euler(t.rot);
				let dim = t.dim;
				vec4_mult(rot, 180 / 3.141592);
				let f: f32 = 0.0;

				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
				zui_text("Loc");

				h = zui_handle(__ID__);
				h.text = math_round(local_pos.x) + "";
				f = parse_float(zui_text_input(h, "X"));
				if (h.changed) {
					local_pos.x = f;
				}

				h = zui_handle(__ID__);
				h.text = math_round(local_pos.y) + "";
				f = parse_float(zui_text_input(h, "Y"));
				if (h.changed) {
					local_pos.y = f;
				}

				h = zui_handle(__ID__);
				h.text = math_round(local_pos.z) + "";
				f = parse_float(zui_text_input(h, "Z"));
				if (h.changed) {
					local_pos.z = f;
				}

				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
				zui_text("Rotation");

				h = zui_handle(__ID__);
				h.text = math_round(rot.x) + "";
				f = parse_float(zui_text_input(h, "X"));
				let changed = false;
				if (h.changed) {
					changed = true;
					rot.x = f;
				}

				h = zui_handle(__ID__);
				h.text = math_round(rot.y) + "";
				f = parse_float(zui_text_input(h, "Y"));
				if (h.changed) {
					changed = true;
					rot.y = f;
				}

				h = zui_handle(__ID__);
				h.text = math_round(rot.z) + "";
				f = parse_float(zui_text_input(h, "Z"));
				if (h.changed) {
					changed = true;
					rot.z = f;
				}

				if (changed && context_raw.selected_object.name != "Scene") {
					vec4_mult(rot, 3.141592 / 180);
					quat_from_euler(context_raw.selected_object.transform.rot, rot.x, rot.y, rot.z);
					transform_build_matrix(context_raw.selected_object.transform);
					// ///if arm_physics
					// if (rb != null) rb.syncTransform();
					// ///end
				}

				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
				zui_text("Scale");

				h = zui_handle(__ID__);
				h.text = math_round(scale.x) + "";
				f = parse_float(zui_text_input(h, "X"));
				if (h.changed) {
					scale.x = f;
				}

				h = zui_handle(__ID__);
				h.text = math_round(scale.y) + "";
				f = parse_float(zui_text_input(h, "Y"));
				if (h.changed) {
					scale.y = f;
				}

				h = zui_handle(__ID__);
				h.text = math_round(scale.z) + "";
				f = parse_float(zui_text_input(h, "Z"));
				if (h.changed) {
					scale.z = f;
				}

				zui_row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
				zui_text("Dimensions");

				h = zui_handle(__ID__);
				h.text = math_round(dim.x) + "";
				f = parse_float(zui_text_input(h, "X"));
				if (h.changed) {
					dim.x = f;
				}

				h = zui_handle(__ID__);
				h.text = math_round(dim.y) + "";
				f = parse_float(zui_text_input(h, "Y"));
				if (h.changed) {
					dim.y = f;
				}

				h = zui_handle(__ID__);
				h.text = math_round(dim.z) + "";
				f = parse_float(zui_text_input(h, "Z"));
				if (h.changed) {
					dim.z = f;
				}

				context_raw.selected_object.transform.dirty = true;

				if (context_raw.selected_object.name == "Scene") {
					let p = scene_world;
					p.strength = zui_slider(zui_handle(__ID__, {value: p.strength}), "Environment", 0.0, 5.0, true);
				}
				else if (context_raw.selected_object.ext_type == "light_object_t") {
					let light = context_raw.selected_object.ext;
					let light_handle = zui_handle(__ID__);
					light_handle.value = light.data.strength / 10;
					light.data.strength = zui_slider(light_handle, "Strength", 0.0, 5.0, true) * 10;
				}
				else if (context_raw.selected_object.ext_type == "camera_object_t") {
					let cam = context_raw.selected_object.ext;
					let fov_handle = zui_handle(__ID__);
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
