
#include "global.h"

extern buffer_t *slot_material_default_canvas;

i32        _tab_meshes_draw_i;
any_map_t *tab_meshes_preview_map = NULL;

void tab_meshes_draw_context_menu_delete_next_frame(mesh_object_t *o) {
	data_delete_mesh(o->data->_->handle);
	mesh_object_remove(o);
	g_context->paint_object = context_main_object();
	util_mesh_merge(NULL);
	g_context->ddirty = 2;
}

void tab_meshes_draw_context_menu_delete(mesh_object_t *o) {
	array_remove(project_paint_objects, o);
	while (o->base->children->length > 0) {
		object_t *child = o->base->children->buffer[0];
		object_set_parent(child, NULL);
		if (project_paint_objects->buffer[0]->base != child) {
			object_set_parent(child, project_paint_objects->buffer[0]->base);
		}
		if (o->base->children->length == 0) {
			project_paint_objects->buffer[0]->base->transform->scale = o->base->transform->scale;
			transform_build_matrix(project_paint_objects->buffer[0]->base->transform);
		}
	}
	sys_notify_on_next_frame(tab_meshes_draw_context_menu_delete_next_frame, o);
}

static char *f32_to_string2(float f) {
	return f32_to_string((int)(f * 100) / 100.0);
}

void tab_meshes_draw_context_menu() {
	i32            i = _tab_meshes_draw_i;
	mesh_object_t *o = project_paint_objects->buffer[i];

	if (ui_menu_button(tr("Export"), "", ICON_EXPORT)) {
		g_context->export_mesh_index = i + 1;
		box_export_show_mesh();
	}
	if (project_paint_objects->length > 1 && ui_menu_button(tr("Delete"), "delete", ICON_DELETE)) {
		sys_notify_on_next_frame(tab_meshes_draw_context_menu_delete, o);
	}
	if (ui_menu_button(tr("Duplicate"), "ctrl+d", ICON_DUPLICATE)) {
		sim_duplicate();
	}

	g_context->selected_object = o->base;
	ui_handle_t *h             = ui_handle(__ID__);

	transform_t *t   = g_context->selected_object->transform;
	vec4_t       rot = quat_get_euler(t->rot);
	rot              = vec4_mult(rot, 180 / 3.141592);
	f32  f           = 0.0;
	bool changed     = false;
	ui->changed      = false;

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
	h->text = string_copy(f32_to_string2(rot.x));
	f       = parse_float(ui_text_input(h, "X", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed = true;
		rot.x   = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string2(rot.y));
	f       = parse_float(ui_text_input(h, "Y", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed = true;
		rot.y   = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string2(rot.z));
	f       = parse_float(ui_text_input(h, "Z", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed = true;
		rot.z   = f;
	}

	ui_row4();
	ui_text("Scale", UI_ALIGN_LEFT, 0x00000000);

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string2(t->scale.x));
	f       = parse_float(ui_text_input(h, "X", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed    = true;
		t->scale.x = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string2(t->scale.y));
	f       = parse_float(ui_text_input(h, "Y", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed    = true;
		t->scale.y = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string2(t->scale.z));
	f       = parse_float(ui_text_input(h, "Z", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed    = true;
		t->scale.z = f;
	}

	ui_row4();
	ui_text("Dim", UI_ALIGN_LEFT, 0x00000000);

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string2(t->dim.x));
	f       = parse_float(ui_text_input(h, "X", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed  = true;
		t->dim.x = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string2(t->dim.y));
	f       = parse_float(ui_text_input(h, "Y", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed  = true;
		t->dim.y = f;
	}

	h       = ui_handle(__ID__);
	h->text = string_copy(f32_to_string2(t->dim.z));
	f       = parse_float(ui_text_input(h, "Z", UI_ALIGN_LEFT, true, false));
	if (h->changed) {
		changed  = true;
		t->dim.z = f;
	}

	if (changed) {
		rot                                        = vec4_mult(rot, 3.141592 / 180.0);
		g_context->selected_object->transform->rot = quat_from_euler(rot.x, rot.y, rot.z);
		transform_build_matrix(g_context->selected_object->transform);
		transform_compute_dim(g_context->selected_object->transform);

		// physics_body_t *pb = any_imap_get(physics_body_object_map, g_context->selected_object->uid);
		// if (pb != NULL) {
		// 	physics_body_sync_transform(pb);
		// }
	}

	// physics_body_t *pb          = any_imap_get(physics_body_object_map, g_context->selected_object->uid);
	// ui_handle_t    *hshape      = ui_handle(__ID__);
	// string_array_t *shape_combo = any_array_create_from_raw(
	//     (void *[]){
	//         tr("None"),
	//         tr("Box"),
	//         tr("Sphere"),
	//         tr("Convex Hull"),
	//         tr("Terrain"),
	//         tr("Mesh"),
	//     },
	//     6);
	// hshape->i = pb != NULL ? pb->shape + 1 : 0;
	// ui_combo(hshape, shape_combo, tr("Shape"), true, UI_ALIGN_LEFT, true);

	// ui_handle_t *hdynamic = ui_handle(__ID__);
	// hdynamic->b           = pb != NULL ? pb->mass > 0 : false;
	// ui_check(hdynamic, "Dynamic", "");

	// if (hshape->changed || hdynamic->changed) {
	// 	sim_remove_body(g_context->selected_object->uid);
	// 	if (hshape->i > 0) {
	// 		sim_add_body(g_context->selected_object, hshape->i - 1, hdynamic->b ? 1.0 : 0.0);
	// 	}
	// }

	// ui_text("Script", UI_ALIGN_LEFT, ui->ops->theme->SEPARATOR_COL);

	// char *script = any_map_get(sim_object_script_map, g_context->selected_object);
	// if (script == NULL) {
	// 	script = "";
	// }

	// ui_handle_t *hscript = ui_handle(__ID__);
	// hscript->text        = string_copy(script);

	// draw_font_t *_font      = ui->ops->font;
	// i32          _font_size = ui->font_size;
	// draw_font_t *fmono      = data_get_font("font_mono.ttf");
	// ui_set_font(ui, fmono);
	// ui->font_size = math_floor(15 * UI_SCALE());
	// gc_unroot(ui_text_area_coloring);
	// ui_text_area_coloring = tab_scripts_get_text_coloring();
	// gc_root(ui_text_area_coloring);
	// ui_text_area(hscript, UI_ALIGN_LEFT, true, "", false);
	// gc_unroot(ui_text_area_coloring);
	// ui_text_area_coloring = NULL;
	// ui_set_font(ui, _font);
	// ui->font_size = _font_size;

	// script = string_copy(hscript->text);
	// any_map_set(sim_object_script_map, g_context->selected_object, script);

	f32_array_t *row = f32_array_create_from_raw(
	    (f32[]){
	        1 / 4.0,
	        3 / 4.0,
	    },
	    2);
	ui_row(row);
	ui_text("Name", UI_ALIGN_LEFT, 0x00000000);
	h             = ui_handle(__ID__);
	h->text       = string_copy(o->base->name);
	o->base->name = string_copy(ui_text_input(h, "", UI_ALIGN_LEFT, true, false));

	if (ui->changed || ui->is_typing) {
		ui_menu_keep_open = true;
	}
}

void tab_meshes_draw_edit() {
	if (ui_menu_button(tr("Flip Normals"), "", ICON_NONE)) {
		util_mesh_flip_normals();
		g_context->ddirty = 2;
	}

	if (ui_menu_sub_button(ui_handle(__ID__), tr("Calculate Normals"))) {
		ui_menu_sub_begin(2);
		if (ui_menu_button(tr("Smooth"), "", ICON_NONE)) {
			util_mesh_calc_normals(true);
			g_context->ddirty = 2;
		}
		if (ui_menu_button(tr("Flat"), "", ICON_NONE)) {
			util_mesh_calc_normals(false);
			g_context->ddirty = 2;
		}
		ui_menu_sub_end();
	}

	if (ui_menu_button(tr("Geometry to Origin"), "", ICON_NONE)) {
		util_mesh_to_origin();
		g_context->ddirty = 2;
	}

	if (ui_menu_button(tr("Apply Displacement"), "", ICON_NONE)) {
		util_mesh_apply_displacement(project_layers->buffer[0]->texpaint_pack, 0.1, 1.0);
		util_mesh_calc_normals(false);
		g_context->ddirty = 2;
	}

	if (ui_menu_sub_button(ui_handle(__ID__), tr("Rotate"))) {
		ui_menu_sub_begin(3);
		if (ui_menu_button(tr("X"), "", ICON_NONE)) {
			util_mesh_swap_axis(1, 2);
			g_context->ddirty = 2;
		}
		if (ui_menu_button(tr("Y"), "", ICON_NONE)) {
			util_mesh_swap_axis(2, 0);
			g_context->ddirty = 2;
		}
		if (ui_menu_button(tr("Z"), "", ICON_NONE)) {
			util_mesh_swap_axis(0, 1);
			g_context->ddirty = 2;
		}
		ui_menu_sub_end();
	}

#ifdef WITH_PLUGINS
	if (ui_menu_button(tr("UV Unwrap"), "", ICON_NONE)) {
		plugin_uv_unwrap_button();
	}
#endif

	if (g_config->experimental && ui_menu_button(tr("Decimate"), "", ICON_NONE)) {
		util_mesh_decimate(0.5);
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

	// util_mesh_pack_uvs(raw->vertex_arrays->buffer[2]->values);
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

	if (g_config->experimental) {
		project_fetch_default_meshes();
		if (ui_menu_button(tr("Append Shape"), "", ICON_NONE)) {
			ui_menu_draw(&tab_meshes_draw_append_shape, -1, -1);
		}
	}
}

static vec4_t aabb_center(mesh_data_t *raw) {
	vec4_t          aabb_min  = (vec4_t){-0.01, -0.01, -0.01, 0.0};
	vec4_t          aabb_max  = (vec4_t){0.01, 0.01, 0.01, 0.0};
	i32             i         = 0;
	vertex_array_t *positions = mesh_data_get_vertex_array(raw, "pos");
	while (i < positions->values->length) {
		if (positions->values->buffer[i] > aabb_max.x) {
			aabb_max.x = positions->values->buffer[i];
		}
		if (positions->values->buffer[i + 1] > aabb_max.y) {
			aabb_max.y = positions->values->buffer[i + 1];
		}
		if (positions->values->buffer[i + 2] > aabb_max.z) {
			aabb_max.z = positions->values->buffer[i + 2];
		}
		if (positions->values->buffer[i] < aabb_min.x) {
			aabb_min.x = positions->values->buffer[i];
		}
		if (positions->values->buffer[i + 1] < aabb_min.y) {
			aabb_min.y = positions->values->buffer[i + 1];
		}
		if (positions->values->buffer[i + 2] < aabb_min.z) {
			aabb_min.z = positions->values->buffer[i + 2];
		}
		i += 4;
	}
	f32 f = raw->scale_pos / 32767.0f;
	return (vec4_t){(aabb_min.x + aabb_max.x) / 2.0f * f, (aabb_min.y + aabb_max.y) / 2.0f * f, (aabb_min.z + aabb_max.z) / 2.0f * f, 0.0f};
}

void tab_meshes_make_preview(mesh_object_t *o) {
	if (tab_meshes_preview_map == NULL) {
		tab_meshes_preview_map = any_map_create();
		gc_root(tab_meshes_preview_map);
	}

	char          *uid_key = i32_to_string(o->base->uid);
	gpu_texture_t *image   = any_map_get(tab_meshes_preview_map, uid_key);
	if (image == NULL) {
		image = gpu_create_render_target(util_render_material_preview_size, util_render_material_preview_size, GPU_TEXTURE_FORMAT_RGBA64);
		any_map_set(tab_meshes_preview_map, uid_key, image);
	}

	g_context->material_preview = true;

	slot_material_t *mat = GC_ALLOC_INIT(slot_material_t, {0});
	mat->image           = image;
	mat->image_icon      = gpu_create_render_target(50, 50, GPU_TEXTURE_FORMAT_RGBA64);
	mat->preview_ready   = true;
	mat->canvas          = armpack_decode(slot_material_default_canvas);
	mat->canvas          = util_clone_canvas(mat->canvas); // Clone to create GC references

	slot_material_t *_material = g_context->material;
	g_context->material        = mat;

	mesh_object_t_array_t *_scene_meshes = scene_meshes;
	gc_unroot(scene_meshes);
	scene_meshes = any_array_create_from_raw((void *[]){o}, 1);
	gc_root(scene_meshes);

	mesh_object_t *painto   = g_context->paint_object;
	g_context->paint_object = o;

	g_context->saved_camera = scene_camera->base->transform->local;
	mat4_t m =
	    (mat4_t){0.9146286343879498, 0.404295023959927,   0.000007410128652369705, 0, -0.0032648027153306235, 0.007367569133732468, 0.9999675337275382,   0,
	             0.404281837254303,  -0.9145989516155143, 0.008058532943908717,    0, 0.4659988049397712,     -1.0687517188018691,  0.015935682577325486, 1};
	transform_set_matrix(scene_camera->base->transform, m);
	f32 saved_fov           = scene_camera->data->fov;
	scene_camera->data->fov = 0.4;
	viewport_update_camera_type(CAMERA_TYPE_PERSPECTIVE);

	world_data_t *probe           = scene_world;
	f32           _probe_strength = probe->strength;
	probe->strength               = 2;
	f32 _envmap_angle             = g_context->envmap_angle;
	g_context->envmap_angle       = 0.0;

	gpu_texture_t *_envmap = scene_world->_->envmap;
	scene_world->_->envmap = g_context->preview_envmap;

	// Fit into camera
	vec4_t saved_scale = o->base->transform->scale;
	vec4_t saved_loc   = o->base->transform->loc;
	quat_t saved_rot   = o->base->transform->rot;
	{
		vec4_t aabb = mesh_data_calculate_aabb(o->data);
		f32    r    = math_sqrt(aabb.x * aabb.x + aabb.y * aabb.y + aabb.z * aabb.z);
		f32    s    = 1.0 / r;
		if (o->base->parent == NULL) {
			s *= o->base->transform->scale.x;
		}
		s *= o->data->scale_pos;
		vec4_t center             = aabb_center(o->data);
		o->base->transform->scale = (vec4_t){s, s, s, 1.0};
		o->base->transform->loc   = (vec4_t){-s * center.x, -s * center.y, -s * center.z, 1.0};
		o->base->transform->rot   = (quat_t){0, 0, 0, 1};
		transform_build_matrix(o->base->transform);
	}

	_render_path_last_w = util_render_material_preview_size;
	_render_path_last_h = util_render_material_preview_size;
	camera_object_build_proj(scene_camera, -1.0);
	camera_object_build_mat(scene_camera);

	make_material_parse_mesh_preview_material();
	void (*_commands)(void) = render_path_commands;
	gc_unroot(render_path_commands);
	render_path_commands = render_path_preview_commands_preview;
	gc_root(render_path_commands);
	render_path_render_frame();
	gc_unroot(render_path_commands);
	render_path_commands = _commands;
	gc_root(render_path_commands);

	g_context->material_preview = false;
	_render_path_last_w         = sys_w();
	_render_path_last_h         = sys_h();

	// Restore
	o->base->transform->scale = saved_scale;
	o->base->transform->loc   = saved_loc;
	o->base->transform->rot   = saved_rot;
	transform_build_matrix(o->base->transform);

	gc_unroot(scene_meshes);
	scene_meshes = _scene_meshes;
	gc_root(scene_meshes);
	g_context->paint_object = painto;

	transform_set_matrix(scene_camera->base->transform, g_context->saved_camera);
	viewport_update_camera_type(g_context->camera_type);
	scene_camera->data->fov = saved_fov;
	camera_object_build_proj(scene_camera, -1.0);
	camera_object_build_mat(scene_camera);

	probe->strength         = _probe_strength;
	g_context->envmap_angle = _envmap_angle;
	scene_world->_->envmap  = _envmap;

	g_context->material = _material;
	gpu_delete_texture(mat->image_icon);

	make_material_parse_mesh_material();
	g_context->ddirty = 0;
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

		i32 slotw = math_floor(78 * UI_SCALE());
		i32 num   = math_floor(ui->_window_w / (float)slotw);
		if (num == 0) {
			return;
		}

		f32 uix = 0.0;
		f32 uiy = 0.0;

		for (i32 row = 0; row < math_floor(math_ceil(project_paint_objects->length / (float)num)); ++row) {
			f32_array_t *ar = f32_array_create_from_raw((f32[]){}, 0);
			for (i32 i = 0; i < num * 2; ++i) {
				f32_array_push(ar, 1 / (float)num);
			}
			ui_row(ar);

			ui->_x += 2;
			if (row > 0) {
				ui->_y += UI_ELEMENT_OFFSET() * 10.0;
			}

			for (i32 j = 0; j < num; ++j) {
				i32 imgw = math_floor(75 * UI_SCALE());
				i32 i    = j + row * num;
				if (i >= project_paint_objects->length) {
					ui_end_element_of_size(imgw);
					ui_end_element_of_size(0);
					continue;
				}

				mesh_object_t *o = project_paint_objects->buffer[i];
				uix              = ui->_x;
				uiy              = ui->_y;

				// Draw mesh preview
				char          *uid_key = i32_to_string(o->base->uid);
				gpu_texture_t *preview = tab_meshes_preview_map != NULL ? any_map_get(tab_meshes_preview_map, uid_key) : NULL;
				ui_state_t     state;
				if (preview != NULL) {
					state = ui_image(preview, 0xffffffff, (f32)slotw);
				}
				else {
					gpu_texture_t *icons = resource_get("icons.k");
					rect_t        *rect  = resource_tile50(icons, ICON_CUBE);
					state                = ui_sub_image(icons, ui->ops->theme->BUTTON_COL, slotw, rect->x, rect->y, rect->w, rect->h);
					sys_notify_on_next_frame(tab_meshes_make_preview, o);
				}

				// Selection highlight
				if (g_context->paint_object == o) {
					f32 _uix = ui->_x;
					f32 _uiy = ui->_y;
					ui->_x   = uix;
					ui->_y   = uiy + 4;
					i32 hoff = i % 2 == 1 ? 1 : 0;
					i32 w    = 75;
					ui_fill(0, 0, w + 3, 2, ui->ops->theme->HIGHLIGHT_COL);
					ui_fill(0, w - hoff + 2, w + 3, 2 + hoff, ui->ops->theme->HIGHLIGHT_COL);
					ui_fill(0, 0, 2, w + 3, ui->ops->theme->HIGHLIGHT_COL);
					ui_fill(w + 2, 0, 2, w + 4, ui->ops->theme->HIGHLIGHT_COL);
					ui->_x = _uix;
					ui->_y = _uiy;
				}

				// Click to select
				if (state == UI_STATE_STARTED && ui->input_y > ui->_window_y) {
					g_context->paint_object = o;
				}

				// Context menu
				if (ui->is_hovered && ui->input_released_r) {
					g_context->paint_object = o;
					_tab_meshes_draw_i      = i;
					ui_menu_draw(&tab_meshes_draw_context_menu, -1, -1);
				}

				if (ui->is_hovered) {
					ui_tooltip(o->base->name);
				}

				// Label
				i32 check_w  = UI_ELEMENT_H();
				i32 text_w   = draw_string_width(ui->ops->font, ui->font_size, o->base->name);
				i32 center_x = (slotw - check_w - text_w) / 2;
				ui->_x       = uix + (center_x > 0 ? center_x : 0);
				ui->_y += slotw * 0.9 + 8;

				ui_handle_t *h   = ui_handle(__ID__);
				h->b             = o->base->visible;
				o->base->visible = ui_check(h, o->base->name, "");

				if (h->changed) {
					mesh_object_t_array_t *visibles = any_array_create_from_raw((void *[]){}, 0);
					for (i32 k = 0; k < project_paint_objects->length; ++k) {
						mesh_object_t *p = project_paint_objects->buffer[k];
						if (p->base->visible) {
							any_array_push(visibles, p);
						}
					}
					util_mesh_merge(visibles);
					g_context->ddirty = 2;
				}

				ui->_y -= slotw * 0.9 + 8;
				if (i == project_paint_objects->length - 1) {
					ui->_y += j == num - 1 ? imgw : imgw + UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
				}
			}
		}
	}
}

void tab_meshes_reset_preview_map() {
	if (tab_meshes_preview_map == NULL) {
		return;
	}

	any_array_t *keys = map_keys(tab_meshes_preview_map);
	for (i32 i = 0; i < keys->length; ++i) {
		gpu_texture_t *image = any_map_get(tab_meshes_preview_map, keys->buffer[i]);
		gpu_delete_texture(image);
	}
	gc_unroot(tab_meshes_preview_map);
	tab_meshes_preview_map = NULL;
}
