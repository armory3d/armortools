void tab_meshes_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Meshes", null), false, -1, false) && ui->_window_h > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -100,
		        -100,
		    },
		    2);
		ui_row(row);

		if (ui_icon_button(tr("Import", null), ICON_IMPORT, UI_ALIGN_CENTER)) {
			ui_menu_draw(&tab_meshes_draw_185838, -1, -1);
		}
		if (ui->is_hovered)
			ui_tooltip(tr("Import mesh file", null));

		if (ui_icon_button(tr("Edit", null), ICON_EDIT, UI_ALIGN_CENTER)) {
			ui_menu_draw(&tab_meshes_draw_185981, -1, -1);
		}

		ui_end_sticky();

		for (i32 i = 0; i < project_paint_objects->length; ++i) {

			f32_array_t *row = f32_array_create_from_raw(
			    (f32[]){
			        -30,
			        1.0,
			    },
			    2);
			ui_row(row);
			gpu_texture_t *icons  = resource_get("icons05x.k");
			rect_t        *rect   = resource_tile50(icons, ICON_CUBE);
			i32            icon_h = 25 * UI_SCALE();
			ui_sub_image(icons, ui->ops->theme->LABEL_COL - 0x00333333, icon_h, rect->x / (float)2, rect->y / (float)2, rect->w / (float)2, rect->h / (float)2);

			mesh_object_t *o = project_paint_objects->buffer[i];
			ui_handle_t   *h = ui_handle(__ID__);
			h->b             = o->base->visible;
			o->base->visible = ui_check(h, o->base->name, "");

			if (ui->is_hovered && ui->input_released_r) {
				_tab_meshes_draw_i = i;

				ui_menu_draw(&tab_meshes_draw_186425, -1, -1);
			}
			if (h->changed) {
				mesh_object_t_array_t *visibles = any_array_create_from_raw((any[]){}, 0);
				for (i32 i = 0; i < project_paint_objects->length; ++i) {
					mesh_object_t *p = project_paint_objects->buffer[i];
					if (p->base->visible) {
						any_array_push(visibles, p);
					}
				}
				util_mesh_merge(visibles);
				context_raw->ddirty = 2;
			}
		}
	}
}

void tab_meshes_draw_186425() {
	i32            i = _tab_meshes_draw_i;
	mesh_object_t *o = project_paint_objects->buffer[i];

	if (ui_menu_button(tr("Export", null), "", ICON_EXPORT)) {
		context_raw->export_mesh_index = i + 1;
		box_export_show_mesh();
	}
	if (project_paint_objects->length > 1 && ui_menu_button(tr("Delete", null), "", ICON_DELETE)) {
		array_remove(project_paint_objects, o);
		while (o->base->children->length > 0) {
			object_t *child = o->base->children->buffer[0];
			object_set_parent(child, null);
			if (project_paint_objects->buffer[0]->base != child) {
				object_set_parent(child, project_paint_objects->buffer[0]->base);
			}
			if (o->base->children->length == 0) {
				project_paint_objects->buffer[0]->base->transform->scale = vec4_clone(o->base->transform->scale);
				transform_build_matrix(project_paint_objects->buffer[0]->base->transform);
			}
		}
		data_delete_mesh(o->data->_->handle);
		mesh_object_remove(o);
		context_raw->paint_object = context_main_object();
		util_mesh_merge(null);
		context_raw->ddirty = 2;
	}
	if (ui_menu_button(tr("Duplicate", null), "", ICON_DUPLICATE)) {
		sim_duplicate();
	}

	if (config_raw->experimental) {
		tab_meshes_draw_properties(o);
	}
}

void tab_meshes_draw_185981() {
	if (ui_menu_button(tr("Flip Normals", null), "", ICON_NONE)) {
		util_mesh_flip_normals();
		context_raw->ddirty = 2;
	}

	if (ui_menu_sub_button(ui_handle(__ID__), tr("Calculate Normals", null))) {
		ui_menu_sub_begin(2);
		if (ui_menu_button(tr("Smooth", null), "", ICON_NONE)) {
			util_mesh_calc_normals(true);
			context_raw->ddirty = 2;
		}
		if (ui_menu_button(tr("Flat", null), "", ICON_NONE)) {
			util_mesh_calc_normals(false);
			context_raw->ddirty = 2;
		}
		ui_menu_sub_end();
	}

	if (ui_menu_button(tr("Geometry to Origin", null), "", ICON_NONE)) {
		util_mesh_to_origin();
		context_raw->ddirty = 2;
	}

	if (ui_menu_button(tr("Apply Displacement", null), "", ICON_NONE)) {
		util_mesh_apply_displacement(project_layers->buffer[0]->texpaint_pack, 0.1, 1.0);
		util_mesh_calc_normals(false);
		context_raw->ddirty = 2;
	}

	if (ui_menu_sub_button(ui_handle(__ID__), tr("Rotate", null))) {
		ui_menu_sub_begin(3);
		if (ui_menu_button(tr("X", null), "", ICON_NONE)) {
			util_mesh_swap_axis(1, 2);
			context_raw->ddirty = 2;
		}
		if (ui_menu_button(tr("Y", null), "", ICON_NONE)) {
			util_mesh_swap_axis(2, 0);
			context_raw->ddirty = 2;
		}
		if (ui_menu_button(tr("Z", null), "", ICON_NONE)) {
			util_mesh_swap_axis(0, 1);
			context_raw->ddirty = 2;
		}
		ui_menu_sub_end();
	}

	if (ui_menu_button(tr("UV Unwrap", null), "", ICON_NONE)) {
		string_t *f = "uv_unwrap.js";
		if (char_ptr_array_index_of(config_raw->plugins, f) == -1) {
			config_enable_plugin(f);
		}
		plugin_uv_unwrap_button();
	}

	if (config_raw->experimental && ui_menu_button(tr("Decimate", null), "", ICON_NONE)) {
		util_mesh_decimate(0.5);
	}
}

void tab_meshes_draw_185904() {
	for (i32 i = 0; i < project_mesh_list->length; ++i) {
		if (ui_menu_button(project_mesh_list->buffer[i], "", ICON_NONE)) {
			tab_meshes_append_shape(project_mesh_list->buffer[i]);
		}
	}
}

void tab_meshes_draw_185838() {
	if (ui_menu_button(tr("Replace Existing", null), any_map_get(config_keymap, "file_import_assets"), ICON_NONE)) {
		project_import_mesh(true, null);
	}
	if (ui_menu_button(tr("Append File", null), "", ICON_NONE)) {
		project_append_mesh();
	}

	if (config_raw->experimental) {
		project_fetch_default_meshes();
		if (ui_menu_button(tr("Append Shape", null), "", ICON_NONE)) {
			ui_menu_draw(&tab_meshes_draw_185904, -1, -1);
		}
	}
}

void tab_meshes_draw_properties(mesh_object_t *o) {
	context_raw->selected_object = o->base;
	ui_handle_t *h               = ui_handle(__ID__);
	// h->b = context_raw->selected_object->visible;
	// context_raw->selected_object->visible = ui_check(h, "Visible", "");
	// if (h->changed) {
	// Rebuild full vb for path-tracing
	// util_mesh_merge(null);
	// }

	transform_t *t   = context_raw->selected_object->transform;
	vec4_t       rot = quat_get_euler(t->rot);
	rot              = vec4_mult(rot, 180 / (float)3.141592);
	f32  f           = 0.0;
	bool changed     = false;

	ui_row4();
	ui_text("Loc", UI_ALIGN_LEFT, 0x00000000);

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(t->loc.x));
	f       = parse_float(ui_text_input(h, "X", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed  = true;
		t->loc.x = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(t->loc.y));
	f       = parse_float(ui_text_input(h, "Y", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed  = true;
		t->loc.y = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(t->loc.z));
	f       = parse_float(ui_text_input(h, "Z", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed  = true;
		t->loc.z = f;
	}

	ui_row4();
	ui_text("Rot", UI_ALIGN_LEFT, 0x00000000);

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(rot.x));
	f       = parse_float(ui_text_input(h, "X", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed = true;
		rot.x   = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(rot.y));
	f       = parse_float(ui_text_input(h, "Y", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed = true;
		rot.y   = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(rot.z));
	f       = parse_float(ui_text_input(h, "Z", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed = true;
		rot.z   = f;
	}

	ui_row4();
	ui_text("Scale", UI_ALIGN_LEFT, 0x00000000);

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(t->scale.x));
	f       = parse_float(ui_text_input(h, "X", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed    = true;
		t->scale.x = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(t->scale.y));
	f       = parse_float(ui_text_input(h, "Y", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed    = true;
		t->scale.y = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(t->scale.z));
	f       = parse_float(ui_text_input(h, "Z", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed    = true;
		t->scale.z = f;
	}

	ui_row4();
	ui_text("Dim", UI_ALIGN_LEFT, 0x00000000);

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(t->dim.x));
	f       = parse_float(ui_text_input(h, "X", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed  = true;
		t->dim.x = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(t->dim.y));
	f       = parse_float(ui_text_input(h, "Y", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed  = true;
		t->dim.y = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string(t->dim.z));
	f       = parse_float(ui_text_input(h, "Z", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed  = true;
		t->dim.z = f;
	}

	if (changed) {
		rot                                          = vec4_mult(rot, 3.141592 / (float)180);
		context_raw->selected_object->transform->rot = quat_from_euler(rot.x, rot.y, rot.z);
		transform_build_matrix(context_raw->selected_object->transform);
		transform_compute_dim(context_raw->selected_object->transform);

		physics_body_t *pb = any_imap_get(physics_body_object_map, context_raw->selected_object->uid);
		if (pb != null) {
			physics_body_sync_transform(pb);
		}
	}

	physics_body_t   *pb          = any_imap_get(physics_body_object_map, context_raw->selected_object->uid);
	ui_handle_t      *hshape      = ui_handle(__ID__);
	string_t_array_t *shape_combo = any_array_create_from_raw(
	    (any[]){
	        tr("None", null),
	        tr("Box", null),
	        tr("Sphere", null),
	        tr("Convex Hull", null),
	        tr("Terrain", null),
	        tr("Mesh", null),
	    },
	    6);
	hshape->i = pb != null ? pb->shape + 1 : 0;
	ui_combo(hshape, shape_combo, tr("Shape", null), true, UI_ALIGN_LEFT, true);

	ui_handle_t *hdynamic = ui_handle(__ID__);
	hdynamic->b           = pb != null ? pb->mass > 0 : false;
	ui_check(hdynamic, "Dynamic", "");

	if (hshape->changed || hdynamic->changed) {
		sim_remove_body(context_raw->selected_object->uid);
		if (hshape->i > 0) {
			sim_add_body(context_raw->selected_object, hshape->i - 1, hdynamic->b ? 1.0 : 0.0);
		}
	}

	ui_text("Script", UI_ALIGN_LEFT, ui->ops->theme->SEPARATOR_COL);

	string_t *script = any_map_get(sim_object_script_map, context_raw->selected_object);
	if (script == null) {
		script = "";
	}

	ui_handle_t *hscript = ui_handle(__ID__);
	hscript->text        = string_copy(script);

	draw_font_t *_font      = ui->ops->font;
	i32          _font_size = ui->font_size;
	draw_font_t *fmono      = data_get_font("font_mono.ttf");
	ui_set_font(ui, fmono);
	ui->font_size = math_floor(15 * UI_SCALE());
	gc_unroot(ui_text_area_coloring);
	ui_text_area_coloring = tab_scripts_get_text_coloring();
	gc_root(ui_text_area_coloring);
	ui_text_area(hscript, UI_ALIGN_LEFT, true, "", false);
	gc_unroot(ui_text_area_coloring);
	ui_text_area_coloring = null;
	ui_set_font(ui, _font);
	ui->font_size = _font_size;

	script = string_copy(hscript->text);
	any_map_set(sim_object_script_map, context_raw->selected_object, script);

	if (ui->changed || ui->is_typing) {
		ui_menu_keep_open = true;
	}
}

void tab_meshes_append_shape(string_t *mesh_name) {
	scene_t     *scene_raw = null;
	mesh_data_t *raw       = null;
	if (string_equals(mesh_name, "sphere")) {
		raw_mesh_t *mesh = geom_make_uv_sphere(1, 128, 64, true, 1.0);
		raw              = import_mesh_raw_mesh(mesh);
	}
	else if (string_equals(mesh_name, "plane")) {
		raw_mesh_t *mesh = geom_make_plane(1, 1, 4, 4, 1.0);
		raw              = import_mesh_raw_mesh(mesh);
	}
	else {
		buffer_t *b = iron_load_blob(string_join(string_join(string_join(data_path(), "meshes/"), mesh_name), ".arm"));
		scene_raw   = armpack_decode(b);
		raw         = scene_raw->mesh_datas->buffer[0];
	}

	util_mesh_pack_uvs(raw->vertex_arrays->buffer[2]->values);
	mesh_data_t *md   = mesh_data_create(raw);
	md->_->handle     = md->name;
	mesh_object_t *mo = scene_add_mesh_object(md, project_paint_objects->buffer[0]->material, null);
	mo->base->name    = md->name;
	obj_t *o          = GC_ALLOC_INIT(obj_t, {0});
	o->_              = GC_ALLOC_INIT(obj_runtime_t, {._gc = scene_raw});
	mo->base->raw     = o;
	any_map_set(data_cached_meshes, md->_->handle, md);
	any_array_push(project_paint_objects, mo);

	// tab_scene_import_mesh_done();
	// sys_notify_on_next_frame(function(mo: mesh_object_t) {
	// 	tab_scene_select_object(mo);
	// }, mo);
}

////

void tab_scene_select_object(mesh_object_t *mo) {
	if (mo == null) {
		return;
	}

	context_raw->selected_object = mo->base;

	if (!string_equals(mo->base->ext_type, "mesh_object_t")) {
		return;
	}

	context_raw->paint_object = mo;
	if (context_raw->merged_object != null) {
		context_raw->merged_object->base->visible = false;
	}
	context_select_paint_object(mo);
}

void tab_scene_sort() {
	object_t *scene = _scene_root->children->buffer[0];
	array_sort(scene->children, &tab_scene_sort_187924);
}

i32 tab_scene_sort_187924(any_ptr pa, any_ptr pb) {
	object_t *a = DEREFERENCE(pa);
	object_t *b = DEREFERENCE(pb);
	return strcmp(a->name, b->name);
}

void tab_scene_import_mesh_done() {
	i32 count                      = project_paint_objects->length - _tab_scene_paint_object_length;
	_tab_scene_paint_object_length = project_paint_objects->length;

	for (i32 i = 0; i < count; ++i) {
		mesh_object_t *mo = project_paint_objects->buffer[project_paint_objects->length - 1 - i];
		object_set_parent(mo->base, null);
		tab_scene_select_object(mo);
	}

	sys_notify_on_next_frame(&tab_scene_import_mesh_done_188032, null);
}

void tab_scene_import_mesh_done_188032(any _) {
	util_mesh_merge(null);
	tab_scene_select_object(context_raw->selected_object->ext);
	tab_scene_sort();
}

void tab_scene_draw_list(ui_handle_t *list_handle, object_t *current_object) {
	if (string_equals(char_at(current_object->name, 0), ".")) {
		return; // Hidden
	}

	bool b = false;

	// Highlight every other line
	if (tab_scene_line_counter % 2 == 0) {
		draw_set_color(ui->ops->theme->SEPARATOR_COL);
		draw_filled_rect(0, ui->_y, ui->_window_w, UI_ELEMENT_H());
		draw_set_color(0xffffffff);
	}

	// Highlight selected line
	if (current_object == context_raw->selected_object) {
		draw_set_color(0xff205d9c);
		draw_filled_rect(0, ui->_y, ui->_window_w, UI_ELEMENT_H());
		draw_set_color(0xffffffff);
	}

	if (current_object->children->length > 0) {
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        1 / (float)13,
		        12 / (float)13,
		    },
		    2);
		ui_row(row);
		ui_handle_t *h = ui_nest(list_handle, tab_scene_line_counter);
		if (h->init) {
			h->b = true;
		}
		b = ui_panel(h, "", true, false, false);
		ui_text(current_object->name, UI_ALIGN_LEFT, 0x00000000);
	}
	else {
		ui->_x += 18; // Sign offset

		// Draw line that shows parent relations
		draw_set_color(ui->ops->theme->BUTTON_COL);
		draw_line(ui->_x - 10, ui->_y + UI_ELEMENT_H() / (float)2, ui->_x, ui->_y + UI_ELEMENT_H() / (float)2, 1.0);
		draw_set_color(0xffffffff);

		ui_text(current_object->name, UI_ALIGN_LEFT, 0x00000000);
		ui->_x -= 18;
	}

	tab_scene_line_counter++;
	// Undo applied offset for row drawing caused by end_element()
	ui->_y -= UI_ELEMENT_OFFSET();

	if (ui->is_released) {
		tab_scene_select_object(current_object->ext);
	}

	if (ui->is_hovered && ui->input_released_r) {
		tab_scene_select_object(current_object->ext);

		ui_menu_draw(&tab_scene_draw_list_188308, -1, -1);
	}

	if (b) {
		i32 current_y = ui->_y;
		for (i32 i = 0; i < current_object->children->length; ++i) {
			object_t *child = current_object->children->buffer[i];
			ui->_x += 8;
			tab_scene_draw_list(list_handle, child);
			ui->_x -= 8;
		}

		// Draw line that shows parent relations
		draw_set_color(ui->ops->theme->BUTTON_COL);
		draw_line(ui->_x + 14, current_y, ui->_x + 14, ui->_y - UI_ELEMENT_H() / (float)2, 1.0);
		draw_set_color(0xffffffff);
	}
}

void tab_scene_draw_list_188308() {
	if (ui_menu_button(tr("Duplicate", null), "", ICON_DUPLICATE)) {
		sim_duplicate();
	}
	if (ui_menu_button(tr("Delete", null), "", ICON_DELETE)) {
		sim_delete();
	}
}

void tab_scene_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Scene", null), false, -1, false)) {

		tab_scene_line_counter = 0;

		object_t *scene = _scene_root->children->buffer[0];
		for (i32 i = 0; i < scene->children->length; ++i) {
			object_t *c = scene->children->buffer[i];
			tab_scene_draw_list(ui_handle(__ID__), c);
		}

		// Select object with arrow keys
		if (ui->is_key_pressed && ui->key_code == KEY_CODE_DOWN) {
			i32 i = array_index_of(project_paint_objects, context_raw->selected_object->ext);
			if (i < project_paint_objects->length - 1) {
				tab_scene_select_object(project_paint_objects->buffer[i + 1]);
			}
		}
		if (ui->is_key_pressed && ui->key_code == KEY_CODE_UP) {
			i32 i = array_index_of(project_paint_objects, context_raw->selected_object->ext);
			if (i > 1) {
				tab_scene_select_object(project_paint_objects->buffer[i - 1]);
			}
		}
	}
}
