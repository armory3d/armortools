
#include "global.h"

void export_arm_run_mesh(char *path, mesh_object_t_array_t *paint_objects) {
	mesh_data_t_array_t *mesh_datas = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < paint_objects->length; ++i) {
		mesh_object_t *p = paint_objects->buffer[i];
		any_array_push(mesh_datas, p->data);
	}

	scene_t  *raw = GC_ALLOC_INIT(scene_t, {.mesh_datas = mesh_datas});
	buffer_t *b   = util_encode_scene(raw);

	if (!ends_with(path, ".arm")) {
		path = string("%s.arm", path);
	}
	iron_file_save_bytes(path, b, b->length + 1);
}

void export_arm_run_project() {
	ui_node_canvas_t_array_t *mnodes = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < project_materials->length; ++i) {
		slot_material_t  *m = project_materials->buffer[i];
		ui_node_canvas_t *c = util_clone_canvas(m->canvas);
		for (i32 i = 0; i < c->nodes->length; ++i) {
			ui_node_t *n = c->nodes->buffer[i];
			export_arm_export_node(n, NULL);
		}
		any_array_push(mnodes, c);
	}

	ui_node_canvas_t_array_t *bnodes = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < project_brushes->length; ++i) {
		slot_brush_t     *b = project_brushes->buffer[i];
		ui_node_canvas_t *c = util_clone_canvas(b->canvas);
		for (i32 i = 0; i < c->nodes->length; ++i) {
			ui_node_t *n = c->nodes->buffer[i];
			export_arm_export_node(n, NULL);
		}
		any_array_push(bnodes, c);
	}

	ui_node_canvas_t_array_t *mgroups = NULL;
	if (project_material_groups->length > 0) {
		mgroups = any_array_create_from_raw((void *[]){}, 0);
		for (i32 i = 0; i < project_material_groups->length; ++i) {
			node_group_t     *g = project_material_groups->buffer[i];
			ui_node_canvas_t *c = util_clone_canvas(g->canvas);
			for (i32 i = 0; i < c->nodes->length; ++i) {
				ui_node_t *n = c->nodes->buffer[i];
				export_arm_export_node(n, NULL);
			}
			any_array_push(mgroups, c);
		}
	}

	mesh_data_t_array_t *md = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		mesh_object_t *p = project_paint_objects->buffer[i];
		any_array_push(md, p->data);
	}

	string_t_array_t *texture_files = export_arm_assets_to_files(project_filepath, project_assets);

	string_t_array_t *font_files = export_arm_fonts_to_files(project_filepath, project_fonts);
	string_t_array_t *mesh_files = export_arm_meshes_to_files(project_filepath);

	i32 bits_pos = base_bits_handle->i;
	i32 bpp      = bits_pos == TEXTURE_BITS_BITS8 ? 8 : bits_pos == TEXTURE_BITS_BITS16 ? 16 : 32;

	layer_data_t_array_t *ld = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < project_layers->length; ++i) {
		slot_layer_t *l = project_layers->buffer[i];
		layer_data_t *d = GC_ALLOC_INIT(layer_data_t, {.name          = l->name,
		                                               .res           = l->texpaint != NULL ? l->texpaint->width : project_layers->buffer[0]->texpaint->width,
		                                               .bpp           = bpp,
		                                               .texpaint      = l->texpaint != NULL ? lz4_encode(gpu_get_texture_pixels(l->texpaint)) : NULL,
		                                               .uv_scale      = l->scale,
		                                               .uv_rot        = l->angle,
		                                               .uv_type       = l->uv_type,
		                                               .uv_map        = l->uv_map,
		                                               .decal_mat     = l->uv_type == UV_TYPE_PROJECT ? mat4_to_f32_array(l->decal_mat) : NULL,
		                                               .opacity_mask  = l->mask_opacity,
		                                               .fill_layer    = l->fill_layer != NULL ? array_index_of(project_materials, l->fill_layer) : -1,
		                                               .object_mask   = l->object_mask,
		                                               .blending      = l->blending,
		                                               .parent        = l->parent != NULL ? array_index_of(project_layers, l->parent) : -1,
		                                               .visible       = l->visible,
		                                               .texpaint_nor  = l->texpaint_nor != NULL ? lz4_encode(gpu_get_texture_pixels(l->texpaint_nor)) : NULL,
		                                               .texpaint_pack = l->texpaint_pack != NULL ? lz4_encode(gpu_get_texture_pixels(l->texpaint_pack)) : NULL,
		                                               .paint_base    = l->paint_base,
		                                               .paint_opac    = l->paint_opac,
		                                               .paint_occ     = l->paint_occ,
		                                               .paint_rough   = l->paint_rough,
		                                               .paint_met     = l->paint_met,
		                                               .paint_nor     = l->paint_nor,
		                                               .paint_nor_blend    = l->paint_nor_blend,
		                                               .paint_height       = l->paint_height,
		                                               .paint_height_blend = l->paint_height_blend,
		                                               .paint_emis         = l->paint_emis,
		                                               .paint_subs         = l->paint_subs});
		any_array_push(ld, d);
	}

	packed_asset_t_array_t *packed_assets = (project_raw->packed_assets == NULL || project_raw->packed_assets->length == 0) ? NULL : project_raw->packed_assets;
#ifdef IRON_IOS
	bool same_drive = false;
#else
	bool same_drive = project_raw->envmap != NULL ? char_at(project_filepath, 0) == char_at(project_raw->envmap, 0) : true;
#endif

	project_raw->version         = string_copy(manifest_version_project);
	project_raw->material_groups = mgroups;
	project_raw->assets          = texture_files;
	project_raw->packed_assets   = packed_assets;
	project_raw->swatches        = project_raw->swatches;
	project_raw->envmap = project_raw->envmap != NULL ? (same_drive ? path_to_relative(project_filepath, project_raw->envmap) : project_raw->envmap) : NULL;
	project_raw->envmap_strength = scene_world->strength;
	project_raw->envmap_angle    = context_raw->envmap_angle;
	project_raw->envmap_blur     = context_raw->show_envmap_blur;
	project_raw->camera_world    = mat4_to_f32_array(scene_camera->base->transform->local);
	project_raw->camera_origin   = export_arm_vec3f32(camera_origins->buffer[0]->v);
	project_raw->camera_fov      = scene_camera->data->fov;

	// project_raw.mesh_datas = md; // TODO: fix GC ref
	if (project_raw->mesh_datas == NULL) {
		project_raw->mesh_datas = md;
	}
	else {
		project_raw->mesh_datas->length = 0;
		for (i32 i = 0; i < md->length; ++i) {
			any_array_push(project_raw->mesh_datas, md->buffer[i]);
		}
	}

	project_raw->material_nodes = mnodes;
	project_raw->brush_nodes    = bnodes;
	project_raw->layer_datas    = ld;
	project_raw->font_assets    = font_files;
	project_raw->mesh_assets    = mesh_files;
	project_raw->atlas_objects  = project_atlas_objects;
	project_raw->atlas_names    = project_atlas_names;

#ifdef IRON_BGRA
	project_raw->is_bgra = true;
#else
	project_raw->is_bgra = false;
#endif

#if defined(IRON_ANDROID) || defined(IRON_IOS)
	render_target_t *rt        = any_map_get(render_path_render_targets, "buf");
	gpu_texture_t   *tex       = rt->_image;
	gpu_texture_t   *mesh_icon = gpu_create_render_target(256, 256, GPU_TEXTURE_FORMAT_RGBA32);
	f32              r         = sys_w() / (float)sys_h();
	draw_begin(mesh_icon, false, 0);
	draw_scaled_image(tex, -(256 * r - 256) / 2.0, 0, 256 * r, 256);
	draw_end();

	buffer_t   *mesh_icon_pixels = gpu_get_texture_pixels(mesh_icon);
	u8_array_t *u8a              = mesh_icon_pixels;
	for (i32 i = 0; i < 256 * 256 * 4; ++i) {
		u8a->buffer[i] = math_floor(math_pow(u8a->buffer[i] / 255.0, 1.0 / 2.2) * 255);
	}
	iron_write_png(string("%s_icon.png", substring(project_filepath, 0, string_length(project_filepath) - 4)), mesh_icon_pixels, 256, 256, 0);
	gpu_delete_texture(mesh_icon);
#endif

	if (context_raw->pack_assets_on_save) { // Pack textures
		export_arm_pack_assets(project_raw, project_assets);
	}

	buffer_t *buffer = util_encode_project(project_raw);
	iron_file_save_bytes(project_filepath, buffer, buffer->length + 1);

	// Save to recent
#ifdef IRON_IOS
	char *recent_path = substring(project_filepath, string_last_index_of(project_filepath, "/") + 1, string_length(project_filepath));
#else
	char *recent_path = project_filepath;
#endif

#ifdef IRON_WINDOWS
	recent_path = string_copy(string_replace_all(recent_path, "\\", "/"));
#endif
	string_t_array_t *recent = config_raw->recent_projects;
	string_array_remove(recent, recent_path);
	array_insert(recent, 0, recent_path);
	config_save();

	console_info(tr("Project saved"));
}

void export_arm_export_node(ui_node_t *n, asset_t_array_t *assets) {
	if (string_equals(n->type, "TEX_IMAGE")) {
		i32 index = n->buttons->buffer[0]->default_value->buffer[0];
		if (index > 9000) { // 9999 - Texture deleted
			n->buttons->buffer[0]->data = u8_array_create_from_string("");
		}
		else {
			n->buttons->buffer[0]->data = u8_array_create_from_string(base_enum_texts(n->type)->buffer[index]);
		}
		if (assets != NULL) {
			asset_t *asset = project_assets->buffer[index];
			if (array_index_of(assets, asset) == -1) {
				any_array_push(assets, asset);
			}
		}
	}
}

void export_arm_run_material(char *path) {
	if (!ends_with(path, ".arm")) {
		path = string("%s.arm", path);
	}
	ui_node_canvas_t_array_t *mnodes  = any_array_create_from_raw((void *[]){}, 0);
	ui_node_canvas_t_array_t *mgroups = NULL;
	slot_material_t          *m       = context_raw->material;
	ui_node_canvas_t         *c       = util_clone_canvas(m->canvas);
	asset_t_array_t          *assets  = any_array_create_from_raw((void *[]){}, 0);
	if (ui_nodes_has_group(c)) {
		mgroups = any_array_create_from_raw((void *[]){}, 0);
		ui_nodes_traverse_group(mgroups, c);
		for (i32 i = 0; i < mgroups->length; ++i) {
			ui_node_canvas_t *gc = mgroups->buffer[i];
			for (i32 i = 0; i < gc->nodes->length; ++i) {
				ui_node_t *n = gc->nodes->buffer[i];
				export_arm_export_node(n, assets);
			}
		}
	}
	for (i32 i = 0; i < c->nodes->length; ++i) {
		ui_node_t *n = c->nodes->buffer[i];
		export_arm_export_node(n, assets);
	}
	any_array_push(mnodes, c);

	string_t_array_t *texture_files = export_arm_assets_to_files(path, assets);
	bool              is_cloud      = ends_with(path, "_cloud_.arm");
	if (is_cloud) {
		path = string_copy(string_replace_all(path, "_cloud_", ""));
	}
	packed_asset_t_array_t *packed_assets = NULL;
	if (!context_raw->pack_assets_on_export) {
		packed_assets = export_arm_get_packed_assets(path, texture_files);
	}

	buffer_t_array_t *micons = NULL;
	if (!is_cloud) {
#ifdef IRON_BGRA
		buffer_t *buf = lz4_encode(export_arm_bgra64_swap(gpu_get_texture_pixels(m->image)));
#else
		buffer_t *buf = lz4_encode(gpu_get_texture_pixels(m->image));
#endif
		micons = any_array_create_from_raw(
		    (void *[]){
		        buf,
		    },
		    1);
	}
	project_format_t *raw = GC_ALLOC_INIT(project_format_t, {.version         = manifest_version_project,
	                                                         .material_nodes  = mnodes,
	                                                         .material_groups = mgroups,
	                                                         .material_icons  = micons,
	                                                         .assets          = texture_files,
	                                                         .packed_assets   = packed_assets});
	if (context_raw->write_icon_on_export) { // Separate icon files
		buffer_t *buf = export_arm_rgba64_to_rgba32(gpu_get_texture_pixels(m->image));
#ifdef IRON_BGRA
		buf = export_arm_bgra_swap(buf);
#endif
		iron_write_png(string("%s_icon.png", substring(path, 0, string_length(path) - 4)), buf, m->image->width, m->image->height, 0);
		iron_write_jpg(string("%s_icon.jpg", substring(path, 0, string_length(path) - 4)), buf, m->image->width, m->image->height, 0, 50);
	}

	if (context_raw->pack_assets_on_export) { // Pack textures
		export_arm_pack_assets(raw, assets);
	}

	buffer_t *buffer = util_encode_project(raw);
	iron_file_save_bytes(path, buffer, buffer->length + 1);
}

buffer_t *export_arm_bgra_swap(buffer_t *buffer) {
	for (i32 i = 0; i < math_floor((buffer->length) / 4.0); ++i) {
		i32 r                     = buffer->buffer[i * 4];
		buffer->buffer[i * 4]     = buffer->buffer[i * 4 + 2];
		buffer->buffer[i * 4 + 2] = r;
	}
	return buffer;
}

buffer_t *export_arm_bgra64_swap(buffer_t *buffer) {
	for (i32 i = 0; i < math_floor((buffer->length) / 8.0); ++i) {
		i32 r_low                 = buffer->buffer[i * 8 + 4];
		i32 r_high                = buffer->buffer[i * 8 + 5];
		buffer->buffer[i * 8 + 4] = buffer->buffer[i * 8 + 0];
		buffer->buffer[i * 8 + 5] = buffer->buffer[i * 8 + 1];
		buffer->buffer[i * 8]     = r_low;
		buffer->buffer[i * 8 + 1] = r_high;
	}
	return buffer;
}

void export_arm_run_brush(char *path) {
	if (!ends_with(path, ".arm")) {
		path = string("%s.arm", path);
	}
	ui_node_canvas_t_array_t *bnodes = any_array_create_from_raw((void *[]){}, 0);
	slot_brush_t             *b      = context_raw->brush;
	ui_node_canvas_t         *c      = util_clone_canvas(b->canvas);
	asset_t_array_t          *assets = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < c->nodes->length; ++i) {
		ui_node_t *n = c->nodes->buffer[i];
		export_arm_export_node(n, assets);
	}
	any_array_push(bnodes, c);

	string_t_array_t *texture_files = export_arm_assets_to_files(path, assets);
	bool              is_cloud      = ends_with(path, "_cloud_.arm");
	if (is_cloud) {
		path = string_copy(string_replace_all(path, "_cloud_", ""));
	}
	packed_asset_t_array_t *packed_assets = NULL;
	if (!context_raw->pack_assets_on_export) {
		packed_assets = export_arm_get_packed_assets(path, texture_files);
	}

	buffer_t_array_t *bicons = NULL;
	if (!is_cloud) {
#ifdef IRON_BGRA
		buffer_t *buf = lz4_encode(export_arm_bgra_swap(gpu_get_texture_pixels(b->image)));
#else
		buffer_t *buf = lz4_encode(gpu_get_texture_pixels(b->image));
#endif
		bicons = any_array_create_from_raw(
		    (void *[]){
		        buf,
		    },
		    1);
	}

	project_format_t *raw = GC_ALLOC_INIT(
	    project_format_t,
	    {.version = manifest_version_project, .brush_nodes = bnodes, .brush_icons = bicons, .assets = texture_files, .packed_assets = packed_assets});

	if (context_raw->write_icon_on_export) { // Separate icon file
		buffer_t *buf = export_arm_rgba64_to_rgba32(gpu_get_texture_pixels(b->image));
#ifdef IRON_BGRA
		buf = export_arm_bgra_swap(buf);
#endif
		iron_write_png(string("%s_icon.png", substring(path, 0, string_length(path) - 4)), buf, b->image->width, b->image->height, 0);
	}

	if (context_raw->pack_assets_on_export) { // Pack textures
		export_arm_pack_assets(raw, assets);
	}

	buffer_t *buffer = util_encode_project(raw);
	iron_file_save_bytes(path, buffer, buffer->length + 1);
}

string_t_array_t *export_arm_assets_to_files(char *project_path, asset_t_array_t *assets) {
	string_t_array_t *texture_files = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < assets->length; ++i) {
		asset_t *a = assets->buffer[i];
#ifdef IRON_IOS
		bool same_drive = false;
#else
		bool same_drive = char_at(project_path, 0) == char_at(a->file, 0);
#endif
		// Convert image path from absolute to relative
		if (same_drive) {
			any_array_push(texture_files, path_to_relative(project_path, a->file));
		}
		else {
			any_array_push(texture_files, a->file);
		}
	}
	return texture_files;
}

string_t_array_t *export_arm_meshes_to_files(char *project_path) {
	string_t_array_t *mesh_files = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < project_mesh_assets->length; ++i) {
		char *file = project_mesh_assets->buffer[i];
#ifdef IRON_IOS
		bool same_drive = false;
#else
		bool same_drive = char_at(project_path, 0) == char_at(file, 0);
#endif
		// Convert mesh path from absolute to relative
		if (same_drive) {
			any_array_push(mesh_files, path_to_relative(project_path, file));
		}
		else {
			any_array_push(mesh_files, file);
		}
	}
	return mesh_files;
}

string_t_array_t *export_arm_fonts_to_files(char *project_path, slot_font_t_array_t *fonts) {
	string_t_array_t *font_files = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 1; i < fonts->length; ++i) {
		slot_font_t *f = fonts->buffer[i];
#ifdef IRON_IOS
		bool same_drive = false;
#else
		bool same_drive = char_at(project_path, 0) == char_at(f->file, 0);
#endif
		// Convert font path from absolute to relative
		if (same_drive) {
			any_array_push(font_files, path_to_relative(project_path, f->file));
		}
		else {
			any_array_push(font_files, f->file);
		}
	}
	return font_files;
}

packed_asset_t_array_t *export_arm_get_packed_assets(char *project_path, string_t_array_t *texture_files) {
	packed_asset_t_array_t *packed_assets = NULL;
	if (project_raw->packed_assets != NULL) {
		for (i32 i = 0; i < project_raw->packed_assets->length; ++i) {
			packed_asset_t *pa = project_raw->packed_assets->buffer[i];
#ifdef IRON_IOS
			bool same_drive = false;
#else
			bool same_drive = char_at(project_path, 0) == char_at(pa->name, 0);
#endif
			// Convert path from absolute to relative
			pa->name = same_drive ? path_to_relative(project_path, pa->name) : pa->name;
			for (i32 i = 0; i < texture_files->length; ++i) {
				char *tf = texture_files->buffer[i];
				if (string_equals(pa->name, tf)) {
					if (packed_assets == NULL) {
						packed_assets = any_array_create_from_raw((void *[]){}, 0);
					}
					any_array_push(packed_assets, pa);
					break;
				}
			}
		}
	}
	return packed_assets;
}

void export_arm_pack_assets(project_format_t *raw, asset_t_array_t *assets) {
	if (raw->packed_assets == NULL) {
		raw->packed_assets = any_array_create_from_raw((void *[]){}, 0);
	}
	gpu_texture_t_array_t *temp_images = any_array_create_from_raw((void *[]){}, 0);
	for (i32 i = 0; i < assets->length; ++i) {
		if (!project_packed_asset_exists(raw->packed_assets, assets->buffer[i]->file)) {
			gpu_texture_t *image = project_get_image(assets->buffer[i]);
			gpu_texture_t *temp  = gpu_create_render_target(image->width, image->height, GPU_TEXTURE_FORMAT_RGBA32);
			draw_begin(temp, false, 0);
			draw_image(image, 0, 0);
			draw_end();
			any_array_push(temp_images, temp);
			packed_asset_t *pa = GC_ALLOC_INIT(packed_asset_t, {.name  = assets->buffer[i]->file,
			                                                    .bytes = ends_with(assets->buffer[i]->file, ".jpg")
			                                                                 ? iron_encode_jpg(gpu_get_texture_pixels(temp), temp->width, temp->height, 0, 80)
			                                                                 : iron_encode_png(gpu_get_texture_pixels(temp), temp->width, temp->height, 0)});
			any_array_push(raw->packed_assets, pa);
		}
	}

	for (i32 i = 0; i < temp_images->length; ++i) {
		gpu_texture_t *image = temp_images->buffer[i];
		gpu_delete_texture(image);
	}
}

void export_arm_run_swatches(char *path) {
	if (!ends_with(path, ".arm")) {
		path = string("%s.arm", path);
	}
	project_format_t *raw    = GC_ALLOC_INIT(project_format_t, {.version = manifest_version_project, .swatches = project_raw->swatches});
	buffer_t         *buffer = util_encode_project(raw);
	iron_file_save_bytes(path, buffer, buffer->length + 1);
}

f32_array_t *export_arm_vec3f32(vec4_t v) {
	f32_array_t *res = f32_array_create(3);
	res->buffer[0]   = v.x;
	res->buffer[1]   = v.y;
	res->buffer[2]   = v.z;
	return res;
}

buffer_t *export_arm_rgba64_to_rgba32(buffer_t *buffer) {
	for (i32 i = 0; i < buffer->length / 2.0; ++i) {
		buffer->buffer[i] = half_to_u8_fast(buffer_get_u16(buffer, i * 2));
	}
	return buffer;
}
