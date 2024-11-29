
let tab_scene_line_counter: i32 = 0;

function tab_scene_import_mesh_done() {
	let mo: mesh_object_t = project_paint_objects[project_paint_objects.length - 1];
	object_set_parent(mo.base, null);
}

function tab_scene_draw_list(ui: ui_t, list_handle: ui_handle_t, current_object: object_t) {
	if (char_at(current_object.name, 0) == ".") {
		return; // Hidden
	}
	let b: bool = false;

	// Highlight every other line
	if (tab_scene_line_counter % 2 == 0) {
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
		let h: ui_handle_t = ui_nest(list_handle, tab_scene_line_counter);
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

	tab_scene_line_counter++;
	// Undo applied offset for row drawing caused by end_element()
	ui._y -= ui_ELEMENT_OFFSET(ui);

	if (ui.is_released) {
		context_raw.selected_object = current_object;
	}

	if (b) {
		let current_y: i32 = ui._y;
		for (let i: i32 = 0; i < current_object.children.length; ++i) {
			let child: object_t = current_object.children[i];
			ui._x += 8;
			tab_scene_draw_list(ui, list_handle, child);
			ui._x -= 8;
		}

		// Draw line that shows parent relations
		g2_set_color(ui.ops.theme.BUTTON_COL);
		g2_draw_line(ui._x + 14, current_y, ui._x + 14, ui._y - ui_ELEMENT_H(ui) / 2);
		g2_set_color(0xffffffff);
	}
}

function tab_scene_draw(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	if (ui_tab(htab, tr("Scene"))) {
		ui_begin_sticky();
		let row: f32[] = [1 / 4];
		ui_row(row);
		if (ui_button("Import")) {
			project_import_mesh(false, tab_scene_import_mesh_done);
		}
		ui_end_sticky();

		let outliner_handle: ui_handle_t = ui_handle(__ID__);
		if (outliner_handle.init) {
			outliner_handle.selected = true;
		}

		if (ui_panel(outliner_handle, "Outliner", true, false)) {
			ui._y -= ui_ELEMENT_OFFSET(ui);

			tab_scene_line_counter = 0;

			for (let i: i32 = 0; i < _scene_root.children.length; ++i) {
				let c: object_t = _scene_root.children[i];
				tab_scene_draw_list(ui, ui_handle(__ID__), c);
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
				let rot: vec4_t = quat_get_euler(t.rot);
				rot = vec4_mult(rot, 180 / 3.141592);
				let f: f32 = 0.0;
				let changed: bool = false;

				ui_row4();
				ui_text("Loc");

				h = ui_handle(__ID__);
				h.text = f32_to_string(t.loc.x);
				f = parse_float(ui_text_input(h, "X"));
				if (h.changed) {
					changed = true;
					t.loc.x = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string(t.loc.y);
				f = parse_float(ui_text_input(h, "Y"));
				if (h.changed) {
					changed = true;
					t.loc.y = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string(t.loc.z);
				f = parse_float(ui_text_input(h, "Z"));
				if (h.changed) {
					changed = true;
					t.loc.z = f;
				}

				ui_row4();
				ui_text("Rotation");

				h = ui_handle(__ID__);
				h.text = f32_to_string(rot.x);
				f = parse_float(ui_text_input(h, "X"));
				if (h.changed) {
					changed = true;
					rot.x = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string(rot.y);
				f = parse_float(ui_text_input(h, "Y"));
				if (h.changed) {
					changed = true;
					rot.y = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string(rot.z);
				f = parse_float(ui_text_input(h, "Z"));
				if (h.changed) {
					changed = true;
					rot.z = f;
				}

				ui_row4();
				ui_text("Scale");

				h = ui_handle(__ID__);
				h.text = f32_to_string(t.scale.x);
				f = parse_float(ui_text_input(h, "X"));
				if (h.changed) {
					changed = true;
					t.scale.x = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string(t.scale.y);
				f = parse_float(ui_text_input(h, "Y"));
				if (h.changed) {
					changed = true;
					t.scale.y = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string(t.scale.z);
				f = parse_float(ui_text_input(h, "Z"));
				if (h.changed) {
					changed = true;
					t.scale.z = f;
				}

				ui_row4();
				ui_text("Dimensions");

				h = ui_handle(__ID__);
				h.text = f32_to_string(t.dim.x);
				f = parse_float(ui_text_input(h, "X"));
				if (h.changed) {
					changed = true;
					t.dim.x = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string(t.dim.y);
				f = parse_float(ui_text_input(h, "Y"));
				if (h.changed) {
					changed = true;
					t.dim.y = f;
				}

				h = ui_handle(__ID__);
				h.text = f32_to_string(t.dim.z);
				f = parse_float(ui_text_input(h, "Z"));
				if (h.changed) {
					changed = true;
					t.dim.z = f;
				}

				if (changed) {
					rot = vec4_mult(rot, 3.141592 / 180);
					context_raw.selected_object.transform.rot = quat_from_euler(rot.x, rot.y, rot.z);
					transform_build_matrix(context_raw.selected_object.transform);
					transform_compute_dim(context_raw.selected_object.transform);
					///if arm_physics
					let pb: physics_body_t = map_get(physics_body_object_map, context_raw.selected_object.uid);
					if (pb != null) {
						physics_body_sync_transform(pb);
					}
					///end
				}

				if (ui_button("Sphere Dynamic")) {
					sim_add(context_raw.selected_object, physics_shape_t.SPHERE, 1.0);
				}

				if (ui_button("Box Dynamic")) {
					sim_add(context_raw.selected_object, physics_shape_t.BOX, 1.0);
				}

				if (ui_button("Box Static")) {
					sim_add(context_raw.selected_object, physics_shape_t.BOX, 0.0);
				}
			}
		}

		if (ui_button("Play")) {
			sim_play();
		}
		if (ui_button("Stop")) {
			sim_stop();
		}
	}
}
