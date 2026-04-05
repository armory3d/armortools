
#include "global.h"

ui_handle_t *_ui_header_draw_tool_properties_h;

void ui_header_init() {
	ui_header_handle->layout = UI_LAYOUT_HORIZONTAL;
}

void ui_header_render_ui() {
	if (g_config->touch_ui) {
		ui_header_h = ui_header_default_h + 4;
	}
	else {
		ui_header_h = ui_header_default_h;
	}
	ui_header_h = math_floor(ui_header_h * UI_SCALE());

	if (g_config->layout->buffer[LAYOUT_SIZE_HEADER] == 0) {
		return;
	}

	if (!base_view3d_show) {
		return;
	}

	i32 nodesw = (ui_nodes_show || ui_view2d_show) ? g_config->layout->buffer[LAYOUT_SIZE_NODES_W] : 0;
	i32 ww     = iron_window_width() - ui_toolbar_w(true) - g_config->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] - nodesw;

	if (ui->is_typing) {
		ui_header_handle->redraws = 2;
	}

	if (ui_window(ui_header_handle, base_x(), ui_header_h, ww, ui_header_h, false)) {
		ui->_y += 2;
		ui_header_draw_tool_properties();
	}
}

void ui_header_draw_tool_properties_layer_preview_dirty(void *_) {
	g_context->layer_preview_dirty = true;
}

void ui_header_draw_tool_properties_color_picker_normal() {
	ui_fill(0, 0, ui->_w / (float)UI_SCALE(), ui->ops->theme->ELEMENT_H * 9, ui->ops->theme->SEPARATOR_COL);
	ui->changed = false;
	ui_color_wheel(_ui_header_draw_tool_properties_h, false, -1, 10 * ui->ops->theme->ELEMENT_H * UI_SCALE(), false, NULL, NULL);
	if (ui->changed) {
		g_context->picked_color->normal = _ui_header_draw_tool_properties_h->color;
		ui_header_handle->redraws         = 2;
		ui_menu_keep_open                 = true;
	}
}

void ui_header_draw_tool_properties_color_picker_base() {
	ui_fill(0, 0, ui->_w / (float)UI_SCALE(), ui->ops->theme->ELEMENT_H * 9, ui->ops->theme->SEPARATOR_COL);
	ui->changed = false;
	ui_color_wheel(_ui_header_draw_tool_properties_h, false, -1, 10 * ui->ops->theme->ELEMENT_H * UI_SCALE(), false, NULL, NULL);
	if (ui->changed) {
		g_context->picked_color->base = _ui_header_draw_tool_properties_h->color;
		ui_header_handle->redraws       = 2;
		ui_menu_keep_open               = true;
	}
}

void ui_header_draw_tool_properties_to_mask(slot_layer_t *m) {
	_gpu_begin(m->texpaint, NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
	gpu_set_pipeline(pipes_colorid_to_mask);
	render_target_t *rt = any_map_get(render_path_render_targets, "texpaint_colorid");
	gpu_set_texture(pipes_texpaint_colorid, rt->_image);
	gpu_set_texture(pipes_tex_colorid, project_get_image(project_assets->buffer[g_context->colorid_handle->i]));
	gpu_set_vertex_buffer(const_data_screen_aligned_vb);
	gpu_set_index_buffer(const_data_screen_aligned_ib);
	gpu_draw();
	gpu_end();
	g_context->colorid_picked      = false;
	ui_toolbar_handle->redraws       = 1;
	ui_header_handle->redraws        = 1;
	g_context->layer_preview_dirty = true;
	layers_update_fill_layers();
}

void ui_header_draw_tool_properties_import(char *path) {
	import_asset_run(path, -1.0, -1.0, true, false, NULL);
	g_context->colorid_handle->i = project_asset_names->length - 1;
	for (i32 i = 0; i < project_assets->length; ++i) {
		asset_t *a = project_assets->buffer[i];
		// Already imported
		if (string_equals(a->file, path)) {
			g_context->colorid_handle->i = array_index_of(project_assets, a);
		}
	}
	g_context->ddirty               = 2;
	g_context->colorid_picked       = false;
	ui_toolbar_handle->redraws        = 1;
	ui_base_hwnds->buffer[2]->redraws = 2;
}

void ui_header_draw_tool_properties() {
	if (g_context->tool == TOOL_TYPE_COLORID) {
		ui_text(tr("Picked Color"), UI_ALIGN_LEFT, 0x00000000);
		if (g_context->colorid_picked) {
			render_target_t *rt = any_map_get(render_path_render_targets, "texpaint_colorid");
			ui_image(rt->_image, 0xffffffff, 64);
		}
		ui->enabled = g_context->colorid_picked;
		if (ui_button(tr("Clear"), UI_ALIGN_CENTER, "")) {
			g_context->colorid_picked = false;
			ui_toolbar_handle->redraws  = 1;
		}
		ui->enabled = true;
		ui_text(tr("Color ID Map"), UI_ALIGN_LEFT, 0x00000000);
		if (project_asset_names->length > 0) {
			i32 cid = ui_combo(g_context->colorid_handle, base_combo_enum_texts("TEX_IMAGE"), tr("Color ID"), false, UI_ALIGN_LEFT, true);
			if (g_context->colorid_handle == ui->combo_selected_handle) {
				ui->combo_selected_images = base_combo_enum_images("TEX_IMAGE");
			}
			if (g_context->colorid_handle->changed) {
				g_context->ddirty         = 2;
				g_context->colorid_picked = false;
				ui_toolbar_handle->redraws  = 1;
			}
			ui_image(project_get_image(project_assets->buffer[cid]), 0xffffffff, -1.0);
			if (ui->is_hovered) {
				ui_tooltip_image(project_get_image(project_assets->buffer[cid]), 256);
			}
		}
		if (ui_button(tr("Import"), UI_ALIGN_CENTER, "")) {
			ui_files_show(string_array_join(path_texture_formats(), ","), false, true, &ui_header_draw_tool_properties_import);
		}
		ui->enabled = g_context->colorid_picked;
		if (ui_button(tr("To Mask"), UI_ALIGN_CENTER, "")) {
			if (slot_layer_is_mask(g_context->layer)) {
				context_set_layer(g_context->layer->parent);
			}
			slot_layer_t *m = layers_new_mask(false, g_context->layer, -1);
			sys_notify_on_next_frame(&ui_header_draw_tool_properties_to_mask, m);
			history_new_white_mask();
		}
		ui->enabled = true;
	}
	else if (g_context->tool == TOOL_TYPE_PICKER || g_context->tool == TOOL_TYPE_MATERIAL) {

		ui_handle_t *h_color = ui_handle(__ID__);
		h_color->color       = g_context->picked_color->base;
		h_color->color       = color_set_ab(h_color->color, 255);
		ui_state_t state     = ui_text("", 0, h_color->color);
		if (state == UI_STATE_STARTED) {
			base_drag_off_x = -(mouse_x - ui->_x - ui->_window_x - 3);
			base_drag_off_y = -(mouse_y - ui->_y - ui->_window_y + 1);
			gc_unroot(base_drag_swatch);
			base_drag_swatch = project_clone_swatch(g_context->picked_color);
			gc_root(base_drag_swatch);
		}
		if (ui->is_hovered) {
			ui_tooltip(tr("Drag and drop picked color to swatches, materials, layers or to the node editor"));
		}
		if (ui->is_hovered && ui->input_released) {
			gc_unroot(_ui_header_draw_tool_properties_h);
			_ui_header_draw_tool_properties_h = h_color;
			gc_root(_ui_header_draw_tool_properties_h);
			ui_menu_draw(&ui_header_draw_tool_properties_color_picker_base, -1, -1);
		}
		if (ui_button(tr("Add Swatch"), UI_ALIGN_CENTER, "")) {
			swatch_color_t *new_swatch = project_clone_swatch(g_context->picked_color);
			context_set_swatch(new_swatch);
			any_array_push(g_project->swatches, new_swatch);
			ui_base_hwnds->buffer[2]->redraws = 1;
		}
		if (ui->is_hovered) {
			ui_tooltip(tr("Add picked color to swatches"));
		}

		if (g_config->workflow == WORKFLOW_PBR) {

			i32 _w = ui->_w;
			ui->_w /= 2;

			ui_handle_t *h_normal = ui_handle(__ID__);
			h_normal->color       = g_context->picked_color->normal;
			ui_text("", 0, h_normal->color);
			if (ui->is_hovered && ui->input_released) {
				gc_unroot(_ui_header_draw_tool_properties_h);
				_ui_header_draw_tool_properties_h = h_normal;
				gc_root(_ui_header_draw_tool_properties_h);
				ui_menu_draw(&ui_header_draw_tool_properties_color_picker_normal, -1, -1);
			}
			ui_text(tr("Normal"), UI_ALIGN_LEFT, 0x00000000);
			ui->_w = _w;

			ui_handle_t *hocc                    = ui_handle(__ID__);
			hocc->f                              = g_context->picked_color->occlusion;
			g_context->picked_color->occlusion = ui_slider(hocc, tr("Occlusion"), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);

			ui_handle_t *hrough                  = ui_handle(__ID__);
			hrough->f                            = g_context->picked_color->roughness;
			g_context->picked_color->roughness = ui_slider(hrough, tr("Roughness"), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);

			ui_handle_t *hmet                   = ui_handle(__ID__);
			hmet->f                             = g_context->picked_color->metallic;
			g_context->picked_color->metallic = ui_slider(hmet, tr("Metallic"), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);

			ui_handle_t *hheight              = ui_handle(__ID__);
			hheight->f                        = g_context->picked_color->height;
			g_context->picked_color->height = ui_slider(hheight, tr("Height"), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
		}

		ui_handle_t *hopac                 = ui_handle(__ID__);
		hopac->f                           = g_context->picked_color->opacity;
		g_context->picked_color->opacity = ui_slider(hopac, tr("Opacity"), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);

		ui_handle_t *h_select_mat = ui_handle(__ID__);
		if (h_select_mat->init) {
			h_select_mat->b = g_context->picker_select_material;
		}
		g_context->picker_select_material = ui_check(h_select_mat, tr("Select Material"), "");

		string_array_t *picker_mask_combo = any_array_create_from_raw(
		    (void *[]){
		        tr("None"),
		        tr("Material"),
		    },
		    2);
		ui_combo(g_context->picker_mask_handle, picker_mask_combo, tr("Mask"), true, UI_ALIGN_LEFT, true);
		if (g_context->picker_mask_handle->changed) {
			make_material_parse_paint_material(true);
		}
	}
	else if (g_context->tool == TOOL_TYPE_BAKE) {
		ui->changed = false;

		bool baking  = g_context->pdirty > 0;
		bool rt_bake = render_path_paint_is_rt_bake();
		if (baking && ui_button(tr("Stop"), UI_ALIGN_CENTER, "")) {
			g_context->pdirty = 0;
			g_context->rdirty = 2;
		}

		if (!baking && ui_button(tr("Bake"), UI_ALIGN_CENTER, "")) {
			g_context->pdirty = rt_bake ? g_context->bake_samples : 1;
			g_context->rdirty = 3;
			sys_notify_on_next_frame(&ui_header_draw_tool_properties_layer_preview_dirty, NULL);
			ui_base_hwnds->buffer[0]->redraws        = 2;
			history_push_undo                        = true;
			render_path_raytrace_bake_current_sample = 0;
		}

		ui_handle_t *bake_handle = ui_handle(__ID__);
		if (bake_handle->init) {
			bake_handle->i = g_context->bake_type;
		}
		string_array_t *bakes = any_array_create_from_raw(
		    (void *[]){
		        tr("Curvature"),
		        tr("Normal"),
		        tr("Object Normal"),
		        tr("Height"),
		        tr("Derivative"),
		        tr("Position"),
		        tr("TexCoord"),
		        tr("Material ID"),
		        tr("Object ID"),
		        tr("Vertex Color"),
		    },
		    10);
		if (gpu_raytrace_supported()) {
			any_array_push(bakes, tr("AO"));
			any_array_push(bakes, tr("Lightmap"));
			any_array_push(bakes, tr("Bent Normal"));
			any_array_push(bakes, tr("Thickness"));
		}

		g_context->bake_type = ui_combo(bake_handle, bakes, tr("Bake"), false, UI_ALIGN_LEFT, true);

		if (bake_handle->changed && ui_menu_show) {
			ui_menu_nested = true; // Update menu height
		}

		if (rt_bake) {
			ui_handle_t *samples_handle = ui_handle(__ID__);
			if (samples_handle->init) {
				samples_handle->f = g_context->bake_samples;
			}
			g_context->bake_samples = math_floor(ui_slider(samples_handle, tr("Samples"), 1, 512, true, 1, true, UI_ALIGN_RIGHT, true));
		}

		if (g_context->bake_type == BAKE_TYPE_NORMAL_OBJECT || g_context->bake_type == BAKE_TYPE_POSITION ||
		    g_context->bake_type == BAKE_TYPE_BENT_NORMAL) {
			ui_handle_t *bake_up_axis_handle = ui_handle(__ID__);
			if (bake_up_axis_handle->init) {
				bake_up_axis_handle->i = g_context->bake_up_axis;
			}
			string_array_t *bake_up_axis_combo = any_array_create_from_raw(
			    (void *[]){
			        tr("Z"),
			        tr("Y"),
			    },
			    2);
			g_context->bake_up_axis = ui_combo(bake_up_axis_handle, bake_up_axis_combo, tr("Up Axis"), true, UI_ALIGN_LEFT, true);
		}

		if (g_context->bake_type == BAKE_TYPE_AO || g_context->bake_type == BAKE_TYPE_CURVATURE) {
			ui_handle_t *bake_axis_handle = ui_handle(__ID__);
			if (bake_axis_handle->init) {
				bake_axis_handle->i = g_context->bake_axis;
			}
			string_array_t *bake_axis_combo = any_array_create_from_raw(
			    (void *[]){
			        tr("XYZ"),
			        tr("X"),
			        tr("Y"),
			        tr("Z"),
			        tr("-X"),
			        tr("-Y"),
			        tr("-Z"),
			    },
			    7);
			g_context->bake_axis = ui_combo(bake_axis_handle, bake_axis_combo, tr("Axis"), true, UI_ALIGN_LEFT, true);
		}

		if (g_context->bake_type == BAKE_TYPE_AO) {
			ui_handle_t *strength_handle = ui_handle(__ID__);
			if (strength_handle->init) {
				strength_handle->f = g_context->bake_ao_strength;
			}
			g_context->bake_ao_strength = ui_slider(strength_handle, tr("Strength"), 0.0, 2.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
			ui_handle_t *radius_handle    = ui_handle(__ID__);
			if (radius_handle->init) {
				radius_handle->f = g_context->bake_ao_radius;
			}
			g_context->bake_ao_radius = ui_slider(radius_handle, tr("Radius"), 0.0, 2.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
			ui_handle_t *offset_handle  = ui_handle(__ID__);
			if (offset_handle->init) {
				offset_handle->f = g_context->bake_ao_offset;
			}
			g_context->bake_ao_offset = ui_slider(offset_handle, tr("Offset"), 0.0, 2.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
		}

		if (rt_bake) {
			f32 progress = render_path_raytrace_bake_current_sample / (float)g_context->bake_samples;
			if (progress > 1.0)
				progress = 1.0;
			// Progress bar
			draw_set_color(ui->ops->theme->SEPARATOR_COL);
			ui_draw_rect(true, ui->_x + 1, ui->_y, ui->_w - 2, UI_ELEMENT_H());
			draw_set_color(ui->ops->theme->HIGHLIGHT_COL);
			ui_draw_rect(true, ui->_x + 1, ui->_y, (ui->_w - 2) * progress, UI_ELEMENT_H());
			draw_set_color(0xffffffff);
			ui_text(string("%s: %d", tr("Samples"), render_path_raytrace_bake_current_sample), UI_ALIGN_LEFT, 0x00000000);
			ui_text(string("%s: %d", tr("Rays/pixel"), render_path_raytrace_bake_rays_pix), UI_ALIGN_LEFT, 0x00000000);
			ui_text(string("%s: %d", tr("Rays/second"), render_path_raytrace_bake_rays_sec), UI_ALIGN_LEFT, 0x00000000);
		}

		if (g_context->bake_type == BAKE_TYPE_CURVATURE) {
			ui_handle_t *strength_handle = ui_handle(__ID__);
			if (strength_handle->init) {
				strength_handle->f = g_context->bake_curv_strength;
			}
			g_context->bake_curv_strength = ui_slider(strength_handle, tr("Strength"), 0.0, 2.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
			ui_handle_t *radius_handle      = ui_handle(__ID__);
			if (radius_handle->init) {
				radius_handle->f = g_context->bake_curv_radius;
			}
			g_context->bake_curv_radius = ui_slider(radius_handle, tr("Radius"), 0.0, 2.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
			ui_handle_t *offset_handle    = ui_handle(__ID__);
			if (offset_handle->init) {
				offset_handle->f = g_context->bake_curv_offset;
			}
			g_context->bake_curv_offset = ui_slider(offset_handle, tr("Offset"), -2.0, 2.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
			ui_handle_t *smooth_handle    = ui_handle(__ID__);
			if (smooth_handle->init) {
				smooth_handle->f = g_context->bake_curv_smooth;
			}
			g_context->bake_curv_smooth = math_floor(ui_slider(smooth_handle, tr("Smooth"), 0, 5, false, 1, true, UI_ALIGN_RIGHT, true));
		}

		if (g_context->bake_type == BAKE_TYPE_NORMAL || g_context->bake_type == BAKE_TYPE_HEIGHT || g_context->bake_type == BAKE_TYPE_DERIVATIVE) {
			string_array_t *ar = any_array_create_from_raw((void *[]){}, 0);
			for (i32 i = 0; i < project_paint_objects->length; ++i) {
				mesh_object_t *p = project_paint_objects->buffer[i];
				any_array_push(ar, p->base->name);
			}
			ui_handle_t *poly_handle = ui_handle(__ID__);
			if (poly_handle->init) {
				poly_handle->i = g_context->bake_high_poly;
			}
			g_context->bake_high_poly = ui_combo(poly_handle, ar, tr("High Poly"), false, UI_ALIGN_LEFT, true);
		}

		if (ui->changed) {
			make_material_parse_paint_material(true);
		}
	}
	else if (g_context->tool == TOOL_TYPE_BRUSH || g_context->tool == TOOL_TYPE_ERASER || g_context->tool == TOOL_TYPE_FILL ||
	         g_context->tool == TOOL_TYPE_DECAL || g_context->tool == TOOL_TYPE_TEXT || g_context->tool == TOOL_TYPE_CLONE ||
	         g_context->tool == TOOL_TYPE_BLUR || g_context->tool == TOOL_TYPE_SMUDGE || g_context->tool == TOOL_TYPE_PARTICLE) {
		bool decal_mask = context_is_decal_mask();
		if (g_context->tool != TOOL_TYPE_FILL) {
			if (decal_mask) {
				g_context->brush_decal_mask_radius =
				    ui_slider(g_context->brush_decal_mask_radius_handle, tr("Radius"), 0.01, 2.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
				if (ui->is_hovered) {
					any_map_t *vars = any_map_create();
					any_map_set(vars, "brush_radius", any_map_get(config_keymap, "brush_radius"));
					any_map_set(vars, "brush_radius_decrease", any_map_get(config_keymap, "brush_radius_decrease"));
					any_map_set(vars, "brush_radius_increase", any_map_get(config_keymap, "brush_radius_increase"));
					ui_tooltip(
					    vtr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} "
					        "and move mouse to the right or press {brush_radius_increase} to increase the radius",
					        vars));
				}
			}
			else {
				g_context->brush_radius = ui_slider(g_context->brush_radius_handle, tr("Radius"), 0.01, 2.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
				if (ui->is_hovered) {
					any_map_t *vars = any_map_create();
					any_map_set(vars, "brush_radius", any_map_get(config_keymap, "brush_radius"));
					any_map_set(vars, "brush_radius_decrease", any_map_get(config_keymap, "brush_radius_decrease"));
					any_map_set(vars, "brush_radius_increase", any_map_get(config_keymap, "brush_radius_increase"));
					ui_tooltip(
					    vtr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} "
					        "and move mouse to the right or press {brush_radius_increase} to increase the radius",
					        vars));
				}
			}
		}

		if (g_context->tool == TOOL_TYPE_DECAL || g_context->tool == TOOL_TYPE_TEXT) {
			g_context->brush_scale_x = ui_slider(g_context->brush_scale_x_handle, tr("Scale X"), 0.01, 2.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
		}

		if (g_context->tool == TOOL_TYPE_BRUSH || g_context->tool == TOOL_TYPE_FILL || g_context->tool == TOOL_TYPE_DECAL ||
		    g_context->tool == TOOL_TYPE_TEXT) {
			ui_handle_t *brush_scale_handle = ui_handle(__ID__);
			if (brush_scale_handle->init) {
				brush_scale_handle->f = g_context->brush_scale;
			}
			g_context->brush_scale = ui_slider(brush_scale_handle, tr("UV Scale"), 0.01, 5.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
			if (brush_scale_handle->changed) {
				if (g_context->tool == TOOL_TYPE_DECAL || g_context->tool == TOOL_TYPE_TEXT) {
					gpu_texture_t *current = _draw_current;
					draw_end();
					util_render_make_decal_preview();
					draw_begin(current, false, 0);
				}
			}

			g_context->brush_angle = ui_slider(g_context->brush_angle_handle, tr("Angle"), 0.0, 360.0, true, 1, true, UI_ALIGN_RIGHT, true);
			if (ui->is_hovered) {
				any_map_t *vars = any_map_create();
				any_map_set(vars, "brush_angle", any_map_get(config_keymap, "brush_angle"));
				ui_tooltip(vtr(
				    "Hold {brush_angle} and move mouse to the left to decrease the angle\nHold {brush_angle} and move mouse to the right to increase the angle",
				    vars));
			}

			if (g_context->brush_angle_handle->changed) {
				make_material_parse_paint_material(true);
			}
		}

		g_context->brush_opacity = ui_slider(g_context->brush_opacity_handle, tr("Opacity"), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
		if (ui->is_hovered) {
			any_map_t *vars = any_map_create();
			any_map_set(vars, "brush_opacity", any_map_get(config_keymap, "brush_opacity"));
			ui_tooltip(vtr("Hold {brush_opacity} and move mouse to the left to decrease the opacity\nHold {brush_opacity} and move mouse to the right to "
			               "increase the opacity",
			               vars));
		}

		if (g_context->tool == TOOL_TYPE_BRUSH || g_context->tool == TOOL_TYPE_ERASER || g_context->tool == TOOL_TYPE_CLONE || decal_mask) {
			ui_handle_t *h = ui_handle(__ID__);
			if (h->init) {
				h->f = g_context->brush_hardness;
			}
			g_context->brush_hardness = ui_slider(h, tr("Hardness"), 0.0, 1.0, true, 100.0, true, UI_ALIGN_RIGHT, true);
		}

		if (g_context->tool != TOOL_TYPE_ERASER) {
			ui_handle_t *brush_blending_handle = ui_handle(__ID__);
			if (brush_blending_handle->init) {
				brush_blending_handle->f = g_context->brush_blending;
			}
			string_array_t *brush_blending_combo = any_array_create_from_raw(
			    (void *[]){
			        tr("Mix"),
			        tr("Darken"),
			        tr("Multiply"),
			        tr("Burn"),
			        tr("Lighten"),
			        tr("Screen"),
			        tr("Dodge"),
			        tr("Add"),
			        tr("Overlay"),
			        tr("Soft Light"),
			        tr("Linear Light"),
			        tr("Difference"),
			        tr("Subtract"),
			        tr("Divide"),
			        tr("Hue"),
			        tr("Saturation"),
			        tr("Color"),
			        tr("Value"),
			    },
			    18);
			g_context->brush_blending = ui_combo(brush_blending_handle, brush_blending_combo, tr("Blending"), false, UI_ALIGN_LEFT, true);
			if (brush_blending_handle->changed) {
				make_material_parse_paint_material(true);
			}
		}

		if (g_context->tool == TOOL_TYPE_BRUSH || g_context->tool == TOOL_TYPE_FILL) {
			ui_handle_t    *paint_handle   = ui_handle(__ID__);
			string_array_t *texcoord_combo = any_array_create_from_raw(
			    (void *[]){
			        tr("UV Map"),
			        tr("Triplanar"),
			        tr("Project"),
			    },
			    3);
			g_context->brush_paint = ui_combo(paint_handle, texcoord_combo, tr("TexCoord"), false, UI_ALIGN_LEFT, true);
			if (paint_handle->changed) {
				make_material_parse_paint_material(true);
			}
		}

		if (g_context->tool == TOOL_TYPE_TEXT) {
			ui_handle_t *h = ui_handle(__ID__);
			h->text        = string_copy(g_context->text_tool_text);
			i32 w          = ui->_w;
			if (ui->text_selected_handle == h || ui->submit_text_handle == h) {
				ui->_w *= 3;
			}
			g_context->text_tool_text = string_copy(ui_text_input(h, "", UI_ALIGN_LEFT, true, true));
			ui->_w                      = w;
			if (h->changed) {
				gpu_texture_t *current = _draw_current;
				draw_end();
				util_render_make_text_preview();
				util_render_make_decal_preview();
				draw_begin(current, false, 0);
			}
		}

		if (g_context->tool == TOOL_TYPE_FILL) {
			string_array_t *fill_mode_combo = any_array_create_from_raw(
			    (void *[]){
			        tr("Object"),
			        tr("Face"),
			        tr("Angle"),
			        tr("UV Island"),
			    },
			    4);
			ui_combo(g_context->fill_type_handle, fill_mode_combo, tr("Fill Mode"), false, UI_ALIGN_LEFT, true);
			if (g_context->fill_type_handle->changed) {
				if (g_context->fill_type_handle->i == FILL_TYPE_FACE) {
					gpu_texture_t *current = _draw_current;
					draw_end();
					// cache_uv_map();
					util_uv_cache_triangle_map();
					draw_begin(current, false, 0);
					// wireframe_handle.b = draw_wireframe = true;
				}
				make_material_parse_paint_material(true);
				make_material_parse_mesh_material();
			}
		}
		else {
			i32  _w           = ui->_w;
			f32  sc           = UI_SCALE();
			bool touch_header = (g_config->touch_ui && g_config->layout->buffer[LAYOUT_SIZE_HEADER] == 1);
			if (touch_header) {
				ui->_x -= 4 * sc;
			}
			ui->_w = math_floor((touch_header ? 54 : 60) * sc);

			ui_handle_t *xray_handle = ui_handle(__ID__);
			if (xray_handle->init) {
				xray_handle->b = g_context->xray;
			}
			g_context->xray = ui_check(xray_handle, tr("X-Ray"), "");
			if (xray_handle->changed) {
				make_material_parse_paint_material(true);
			}

			ui_handle_t *sym_x_handle = ui_handle(__ID__);
			if (sym_x_handle->init) {
				sym_x_handle->b = false;
			}

			ui_handle_t *sym_y_handle = ui_handle(__ID__);
			if (sym_y_handle->init) {
				sym_y_handle->b = false;
			}

			ui_handle_t *sym_z_handle = ui_handle(__ID__);
			if (sym_z_handle->init) {
				sym_z_handle->b = false;
			}

			if (g_config->layout->buffer[LAYOUT_SIZE_HEADER] == 1) {
				if (g_config->touch_ui) {
					ui->_w             = math_floor(19 * sc);
					g_context->sym_x = ui_check(sym_x_handle, "", "");
					ui->_x -= 4 * sc;
					g_context->sym_y = ui_check(sym_y_handle, "", "");
					ui->_x -= 4 * sc;
					g_context->sym_z = ui_check(sym_z_handle, "", "");
					ui->_x -= 4 * sc;
					ui->_w  = math_floor(40 * sc);
					char *x = tr("X");
					char *y = tr("Y");
					char *z = tr("Z");
					ui_text(string("%s%s%s", x, y, z), UI_ALIGN_LEFT, 0x00000000);
				}
				else {
					ui->_w = math_floor(56 * sc);
					ui_text(tr("Symmetry"), UI_ALIGN_LEFT, 0x00000000);
					ui->_w             = math_floor(25 * sc);
					g_context->sym_x = ui_check(sym_x_handle, tr("X"), "");
					g_context->sym_y = ui_check(sym_y_handle, tr("Y"), "");
					g_context->sym_z = ui_check(sym_z_handle, tr("Z"), "");
				}
				ui->_w = _w;
			}
			else {
				// Popup
				ui->_w             = _w;
				g_context->sym_x = ui_check(sym_x_handle, string("%s %s", tr("Symmetry"), tr("X")), "");
				g_context->sym_y = ui_check(sym_y_handle, string("%s %s", tr("Symmetry"), tr("Y")), "");
				g_context->sym_z = ui_check(sym_z_handle, string("%s %s", tr("Symmetry"), tr("Z")), "");
			}

			if (sym_x_handle->changed || sym_y_handle->changed || sym_z_handle->changed) {
				make_material_parse_paint_material(true);
			}
		}
	}
	if (g_context->tool == TOOL_TYPE_GIZMO) {
		// if (!sim_running && ui_button("Play")) {
		// 	sim_play();
		// 	g_context.selected_object = scene_camera.base;
		// }
		// if (sim_running && ui_button("Stop")) {
		// 	sim_stop();
		// }
		// let h_record: ui_handle_t = ui_handle(__ID__);
		// sim_record = ui_check(h_record, tr("Record"));
	}
}
