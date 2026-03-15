
#include "global.h"

void tab_meshes_draw_context_menu() {
	i32            i = _tab_meshes_draw_i;
	mesh_object_t *o = project_paint_objects->buffer[i];

	if (ui_menu_button(tr("Export"), "", ICON_EXPORT)) {
		context_raw->export_mesh_index = i + 1;
		box_export_show_mesh();
	}
	if (project_paint_objects->length > 1 && ui_menu_button(tr("Delete"), "", ICON_DELETE)) {
		array_remove(project_paint_objects, o);
		while (o->base->children->length > 0) {
			object_t *child = o->base->children->buffer[0];
			object_set_parent(child, NULL);
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
		util_mesh_merge(NULL);
		context_raw->ddirty = 2;
	}
	if (ui_menu_button(tr("Duplicate"), "", ICON_DUPLICATE)) {
		sim_duplicate();
	}

	if (config_raw->experimental) {
		tab_meshes_draw_properties(o);
	}
}

void tab_meshes_draw_edit() {
	if (ui_menu_button(tr("Flip Normals"), "", ICON_NONE)) {
		util_mesh_flip_normals();
		context_raw->ddirty = 2;
	}

	if (ui_menu_sub_button(ui_handle(__ID__), tr("Calculate Normals"))) {
		ui_menu_sub_begin(2);
		if (ui_menu_button(tr("Smooth"), "", ICON_NONE)) {
			util_mesh_calc_normals(true);
			context_raw->ddirty = 2;
		}
		if (ui_menu_button(tr("Flat"), "", ICON_NONE)) {
			util_mesh_calc_normals(false);
			context_raw->ddirty = 2;
		}
		ui_menu_sub_end();
	}

	if (ui_menu_button(tr("Geometry to Origin"), "", ICON_NONE)) {
		util_mesh_to_origin();
		context_raw->ddirty = 2;
	}

	if (ui_menu_button(tr("Apply Displacement"), "", ICON_NONE)) {
		util_mesh_apply_displacement(project_layers->buffer[0]->texpaint_pack, 0.1, 1.0);
		util_mesh_calc_normals(false);
		context_raw->ddirty = 2;
	}

	if (ui_menu_sub_button(ui_handle(__ID__), tr("Rotate"))) {
		ui_menu_sub_begin(3);
		if (ui_menu_button(tr("X"), "", ICON_NONE)) {
			util_mesh_swap_axis(1, 2);
			context_raw->ddirty = 2;
		}
		if (ui_menu_button(tr("Y"), "", ICON_NONE)) {
			util_mesh_swap_axis(2, 0);
			context_raw->ddirty = 2;
		}
		if (ui_menu_button(tr("Z"), "", ICON_NONE)) {
			util_mesh_swap_axis(0, 1);
			context_raw->ddirty = 2;
		}
		ui_menu_sub_end();
	}

	if (ui_menu_button(tr("UV Unwrap"), "", ICON_NONE)) {
		char *f = "uv_unwrap.js";
		if (string_array_index_of(config_raw->plugins, f) == -1) {
			config_enable_plugin(f);
		}
		plugin_uv_unwrap_button();
	}

	if (config_raw->experimental && ui_menu_button(tr("Decimate"), "", ICON_NONE)) {
		util_mesh_decimate(0.5);
	}
}

void tab_meshes_draw_append_shape() {
	for (i32 i = 0; i < project_mesh_list->length; ++i) {
		if (ui_menu_button(project_mesh_list->buffer[i], "", ICON_NONE)) {
			tab_meshes_append_shape(project_mesh_list->buffer[i]);
		}
	}
}

void tab_meshes_draw_import() {
	if (ui_menu_button(tr("Replace Existing"), any_map_get(config_keymap, "file_import_assets"), ICON_NONE)) {
		project_import_mesh(true, NULL);
	}
	if (ui_menu_button(tr("Append File"), "", ICON_NONE)) {
		project_append_mesh();
	}

	if (config_raw->experimental) {
		project_fetch_default_meshes();
		if (ui_menu_button(tr("Append Shape"), "", ICON_NONE)) {
			ui_menu_draw(&tab_meshes_draw_append_shape, -1, -1);
		}
	}
}

void tab_meshes_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Meshes"), false, -1, false) && ui->_window_h > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        -100,
		        -100,
		    },
		    2);
		ui_row(row);

		if (ui_icon_button(tr("Import"), ICON_IMPORT, UI_ALIGN_CENTER)) {
			ui_menu_draw(&tab_meshes_draw_import, -1, -1);
		}
		if (ui->is_hovered)
			ui_tooltip(tr("Import mesh file"));

		if (ui_icon_button(tr("Edit"), ICON_EDIT, UI_ALIGN_CENTER)) {
			ui_menu_draw(&tab_meshes_draw_edit, -1, -1);
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

				ui_menu_draw(&tab_meshes_draw_context_menu, -1, -1);
			}
			if (h->changed) {
				mesh_object_t_array_t *visibles = any_array_create_from_raw((void *[]){}, 0);
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

void tab_meshes_draw_properties(mesh_object_t *o) {
	context_raw->selected_object = o->base;
	ui_handle_t *h               = ui_handle(__ID__);
	// h->b = context_raw->selected_object->visible;
	// context_raw->selected_object->visible = ui_check(h, "Visible", "");
	// if (h->changed) {
	// Rebuild full vb for path-tracing
	// util_mesh_merge(NULL);
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
		if (pb != NULL) {
			physics_body_sync_transform(pb);
		}
	}

	physics_body_t   *pb          = any_imap_get(physics_body_object_map, context_raw->selected_object->uid);
	ui_handle_t      *hshape      = ui_handle(__ID__);
	string_t_array_t *shape_combo = any_array_create_from_raw(
	    (void *[]){
	        tr("None"),
	        tr("Box"),
	        tr("Sphere"),
	        tr("Convex Hull"),
	        tr("Terrain"),
	        tr("Mesh"),
	    },
	    6);
	hshape->i = pb != NULL ? pb->shape + 1 : 0;
	ui_combo(hshape, shape_combo, tr("Shape"), true, UI_ALIGN_LEFT, true);

	ui_handle_t *hdynamic = ui_handle(__ID__);
	hdynamic->b           = pb != NULL ? pb->mass > 0 : false;
	ui_check(hdynamic, "Dynamic", "");

	if (hshape->changed || hdynamic->changed) {
		sim_remove_body(context_raw->selected_object->uid);
		if (hshape->i > 0) {
			sim_add_body(context_raw->selected_object, hshape->i - 1, hdynamic->b ? 1.0 : 0.0);
		}
	}

	ui_text("Script", UI_ALIGN_LEFT, ui->ops->theme->SEPARATOR_COL);

	char *script = any_map_get(sim_object_script_map, context_raw->selected_object);
	if (script == NULL) {
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
	ui_text_area_coloring = NULL;
	ui_set_font(ui, _font);
	ui->font_size = _font_size;

	script = string_copy(hscript->text);
	any_map_set(sim_object_script_map, context_raw->selected_object, script);

	if (ui->changed || ui->is_typing) {
		ui_menu_keep_open = true;
	}
}

void tab_meshes_append_shape(char *mesh_name) {
	scene_t     *scene_raw = NULL;
	mesh_data_t *raw       = NULL;
	if (string_equals(mesh_name, "sphere")) {
		raw_mesh_t *mesh = geom_make_uv_sphere(1, 128, 64, true, 1.0);
		raw              = import_mesh_raw_mesh(mesh);
	}
	else if (string_equals(mesh_name, "plane")) {
		raw_mesh_t *mesh = geom_make_plane(1, 1, 4, 4, 1.0);
		raw              = import_mesh_raw_mesh(mesh);
	}
	else {
		buffer_t *b = iron_load_blob(string("%smeshes/%s.arm", data_path(), mesh_name));
		scene_raw   = armpack_decode(b);
		raw         = scene_raw->mesh_datas->buffer[0];
	}

	util_mesh_pack_uvs(raw->vertex_arrays->buffer[2]->values);
	mesh_data_t *md   = mesh_data_create(raw);
	md->_->handle     = md->name;
	mesh_object_t *mo = scene_add_mesh_object(md, project_paint_objects->buffer[0]->material, NULL);
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
