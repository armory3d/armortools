
let tab_scene_line_counter: i32 = 0;
let tab_scene_new_meshes: string[] = null;
let _tab_scene_paint_object_length: i32 = 1;

function tab_scene_select_object(mo: mesh_object_t) {
	if (mo == null) {
		return;
	}

	context_raw.selected_object = mo.base;

	if (mo.base.ext_type != "mesh_object_t") {
		return;
	}

	context_raw.paint_object = mo;
	if (context_raw.merged_object != null) {
		context_raw.merged_object.base.visible = false;
	}
	context_select_paint_object(mo);
}

function tab_scene_import_mesh_done() {
	let count: i32 = project_paint_objects.length - _tab_scene_paint_object_length;
	_tab_scene_paint_object_length = project_paint_objects.length;

	for (let i: i32 = 0; i < count; ++i) {
		let mo: mesh_object_t = project_paint_objects[project_paint_objects.length - 1 - i];
		object_set_parent(mo.base, null);
		tab_scene_select_object(mo);
	}

	app_notify_on_next_frame(function() {
		util_mesh_merge();
	});
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
		tab_scene_select_object(current_object.ext);
	}

	if (ui.is_hovered && ui.input_released_r) {
		tab_scene_select_object(current_object.ext);

		ui_menu_draw(function (ui: ui_t) {
			if (ui_menu_button(ui, tr("Duplicate"))) {
				sim_duplicate();
			}
			if (ui_menu_button(ui, tr("Delete"))) {
				sim_delete();
			}
		});
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

function tab_scene_new_object(mesh_name: string) {
	let blob: buffer_t = iron_load_blob(data_path() + "meshes/" + mesh_name);
	let raw: scene_t = armpack_decode(blob);
	util_mesh_ext_pack_uvs(raw.mesh_datas[0].vertex_arrays[2].values);
	let md: mesh_data_t = mesh_data_create(raw.mesh_datas[0]);
	md._.handle = md.name;
	let mo: mesh_object_t = scene_add_mesh_object(md, project_paint_objects[0].materials);
	mo.base.name = md.name;
	let o: obj_t = {};
	o._ = { _gc: raw };
	mo.base.raw = o;
	map_set(data_cached_meshes, md._.handle, md);
	array_push(project_paint_objects, mo);
	tab_scene_import_mesh_done();
	app_notify_on_next_frame(function(mo: mesh_object_t) {
		tab_scene_select_object(mo);
	}, mo);
}

function tab_scene_new_menu(ui: ui_t) {
	for (let i: i32 = 0; i < tab_scene_new_meshes.length; ++i) {
		let mesh_name: string = tab_scene_new_meshes[i];
		if (ui_menu_button(ui, mesh_name)) {
			tab_scene_new_object(mesh_name);
		}
	}
}

function tab_scene_draw(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	if (ui_tab(htab, tr("Scene"))) {

		ui_begin_sticky();

		let row: f32[] = [1 / 4, 1 / 4];
		ui_row(row);

		if (ui_button("New")) {
			if (tab_scene_new_meshes == null) {
				tab_scene_new_meshes = file_read_directory(path_data() + path_sep + "meshes");
			}

			ui_menu_draw(tab_scene_new_menu);
		}

		if (ui_button("Import")) {
			project_import_mesh(false, tab_scene_import_mesh_done);
		}

		ui_end_sticky();

		{
			ui._y -= ui_ELEMENT_OFFSET(ui);

			tab_scene_line_counter = 0;

			let scene: object_t = _scene_root.children[0];

			for (let i: i32 = 0; i < scene.children.length; ++i) {
				let c: object_t = scene.children[i];
				tab_scene_draw_list(ui, ui_handle(__ID__), c);
			}

			// Select object with arrow keys
			if (ui.is_key_pressed && ui.key_code == key_code_t.DOWN) {
				let i: i32 = array_index_of(project_paint_objects, context_raw.selected_object.ext);
				if (i < project_paint_objects.length - 1) {
					tab_scene_select_object(project_paint_objects[i + 1]);
				}
			}
			if (ui.is_key_pressed && ui.key_code == key_code_t.UP) {
				let i: i32 = array_index_of(project_paint_objects, context_raw.selected_object.ext);
				if (i > 1) {
					tab_scene_select_object(project_paint_objects[i - 1]);
				}
			}
		}
	}
}
