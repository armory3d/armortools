
let tab_objects_material_id: i32 = 0;
let tab_objects_line_counter: i32 = 0;
let tab_objects_current_object: object_t;

function f32_to_string_prec(f: f32): string {
	f = math_round(f);
	return f + "";
}

function tab_objects_import_mesh_done() {
	let mo: mesh_object_t = array_pop(project_paint_objects);
	object_set_parent(mo.base, null);
}

function tab_objects_draw_menu(ui: ui_t) {
	if (ui_menu_button(ui, "Assign Material")) {
		tab_objects_material_id++;

		for (let i: i32 = 0; i < _scene_raw.shader_datas.length; ++i) {
			let sh: shader_data_t = _scene_raw.shader_datas[i];
			if (sh.name == "Material_data") {
				let s: shader_data_t = util_clone_shader_data(sh);
				s.name = "TempMaterial_data" + tab_objects_material_id;
				array_push(_scene_raw.shader_datas, s);
				break;
			}
		}

		for (let i: i32 = 0; i < _scene_raw.material_datas.length; ++i) {
			let mat: material_data_t = _scene_raw.material_datas[i];
			if (mat.name == "Material") {
				let m: material_data_t = util_clone_material_data(mat);
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

function tab_objects_draw_list(ui: ui_t, list_handle: ui_handle_t, current_object: object_t) {
	if (char_at(current_object.name, 0) == ".") {
		return; // Hidden
	}
	let b: bool = false;

	// Highlight every other line
	if (tab_objects_line_counter % 2 == 0) {
		g2_set_color(ui.ops.theme.SEPARATOR_COL);
		g2_fill_rect(0, ui._y, ui._window_w, ui_ELEMENT_H(ui));
		g2_set_color(0xffffffff);
	}

	// Highlight selected line
	if (current_object == context_raw.selected_object) {
		g2_set_color(0xff205d9c);
		g2_fill_rect(0, ui._y, ui._window_w, ui_ELEMENT_H(ui));
		g2_set_color(0xffffffff);
	}

	if (current_object.children.length > 0) {
		let row: f32[] = [1 / 13, 12 / 13];
		ui_row(row);
		let h: ui_handle_t = ui_nest(list_handle, tab_objects_line_counter);
		if (h.init) {
			h.selected = true;
		}
		b = ui_panel(h, "", true, false);
		ui_text(current_object.name);
	}
	else {
		ui._x += 18; // Sign offset

		// Draw line that shows parent relations
		g2_set_color(ui.ops.theme.BUTTON_COL);
		g2_draw_line(ui._x - 10, ui._y + ui_ELEMENT_H(ui) / 2, ui._x, ui._y + ui_ELEMENT_H(ui) / 2);
		g2_set_color(0xffffffff);

		ui_text(current_object.name);
		ui._x -= 18;
	}

	tab_objects_line_counter++;
	// Undo applied offset for row drawing caused by end_element()
	ui._y -= ui_ELEMENT_OFFSET(ui);

	if (ui.is_released) {
		context_raw.selected_object = current_object;
	}

	if (ui.is_hovered && ui.input_released_r) {
		tab_objects_current_object = current_object;
		ui_menu_draw(tab_objects_draw_menu);
	}

	if (b) {
		let current_y: i32 = ui._y;
		for (let i: i32 = 0; i < current_object.children.length; ++i) {
			let child: object_t = current_object.children[i];
			// ui.indent();
			tab_objects_draw_list(ui, list_handle, child);
			// ui.unindent();
		}

		// Draw line that shows parent relations
		g2_set_color(ui.ops.theme.BUTTON_COL);
		g2_draw_line(ui._x + 14, current_y, ui._x + 14, ui._y - ui_ELEMENT_H(ui) / 2);
		g2_set_color(0xffffffff);
	}
}

function tab_objects_draw(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	if (ui_tab(htab, tr("Objects"))) {
		ui_begin_sticky();
		let row: f32[] = [1 / 4];
		ui_row(row);
		if (ui_button("Import")) {
			project_import_mesh(false, tab_objects_import_mesh_done);
		}
		ui_end_sticky();

		let outliner_handle: ui_handle_t = ui_handle(__ID__);
		if (outliner_handle.init) {
			outliner_handle.selected = true;
		}

		if (ui_panel(outliner_handle, "Outliner", true, false)) {
			ui._y -= ui_ELEMENT_OFFSET(ui);

			tab_objects_line_counter = 0;

			for (let i: i32 = 0; i < _scene_root.children.length; ++i) {
				let c: object_t = _scene_root.children[i];
				tab_objects_draw_list(ui, ui_handle(__ID__), c);
			}
		}

		let properties_handle: ui_handle_t = ui_handle(__ID__);
		if (properties_handle.init) {
			properties_handle.selected = true;
		}

		if (ui_panel(properties_handle, "Properties", true, false)) {
			if (context_raw.selected_object != null) {
				let h: ui_handle_t = ui_handle(__ID__);
				h.selected = context_raw.selected_object.visible;
				context_raw.selected_object.visible = ui_check(h, "Visible");

				let t: transform_t = context_raw.selected_object.transform;
				let local_pos: vec4_t = t.loc;
				let scale: vec4_t = t.scale;
				let rot: quat_t = quat_get_euler(t.rot);
				let dim: vec4_t = t.dim;
				rot = vec4_mult(rot, 180 / 3.141592);
				let f: f32 = 0.0;

				let row: f32[] = [1 / 4, 1 / 4, 1 / 4, 1 / 4];
				ui_row(row);
				ui_text("Loc");

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(local_pos.x);
				f = parse_float(ui_text_input(h, "X"));
				if (h.changed) {
					local_pos.x = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(local_pos.y);
				f = parse_float(ui_text_input(h, "Y"));
				if (h.changed) {
					local_pos.y = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(local_pos.z);
				f = parse_float(ui_text_input(h, "Z"));
				if (h.changed) {
					local_pos.z = f;
				}

				ui_row(row);
				ui_text("Rotation");

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(rot.x);
				f = parse_float(ui_text_input(h, "X"));
				let changed: bool = false;
				if (h.changed) {
					changed = true;
					rot.x = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(rot.y);
				f = parse_float(ui_text_input(h, "Y"));
				if (h.changed) {
					changed = true;
					rot.y = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(rot.z);
				f = parse_float(ui_text_input(h, "Z"));
				if (h.changed) {
					changed = true;
					rot.z = f;
				}

				if (changed && context_raw.selected_object.name != "Scene") {
					rot = vec4_mult(rot, 3.141592 / 180);
					context_raw.selected_object.transform.rot = quat_from_euler(rot.x, rot.y, rot.z);
					transform_build_matrix(context_raw.selected_object.transform);
					// ///if arm_physics
					// if (rb != null) rb.syncTransform();
					// ///end
				}

				ui_row(row);
				ui_text("Scale");

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(scale.x);
				f = parse_float(ui_text_input(h, "X"));
				if (h.changed) {
					scale.x = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(scale.y);
				f = parse_float(ui_text_input(h, "Y"));
				if (h.changed) {
					scale.y = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(scale.z);
				f = parse_float(ui_text_input(h, "Z"));
				if (h.changed) {
					scale.z = f;
				}

				ui_row(row);
				ui_text("Dimensions");

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(dim.x);
				f = parse_float(ui_text_input(h, "X"));
				if (h.changed) {
					dim.x = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(dim.y);
				f = parse_float(ui_text_input(h, "Y"));
				if (h.changed) {
					dim.y = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string_prec(dim.z);
				f = parse_float(ui_text_input(h, "Z"));
				if (h.changed) {
					dim.z = f;
				}

				context_raw.selected_object.transform.dirty = true;

				if (context_raw.selected_object.name == "Scene") {
					let p: world_data_t = scene_world;

					let p_handle: ui_handle_t = ui_handle(__ID__);
					if (p_handle.init) {
						p_handle.value = p.strength;
					}

					p.strength = ui_slider(p_handle, "Environment", 0.0, 5.0, true);
				}
				else if (context_raw.selected_object.ext_type == "light_object_t") {
					let light: light_object_t = context_raw.selected_object.ext;
					let light_handle: ui_handle_t = ui_handle(__ID__);
					light_handle.value = light.data.strength / 10;
					light.data.strength = ui_slider(light_handle, "Strength", 0.0, 5.0, true) * 10;
				}
				else if (context_raw.selected_object.ext_type == "camera_object_t") {
					let cam: camera_object_t = context_raw.selected_object.ext;
					let fov_handle: ui_handle_t = ui_handle(__ID__);
					fov_handle.value = math_floor(cam.data.fov * 100) / 100;
					cam.data.fov = ui_slider(fov_handle, "FoV", 0.3, 2.0, true);
					if (fov_handle.changed) {
						camera_object_build_proj(cam);
					}
				}
			}
		}
	}
}
