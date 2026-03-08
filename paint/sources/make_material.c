bool make_material_get_mout() {
	for (i32 i = 0; i < context_raw->material->canvas->nodes->length; ++i) {
		ui_node_t *n = context_raw->material->canvas->nodes->buffer[i];
		if (string_equals(n->type, "OUTPUT_MATERIAL_PBR")) {
			return true;
		}
	}
	return false;
}

void make_material_parse_mesh_material() {
	material_data_t *m = project_materials->buffer[0]->data;

	for (i32 i = 0; i < m->_->shader->contexts->length; ++i) {
		shader_context_t *c = m->_->shader->contexts->buffer[i];
		if (string_equals(c->name, "mesh")) {
			array_remove(m->_->shader->contexts, c);
			make_material_delete_context(c);
			break;
		}
	}

	if (make_mesh_layer_pass_count > 1) {
		i32 i = 0;
		while (i < m->_->shader->contexts->length) {
			shader_context_t *c = m->_->shader->contexts->buffer[i];
			for (i32 j = 1; j < make_mesh_layer_pass_count; ++j) {
				char *name = string_join("mesh", i32_to_string(j));
				if (string_equals(c->name, name)) {
					array_remove(m->_->shader->contexts, c);
					make_material_delete_context(c);
					i--;
					break;
				}
			}
			i++;
		}

		i = 0;
		while (i < m->contexts->length) {
			material_context_t *c = m->contexts->buffer[i];
			for (i32 j = 1; j < make_mesh_layer_pass_count; ++j) {
				char *name = string_join("mesh", i32_to_string(j));
				if (string_equals(c->name, name)) {
					array_remove(m->contexts, c);
					i--;
					break;
				}
			}
			i++;
		}
	}

	material_t            *mm  = GC_ALLOC_INIT(material_t, {.name = "Material", .canvas = NULL});

	node_shader_context_t *con = make_mesh_run(mm, 0);
	shader_context_load(con->data);
	any_array_push(m->_->shader->contexts, con->data);

	for (i32 i = 1; i < make_mesh_layer_pass_count; ++i) {
		material_t            *mm  = GC_ALLOC_INIT(material_t, {.name = "Material", .canvas = NULL});
		node_shader_context_t *con = make_mesh_run(mm, i);
		shader_context_load(con->data);
		any_array_push(m->_->shader->contexts, con->data);
		material_context_t *mcon =
		    GC_ALLOC_INIT(material_context_t, {.name = string_join("mesh", i32_to_string(i)), .bind_textures = any_array_create_from_raw((void *[]){}, 0)});
		material_context_load(mcon);
		any_array_push(m->contexts, mcon);
	}

	context_raw->ddirty        = 2;

	render_path_raytrace_dirty = 1;
}

void make_material_parse_mesh_preview_material(material_data_t *md) {
	if (!make_material_get_mout()) {
		return;
	}

	material_data_t  *m    = md == NULL ? project_materials->buffer[0]->data : md;
	shader_context_t *scon = NULL;
	for (i32 i = 0; i < m->_->shader->contexts->length; ++i) {
		shader_context_t *c = m->_->shader->contexts->buffer[i];
		if (string_equals(c->name, "mesh")) {
			scon = c;
			break;
		}
	}

	array_remove(m->_->shader->contexts, scon);

	material_context_t    *mcon = GC_ALLOC_INIT(material_context_t, {.name = "mesh", .bind_textures = any_array_create_from_raw((void *[]){}, 0)});
	material_t            *sd   = GC_ALLOC_INIT(material_t, {.name = "Material", .canvas = NULL});
	node_shader_context_t *con  = make_mesh_preview_run(sd, mcon);

	for (i32 i = 0; i < m->contexts->length; ++i) {
		if (string_equals(m->contexts->buffer[i]->name, "mesh")) {
			material_context_load(mcon);
			m->contexts->buffer[i] = mcon;
			break;
		}
	}

	if (scon != NULL) {
		make_material_delete_context(scon);
	}

	bool compile_error = false;
	shader_context_load(con->data);
	if (con->data == NULL) {
		compile_error = true;
	}
	scon = con->data;
	if (compile_error) {
		return;
	}

	any_array_push(m->_->shader->contexts, scon);
}

void make_material_parse_paint_material(bool bake_previews) {
	if (!make_material_get_mout()) {
		return;
	}

	if (bake_previews) {
		gpu_texture_t *current = _draw_current;
		bool           in_use  = gpu_in_use;
		if (in_use)
			draw_end();
		make_material_bake_node_previews();
		if (in_use)
			draw_begin(current, false, 0);
	}

	material_data_t *m = project_materials->buffer[0]->data;
	for (i32 i = 0; i < m->_->shader->contexts->length; ++i) {
		shader_context_t *c = m->_->shader->contexts->buffer[i];
		if (string_equals(c->name, "paint")) {
			array_remove(m->_->shader->contexts, c);
			if (c != make_material_default_scon) {
				make_material_delete_context(c);
			}
			break;
		}
	}
	for (i32 i = 0; i < m->contexts->length; ++i) {
		material_context_t *c = m->contexts->buffer[i];
		if (string_equals(c->name, "paint")) {
			array_remove(m->contexts, c);
			break;
		}
	}

	material_t            *sdata         = GC_ALLOC_INIT(material_t, {.name = "Material", .canvas = context_raw->material->canvas});
	material_context_t    *tmcon         = GC_ALLOC_INIT(material_context_t, {.name = "paint", .bind_textures = any_array_create_from_raw((void *[]){}, 0)});
	node_shader_context_t *con           = make_paint_run(sdata, tmcon);

	bool                   compile_error = false;
	shader_context_t      *scon;
	shader_context_load(con->data);
	if (con->data == NULL) {
		compile_error = true;
	}
	scon = con->data;
	if (compile_error) {
		return;
	}
	material_context_load(tmcon);
	material_context_t *mcon = tmcon;

	any_array_push(m->_->shader->contexts, scon);
	any_array_push(m->contexts, mcon);

	if (make_material_default_scon == NULL) {
		gc_unroot(make_material_default_scon);
		make_material_default_scon = scon;
		gc_root(make_material_default_scon);
	}
	if (make_material_default_mcon == NULL) {
		gc_unroot(make_material_default_mcon);
		make_material_default_mcon = mcon;
		gc_root(make_material_default_mcon);
	}
}

void make_material_bake_node_previews() {
	context_raw->node_previews_used = any_array_create_from_raw((void *[]){}, 0);
	if (context_raw->node_previews == NULL) {
		context_raw->node_previews = any_map_create();
	}
	ui_node_t_array_t *empty = any_array_create_from_raw((void *[]){}, 0);
	make_material_traverse_nodes(context_raw->material->canvas->nodes, NULL, empty);

	string_t_array_t *keys = map_keys(context_raw->node_previews);
	for (i32 i = 0; i < keys->length; ++i) {
		char *key = keys->buffer[i];
		if (char_ptr_array_index_of(context_raw->node_previews_used, key) == -1) {
			gpu_texture_t *image = any_map_get(context_raw->node_previews, key);
			gpu_delete_texture(image);
			map_delete(context_raw->node_previews, key);
		}
	}
}

void make_material_traverse_nodes(ui_node_t_array_t *nodes, ui_node_canvas_t *group, ui_node_t_array_t *parents) {
	for (i32 i = 0; i < nodes->length; ++i) {
		ui_node_t *node = nodes->buffer[i];
		make_material_bake_node_preview(node, group, parents);
		if (string_equals(node->type, "GROUP")) {
			for (i32 j = 0; j < project_material_groups->length; ++j) {
				node_group_t *g     = project_material_groups->buffer[j];
				char     *cname = g->canvas->name;
				if (string_equals(cname, node->name)) {
					any_array_push(parents, node);
					make_material_traverse_nodes(g->canvas->nodes, g->canvas, parents);
					array_pop(parents);
					break;
				}
			}
		}
	}
}

void make_material_bake_node_preview(ui_node_t *node, ui_node_canvas_t *group, ui_node_t_array_t *parents) {
	if (string_equals(node->type, "BLUR")) {
		char      *id    = parser_material_node_name(node, parents);
		gpu_texture_t *image = any_map_get(context_raw->node_previews, id);
		any_array_push(context_raw->node_previews_used, id);
		i32 res_x = math_floor(config_get_texture_res_x() / (float)4);
		i32 res_y = math_floor(config_get_texture_res_y() / (float)4);
		if (image == NULL || image->width != res_x || image->height != res_y) {
			if (image != NULL) {
				gpu_delete_texture(image);
			}
			image = gpu_create_render_target(res_x, res_y, GPU_TEXTURE_FORMAT_RGBA32);
			any_map_set(context_raw->node_previews, id, image);
		}

		parser_material_blur_passthrough = true;
		util_render_make_node_preview(context_raw->material->canvas, node, image, group, parents);
		parser_material_blur_passthrough = false;
	}
	else if (string_equals(node->type, "DIRECT_WARP")) {
		char      *id    = parser_material_node_name(node, parents);
		gpu_texture_t *image = any_map_get(context_raw->node_previews, id);
		any_array_push(context_raw->node_previews_used, id);
		i32 res_x = math_floor(config_get_texture_res_x());
		i32 res_y = math_floor(config_get_texture_res_y());
		if (image == NULL || image->width != res_x || image->height != res_y) {
			if (image != NULL) {
				gpu_delete_texture(image);
			}
			image = gpu_create_render_target(res_x, res_y, GPU_TEXTURE_FORMAT_RGBA32);
			any_map_set(context_raw->node_previews, id, image);
		}

		parser_material_warp_passthrough = true;
		util_render_make_node_preview(context_raw->material->canvas, node, image, group, parents);
		parser_material_warp_passthrough = false;
	}
	else if (string_equals(node->type, "BAKE_CURVATURE")) {
		char      *id    = parser_material_node_name(node, parents);
		gpu_texture_t *image = any_map_get(context_raw->node_previews, id);
		any_array_push(context_raw->node_previews_used, id);
		i32 res_x = math_floor(config_get_texture_res_x());
		i32 res_y = math_floor(config_get_texture_res_y());
		if (image == NULL || image->width != res_x || image->height != res_y) {
			if (image != NULL) {
				gpu_delete_texture(image);
			}
			image = gpu_create_render_target(res_x, res_y, GPU_TEXTURE_FORMAT_R8);
			any_map_set(context_raw->node_previews, id, image);
		}

		if (render_path_paint_live_layer == NULL) {
			gc_unroot(render_path_paint_live_layer);
			render_path_paint_live_layer = slot_layer_create("_live", LAYER_SLOT_TYPE_LAYER, NULL);
			gc_root(render_path_paint_live_layer);
		}

		tool_type_t _tool      = context_raw->tool;
		bake_type_t _bake_type = context_raw->bake_type;
		context_raw->tool      = TOOL_TYPE_BAKE;
		context_raw->bake_type = BAKE_TYPE_CURVATURE;

		parser_material_bake_passthrough = true;
		gc_unroot(parser_material_start_node);
		parser_material_start_node = node;
		gc_root(parser_material_start_node);
		gc_unroot(parser_material_start_group);
		parser_material_start_group = group;
		gc_root(parser_material_start_group);
		gc_unroot(parser_material_start_parents);
		parser_material_start_parents = parents;
		gc_root(parser_material_start_parents);
		make_material_parse_paint_material(false);
		parser_material_bake_passthrough = false;
		gc_unroot(parser_material_start_node);
		parser_material_start_node = NULL;
		gc_unroot(parser_material_start_group);
		parser_material_start_group = NULL;
		gc_unroot(parser_material_start_parents);
		parser_material_start_parents = NULL;
		context_raw->pdirty           = 1;
		render_path_paint_use_live_layer(true);
		render_path_paint_commands_paint(false);
		render_path_paint_dilate(true, false);
		render_path_paint_use_live_layer(false);
		context_raw->pdirty = 0;

		context_raw->tool      = _tool;
		context_raw->bake_type = _bake_type;
		make_material_parse_paint_material(false);

		any_map_t       *rts           = render_path_render_targets;
		render_target_t *texpaint_live = any_map_get(rts, "texpaint_live");
		draw_begin(image, false, 0);
		draw_image(texpaint_live->_image, 0, 0);
		draw_end();
	}
}

parse_node_preview_result_t *make_material_parse_node_preview_material(ui_node_t *node, ui_node_canvas_t *group, ui_node_t_array_t *parents) {
	if (node->outputs->length == 0) {
		return NULL;
	}

	material_t            *sdata         = GC_ALLOC_INIT(material_t, {.name = "Material", .canvas = context_raw->material->canvas});
	material_context_t    *mcon_raw      = GC_ALLOC_INIT(material_context_t, {.name = "mesh", .bind_textures = any_array_create_from_raw((void *[]){}, 0)});
	node_shader_context_t *con           = make_node_preview_run(sdata, mcon_raw, node, group, parents);
	bool                   compile_error = false;
	shader_context_t      *scon;
	shader_context_load(con->data);

	if (con->data == NULL) {
		compile_error = true;
	}
	scon = con->data;
	if (compile_error) {
		return NULL;
	}

	material_context_load(mcon_raw);
	material_context_t          *mcon   = mcon_raw;
	parse_node_preview_result_t *result = GC_ALLOC_INIT(parse_node_preview_result_t, {.scon = scon, .mcon = mcon});

	return result;
}

void make_material_parse_brush() {
	parser_logic_parse(context_raw->brush->canvas);
}

char *make_material_blend_mode(node_shader_t *kong, i32 blending, char *cola, char *colb, char *opac) {
	if (blending == BLEND_TYPE_MIX) {
		return string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola), ", "), colb), ", "), opac), ")");
	}
	else if (blending == BLEND_TYPE_DARKEN) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola), ", min3("), cola), ", "), colb), "), "), opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_MULTIPLY) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola), ", "), cola), " * "), colb), ", "), opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_BURN) {
		return string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola),
		                                                                                           ", float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - "),
		                                                                               cola),
		                                                                   ") / "),
		                                                       colb),
		                                           ", "),
		                               opac),
		                   ")");
	}
	else if (blending == BLEND_TYPE_LIGHTEN) {
		return string_join(string_join(string_join(string_join(string_join(string_join("max3(", cola), ", "), colb), " * "), opac), ")");
	}
	else if (blending == BLEND_TYPE_SCREEN) {
		return string_join(
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(string_join(string_join(string_join(string_join(string_join("(float3(1.0, 1.0, 1.0) - (float3(1.0 - ", opac),
		                                                                                    ", 1.0 - "),
		                                                                        opac),
		                                                            ", 1.0 - "),
		                                                opac),
		                                    ") + "),
		                        opac),
		                    " * (float3(1.0, 1.0, 1.0) - "),
		                colb),
		            ")) * (float3(1.0, 1.0, 1.0) - "),
		        cola),
		    "))");
	}
	else if (blending == BLEND_TYPE_DODGE) {
		return string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola), ", "), cola),
		                                                                   " / (float3(1.0, 1.0, 1.0) - "),
		                                                       colb),
		                                           "), "),
		                               opac),
		                   ")");
	}
	else if (blending == BLEND_TYPE_ADD) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola), ", "), cola), " + "), colb), ", "), opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_OVERLAY) {
		// return "lerp3(" + cola + ", float3( \
		// 	" + cola + ".r < 0.5 ? 2.0 * " + cola + ".r * " + colb + ".r : 1.0 - 2.0 * (1.0 - " + cola + ".r) * (1.0 - " + colb + ".r), \
		// 	" + cola + ".g < 0.5 ? 2.0 * " + cola + ".g * " + colb + ".g : 1.0 - 2.0 * (1.0 - " + cola + ".g) * (1.0 - " + colb + ".g), \
		// 	" + cola + ".b < 0.5 ? 2.0 * " + cola + ".b * " + colb + ".b : 1.0 - 2.0 * (1.0 - " + cola + ".b) * (1.0 - " + colb + ".b) \
		// ), " + opac + ")";
		char *cola_rgb = string_join(string_replace_all(cola, ".", "_"), "_rgb");
		char *colb_rgb = string_join(string_replace_all(colb, ".", "_"), "_rgb");
		char *res_r    = string_join(string_replace_all(cola, ".", "_"), "_res_r");
		char *res_g    = string_join(string_replace_all(cola, ".", "_"), "_res_g");
		char *res_b    = string_join(string_replace_all(cola, ".", "_"), "_res_b");
		node_shader_write_frag(kong, string_join(string_join("var ", res_r), ": float;"));
		node_shader_write_frag(kong, string_join(string_join("var ", res_g), ": float;"));
		node_shader_write_frag(kong, string_join(string_join("var ", res_b), ": float;"));
		node_shader_write_frag(kong, string_join(string_join(string_join(string_join("var ", cola_rgb), ": float3 = "), cola), ";")); // cola_rgb = cola.rgb
		node_shader_write_frag(kong, string_join(string_join(string_join(string_join("var ", colb_rgb), ": float3 = "), colb), ";"));
		node_shader_write_frag(
		    kong,
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("if (", cola_rgb),
		                                                                                                                        ".r < 0.5) { "),
		                                                                                                            res_r),
		                                                                                                " = 2.0 * "),
		                                                                                    cola_rgb),
		                                                                        ".r * "),
		                                                            colb_rgb),
		                                                ".r; } else { "),
		                                    res_r),
		                        " = 1.0 - 2.0 * (1.0 - "),
		                    cola_rgb),
		                ".r) * (1.0 - "),
		            colb_rgb),
		        ".r); }"));
		node_shader_write_frag(
		    kong,
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("if (", cola_rgb),
		                                                                                                                        ".g < 0.5) { "),
		                                                                                                            res_g),
		                                                                                                " = 2.0 * "),
		                                                                                    cola_rgb),
		                                                                        ".g * "),
		                                                            colb_rgb),
		                                                ".g; } else { "),
		                                    res_g),
		                        " = 1.0 - 2.0 * (1.0 - "),
		                    cola_rgb),
		                ".g) * (1.0 - "),
		            colb_rgb),
		        ".g); }"));
		node_shader_write_frag(
		    kong,
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("if (", cola_rgb),
		                                                                                                                        ".b < 0.5) { "),
		                                                                                                            res_b),
		                                                                                                " = 2.0 * "),
		                                                                                    cola_rgb),
		                                                                        ".b * "),
		                                                            colb_rgb),
		                                                ".b; } else { "),
		                                    res_b),
		                        " = 1.0 - 2.0 * (1.0 - "),
		                    cola_rgb),
		                ".b) * (1.0 - "),
		            colb_rgb),
		        ".b); }"));
		return string_join(
		    string_join(
		        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola), ", float3("), res_r), ", "),
		                                                        res_g),
		                                            ", "),
		                                res_b),
		                    "), "),
		        opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_SOFT_LIGHT) {
		return string_join(
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(
		                            string_join(
		                                string_join(
		                                    string_join(
		                                        string_join(
		                                            string_join(
		                                                string_join(string_join(string_join(string_join(string_join(string_join("((1.0 - ", opac), ") * "),
		                                                                                                cola),
		                                                                                    " + "),
		                                                                        opac),
		                                                            " * ((float3(1.0, 1.0, 1.0) - "),
		                                                cola),
		                                            ") * "),
		                                        colb),
		                                    " * "),
		                                cola),
		                            " + "),
		                        cola),
		                    " * (float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - "),
		                colb),
		            ") * (float3(1.0, 1.0, 1.0) - "),
		        cola),
		    "))))");
	}
	else if (blending == BLEND_TYPE_LINEAR_LIGHT) {
		return string_join(string_join(string_join(string_join(string_join(string_join("(", cola), " + "), opac), " * (float3(2.0, 2.0, 2.0) * ("), colb),
		                   " - float3(0.5, 0.5, 0.5))))");
	}
	else if (blending == BLEND_TYPE_DIFFERENCE) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola), ", abs3("), cola), " - "), colb), "), "),
		                opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_SUBTRACT) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola), ", "), cola), " - "), colb), ", "), opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_DIVIDE) {
		return string_join(
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(
		                            string_join(
		                                string_join(
		                                    string_join(
		                                        string_join(
		                                            string_join(string_join(string_join(string_join(string_join(string_join(string_join("float3(1.0 - ", opac),
		                                                                                                                    ", 1.0 - "),
		                                                                                                        opac),
		                                                                                            ", 1.0 - "),
		                                                                                opac),
		                                                                    ") * "),
		                                                        cola),
		                                            " + float3("),
		                                        opac),
		                                    ", "),
		                                opac),
		                            ", "),
		                        opac),
		                    ") * "),
		                cola),
		            " / "),
		        colb),
		    "");
	}
	else if (blending == BLEND_TYPE_HUE) {
		node_shader_add_function(kong, str_hue_sat);
		return string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola),
		                                                                                                                   ", hsv_to_rgb(float3(rgb_to_hsv("),
		                                                                                                       colb),
		                                                                                           ").r, rgb_to_hsv("),
		                                                                               cola),
		                                                                   ").g, rgb_to_hsv("),
		                                                       cola),
		                                           ").b)), "),
		                               opac),
		                   ")");
	}
	else if (blending == BLEND_TYPE_SATURATION) {
		node_shader_add_function(kong, str_hue_sat);
		return string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola),
		                                                                                                                   ", hsv_to_rgb(float3(rgb_to_hsv("),
		                                                                                                       cola),
		                                                                                           ").r, rgb_to_hsv("),
		                                                                               colb),
		                                                                   ").g, rgb_to_hsv("),
		                                                       cola),
		                                           ").b)), "),
		                               opac),
		                   ")");
	}
	else if (blending == BLEND_TYPE_COLOR) {
		node_shader_add_function(kong, str_hue_sat);
		return string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola),
		                                                                                                                   ", hsv_to_rgb(float3(rgb_to_hsv("),
		                                                                                                       colb),
		                                                                                           ").r, rgb_to_hsv("),
		                                                                               colb),
		                                                                   ").g, rgb_to_hsv("),
		                                                       cola),
		                                           ").b)), "),
		                               opac),
		                   ")");
	}
	else { // BlendValue
		node_shader_add_function(kong, str_hue_sat);
		return string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp3(", cola),
		                                                                                                                   ", hsv_to_rgb(float3(rgb_to_hsv("),
		                                                                                                       cola),
		                                                                                           ").r, rgb_to_hsv("),
		                                                                               cola),
		                                                                   ").g, rgb_to_hsv("),
		                                                       colb),
		                                           ").b)), "),
		                               opac),
		                   ")");
	}
}

char *make_material_blend_mode_mask(node_shader_t *kong, i32 blending, char *cola, char *colb, char *opac) {
	if (blending == BLEND_TYPE_MIX) {
		return string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", "), colb), ", "), opac), ")");
	}
	else if (blending == BLEND_TYPE_DARKEN) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", min("), cola), ", "), colb), "), "), opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_MULTIPLY) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", "), cola), " * "), colb), ", "), opac), ")");
	}
	else if (blending == BLEND_TYPE_BURN) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", 1.0 - (1.0 - "), cola), ") / "), colb),
		                            ", "),
		                opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_LIGHTEN) {
		return string_join(string_join(string_join(string_join(string_join(string_join("max(", cola), ", "), colb), " * "), opac), ")");
	}
	else if (blending == BLEND_TYPE_SCREEN) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("(1.0 - ((1.0 - ", opac), ") + "), opac), " * (1.0 - "), colb),
		                            ")) * (1.0 - "),
		                cola),
		    "))");
	}
	else if (blending == BLEND_TYPE_DODGE) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", "), cola), " / (1.0 - "), colb), "), "),
		                opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_ADD) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", "), cola), " + "), colb), ", "), opac), ")");
	}
	else if (blending == BLEND_TYPE_OVERLAY) {
		// return "lerp(" + cola + ", " + cola + " < 0.5 ? 2.0 * " + cola + " * " + colb + " : 1.0 - 2.0 * (1.0 - " + cola + ") * (1.0 - " + colb + "), " + opac
		// + ")";
		char *res = string_join(string_replace_all(cola, ".", "_"), "_res");
		node_shader_write_frag(kong, string_join(string_join("var ", res), ": float;"));
		node_shader_write_frag(
		    kong, string_join(
		              string_join(
		                  string_join(
		                      string_join(
		                          string_join(
		                              string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("if (", cola),
		                                                                                                                              " < 0.5) { "),
		                                                                                                                  res),
		                                                                                                      " = 2.0 * "),
		                                                                                          cola),
		                                                                              " * "),
		                                                                  colb),
		                                                      "; } else { "),
		                                          res),
		                              " = 1.0 - 2.0 * (1.0 - "),
		                          cola),
		                      ") * (1.0 - "),
		                  colb),
		              "); }"));
		return string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", "), res), ", "), opac), ")");
	}
	else if (blending == BLEND_TYPE_SOFT_LIGHT) {
		return string_join(
		    string_join(
		        string_join(
		            string_join(
		                string_join(
		                    string_join(
		                        string_join(
		                            string_join(
		                                string_join(
		                                    string_join(
		                                        string_join(
		                                            string_join(
		                                                string_join(string_join(string_join(string_join(string_join(string_join("((1.0 - ", opac), ") * "),
		                                                                                                cola),
		                                                                                    " + "),
		                                                                        opac),
		                                                            " * ((1.0 - "),
		                                                cola),
		                                            ") * "),
		                                        colb),
		                                    " * "),
		                                cola),
		                            " + "),
		                        cola),
		                    " * (1.0 - (1.0 - "),
		                colb),
		            ") * (1.0 - "),
		        cola),
		    "))))");
	}
	else if (blending == BLEND_TYPE_LINEAR_LIGHT) {
		return string_join(string_join(string_join(string_join(string_join(string_join("(", cola), " + "), opac), " * (2.0 * ("), colb), " - 0.5)))");
	}
	else if (blending == BLEND_TYPE_DIFFERENCE) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", abs("), cola), " - "), colb), "), "), opac),
		    ")");
	}
	else if (blending == BLEND_TYPE_SUBTRACT) {
		return string_join(
		    string_join(string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", "), cola), " - "), colb), ", "), opac), ")");
	}
	else if (blending == BLEND_TYPE_DIVIDE) {
		return string_join(
		    string_join(
		        string_join(string_join(string_join(string_join(string_join(string_join(string_join(string_join("(1.0 - ", opac), ") * "), cola), " + "), opac),
		                                            " * "),
		                                cola),
		                    " / "),
		        colb),
		    "");
	}
	else { // BlendHue, BlendSaturation, BlendColor, BlendValue
		return string_join(string_join(string_join(string_join(string_join(string_join("lerp(", cola), ", "), colb), ", "), opac), ")");
	}
}

f32 make_material_get_displace_strength() {
	vec4_t sc = context_main_object()->base->transform->scale;
	return config_raw->displace_strength * 0.02 * sc.x;
}

void make_material_delete_context(shader_context_t *c) {
	sys_notify_on_next_frame(&make_material_delete_context_137861, c); // Ensure pipeline is no longer in use
}

void make_material_delete_context_137861(shader_context_t *c) {
	shader_context_delete(c);
}
