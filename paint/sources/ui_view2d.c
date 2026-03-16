
#include "global.h"

void ui_view2d_init() {
	ui_view2d_pipe = gpu_create_pipeline();
	gc_root(ui_view2d_pipe);

	ui_view2d_pipe->vertex_shader   = sys_get_shader("layer_view.vert");
	ui_view2d_pipe->fragment_shader = sys_get_shader("layer_view.frag");
	gpu_vertex_structure_t *vs      = GC_ALLOC_INIT(gpu_vertex_structure_t, {0});
	gpu_vertex_struct_add(vs, "pos", GPU_VERTEX_DATA_F32_2X);
	ui_view2d_pipe->input_layout              = vs;
	ui_view2d_pipe->blend_source              = GPU_BLEND_ONE;
	ui_view2d_pipe->blend_destination         = GPU_BLEND_ZERO;
	ui_view2d_pipe->color_write_mask_alpha[0] = false;
	gpu_pipeline_compile(ui_view2d_pipe);
	pipes_offset = 0;
	pipes_get_constant_location("float4");
	pipes_get_constant_location("float4");
	pipes_get_constant_location("float4");
	ui_view2d_channel_loc = pipes_get_constant_location("int");
}

void ui_view2d_draw_image(gpu_texture_t *image, f32 dx, f32 dy, f32 dw, f32 dh, i32 channel) {
	if (ui_view2d_type == VIEW_2D_TYPE_LAYER) {
		gpu_set_int(ui_view2d_channel_loc, channel);
	}
	draw_scaled_image(image, dx, dy, dw, dh);
}

void ui_view2d_render_color_pick(void *_) {
	render_target_t *rt              = any_map_get(render_path_render_targets, "texpaint_picker");
	gpu_texture_t   *texpaint_picker = rt->_image;
	draw_begin(texpaint_picker, false, 0);
	draw_scaled_image(_ui_view2d_render_tex, -_ui_view2d_render_x, -_ui_view2d_render_y, _ui_view2d_render_tw, _ui_view2d_render_th);
	draw_end();
	buffer_t *a = gpu_get_texture_pixels(texpaint_picker);
#ifdef IRON_BGRA
	i32 i0 = 2;
	i32 i1 = 1;
	i32 i2 = 0;
#else
	i32 i0 = 0;
	i32 i1 = 1;
	i32 i2 = 2;
#endif

	context_raw->picked_color->base = color_set_rb(context_raw->picked_color->base, buffer_get_u8(a, i0));
	context_raw->picked_color->base = color_set_gb(context_raw->picked_color->base, buffer_get_u8(a, i1));
	context_raw->picked_color->base = color_set_bb(context_raw->picked_color->base, buffer_get_u8(a, i2));
	ui_header_handle->redraws       = 2;

	if (context_raw->color_picker_callback != NULL) {
		context_raw->color_picker_callback(context_raw->picked_color);
	}
}

void ui_view2d_render(void *_) {
	ui_view2d_ww = config_raw->layout->buffer[LAYOUT_SIZE_NODES_W];
	ui_view2d_wx = math_floor(sys_w()) + ui_toolbar_w(true);
	ui_view2d_wy = 0;

	if (!ui_base_show) {
		ui_view2d_ww += config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] + ui_toolbar_w(true);
		ui_view2d_wx -= ui_toolbar_w(true);
	}
	if (!base_view3d_show) {
		ui_view2d_ww += base_view3d_w();
	}

	if (!ui_view2d_show) {
		return;
	}

	if (context_raw->pdirty >= 0) {
		ui_view2d_hwnd->redraws = 2; // Paint was active
	}

	// Cache grid
	if (ui_view2d_grid_redraw) {
		if (ui_view2d_grid != NULL) {
			gpu_delete_texture(ui_view2d_grid);
		}
		gc_unroot(ui_view2d_grid);
		ui_view2d_grid = ui_nodes_draw_grid(ui_view2d_pan_scale);
		gc_root(ui_view2d_grid);
		ui_view2d_grid_redraw = false;
	}

	// Ensure UV map is drawn
	if (ui_view2d_uvmap_show) {
		util_uv_cache_uv_map();
	}

	// Ensure font image is drawn
	if (context_raw->font->image == NULL) {
		util_render_make_font_preview();
	}

	ui->input_enabled = base_ui_enabled;

	ui_begin(ui);

	i32 headerh = config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 1 ? ui_header_h * 2 : ui_header_h;
	i32 apph    = iron_window_height() - config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] + headerh;
	if (!base_view3d_show) {
		apph = base_h();
	}
	ui_view2d_wh = iron_window_height() - config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H];

	if (ui_nodes_show) {
		ui_view2d_wh -= config_raw->layout->buffer[LAYOUT_SIZE_NODES_H];
		if (config_raw->touch_ui) {
			ui_view2d_wh += ui_header_h;
		}
	}

	if (!base_view3d_show && ui_nodes_show) {
		ui_view2d_wx = 0;
		ui_view2d_ww = base_view3d_w();
		ui_view2d_wh = iron_window_height() - config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H];
	}

	if (ui_window(ui_view2d_hwnd, ui_view2d_wx, ui_view2d_wy, ui_view2d_ww, ui_view2d_wh, false)) {

		if (!config_raw->touch_ui) {
			bool expand = !base_view3d_show && !ui_nodes_show && config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] == 0;
			ui_tab(ui_view2d_htab, expand ? string("%s          ", tr("2D View")) : tr("2D View"), false, -1, !base_view3d_show);
			if (ui_tab(ui_view2d_htab, tr("+"), false, -1, false)) {
				ui_view2d_htab->i = 0;
			}
		}

		// Grid
		// draw_set_color(0xffffffff);
		// let step: f32 = ui_nodes_grid_cell_w * ui_view2d_pan_scale;
		// let x: f32    = math_fmod(ui_view2d_pan_x, step) - step;
		// let y: f32    = math_fmod(ui_view2d_pan_y, step) - step;
		// draw_image(ui_view2d_grid, x, y);
		draw_set_color(ui->ops->theme->SEPARATOR_COL + 0x00020202);
		draw_filled_rect(0, 0, ui_view2d_ww, ui_view2d_wh);
		draw_set_color(0xffffffff);

		// Texture
		gpu_texture_t *tex     = NULL;
		slot_layer_t  *l       = context_raw->layer;
		i32            channel = 0;

		i32 wm = fmin(ui_view2d_ww, ui_view2d_wh);
		i32 tw = wm * 0.9 * ui_view2d_pan_scale;
		i32 tx = ui_view2d_ww / 2.0 - tw / 2.0 + ui_view2d_pan_x;
		i32 ty = apph / 2.0 - tw / 2.0 + ui_view2d_pan_y;

		if (ui_view2d_type == VIEW_2D_TYPE_ASSET) {
			tex = project_get_image(context_raw->texture);
		}
		else if (ui_view2d_type == VIEW_2D_TYPE_NODE) {
			ui_nodes_t       *nodes = ui_nodes_get_nodes();
			ui_node_canvas_t *c     = ui_nodes_get_canvas(true);
			if (nodes->nodes_selected_id->length > 0) {
				ui_node_t     *sel = ui_get_node(c->nodes, nodes->nodes_selected_id->buffer[0]);
				gpu_texture_t *img = ui_nodes_get_node_preview_image(sel);
				if (img != NULL) {
					tex = ui_nodes_get_node_preview_image(sel);
				}
			}
		}
		else if (ui_view2d_type == VIEW_2D_TYPE_LAYER) {
			slot_layer_t *layer = l;

			if (config_raw->brush_live && render_path_paint_live_layer_drawn > 0) {
				layer = render_path_paint_live_layer;
			}
			if (context_raw->tool == TOOL_TYPE_MATERIAL) {
				layer = render_path_paint_live_layer;
			}
			if (ui_view2d_layer_mode == VIEW_2D_LAYER_MODE_VISIBLE) {
				gpu_texture_t *current = _draw_current;
				bool           in_use  = gpu_in_use;
				if (in_use)
					draw_end();
				layer = layers_flatten(false, NULL);
				if (in_use)
					draw_begin(current, false, 0);
			}
			else if (slot_layer_is_group(layer)) {
				gpu_texture_t *current = _draw_current;
				bool           in_use  = gpu_in_use;
				if (in_use)
					draw_end();
				layer = layers_flatten(false, slot_layer_get_children(layer));
				if (in_use)
					draw_begin(current, false, 0);
			}

			tex = slot_layer_is_mask(context_raw->layer)    ? layer->texpaint
			      : ui_view2d_tex_type == PAINT_TEX_BASE    ? layer->texpaint
			      : ui_view2d_tex_type == PAINT_TEX_OPACITY ? layer->texpaint
			      : ui_view2d_tex_type == PAINT_TEX_NORMAL  ? layer->texpaint_nor
			                                                : layer->texpaint_pack;

			channel = slot_layer_is_mask(context_raw->layer)      ? 1
			          : ui_view2d_tex_type == PAINT_TEX_OCCLUSION ? 1
			          : ui_view2d_tex_type == PAINT_TEX_ROUGHNESS ? 2
			          : ui_view2d_tex_type == PAINT_TEX_METALLIC  ? 3
			          : ui_view2d_tex_type == PAINT_TEX_OPACITY   ? 4
			          : ui_view2d_tex_type == PAINT_TEX_HEIGHT    ? 4
			          : ui_view2d_tex_type == PAINT_TEX_NORMAL    ? 5
			                                                      : 0;
		}
		else if (ui_view2d_type == VIEW_2D_TYPE_FONT) {
			tex = context_raw->font->image;
		}

		i32 th = tw;
		if (tex != NULL) {
			th = tw * (tex->height / (float)tex->width);
			ty = apph / 2.0 - th / 2.0 + ui_view2d_pan_y;
			if (ui_view2d_type == VIEW_2D_TYPE_LAYER) {
				draw_set_pipeline(ui_view2d_pipe);
			}

			ui_view2d_draw_image(tex, tx, ty, tw, th, channel);
			if (ui_view2d_tiled_show) {
				ui_view2d_draw_image(tex, tx - tw, ty, tw, th, channel);
				ui_view2d_draw_image(tex, tx - tw, ty - th, tw, th, channel);
				ui_view2d_draw_image(tex, tx - tw, ty + th, tw, th, channel);
				ui_view2d_draw_image(tex, tx + tw, ty, tw, th, channel);
				ui_view2d_draw_image(tex, tx + tw, ty - th, tw, th, channel);
				ui_view2d_draw_image(tex, tx + tw, ty + th, tw, th, channel);
				ui_view2d_draw_image(tex, tx, ty - th, tw, th, channel);
				ui_view2d_draw_image(tex, tx, ty + th, tw, th, channel);
			}

			if (ui_view2d_type == VIEW_2D_TYPE_LAYER) {
				draw_set_pipeline(NULL);
			}

			// Texture and node preview color picking
			if ((context_in_2d_view(VIEW_2D_TYPE_ASSET) || context_in_2d_view(VIEW_2D_TYPE_NODE)) && context_raw->tool == TOOL_TYPE_PICKER && ui->input_down) {
				gc_unroot(_ui_view2d_render_tex);
				_ui_view2d_render_tex = tex;
				gc_root(_ui_view2d_render_tex);
				_ui_view2d_render_x = ui->input_x - tx - ui_view2d_wx;
				;
				_ui_view2d_render_y  = ui->input_y - ty - ui_view2d_wy;
				_ui_view2d_render_tw = tw;
				_ui_view2d_render_th = th;
				sys_notify_on_next_frame(&ui_view2d_render_color_pick, NULL);
			}
		}

		// UV map
		if (ui_view2d_type == VIEW_2D_TYPE_LAYER && ui_view2d_uvmap_show) {
			draw_scaled_image(util_uv_uvmap, tx, ty, tw, th);
		}

		// Menu
		i32 top_y = ui_menu_top_y();
		i32 ew    = math_floor(UI_ELEMENT_W());
		draw_set_color(ui->ops->theme->WINDOW_BG_COL);
		draw_filled_rect(0, top_y, ui_view2d_ww, UI_ELEMENT_H() + UI_ELEMENT_OFFSET() * 2);
		draw_set_color(0xffffffff);

		f32 start_y = top_y + UI_ELEMENT_OFFSET();
		ui->_x      = 2;
		ui->_y      = 2 + start_y;
		ui->_w      = ew;

		// Editable layer name
		ui_handle_t *h    = ui_handle(__ID__);
		char        *text = ui_view2d_type == VIEW_2D_TYPE_NODE ? context_raw->node_preview_name : h->text;

		ui->_w = math_floor(math_min(draw_string_width(ui->ops->font, ui->font_size, text) + 15 * UI_SCALE(), 100 * UI_SCALE()));

		if (ui_view2d_type == VIEW_2D_TYPE_ASSET) {
			asset_t *asset = context_raw->texture;
			if (asset != NULL) {
				string_t_array_t *asset_names = project_asset_names;
				i32               i           = string_array_index_of(asset_names, asset->name);
				h->text                       = string_copy(asset->name);
				asset->name                   = string_copy(ui_text_input(h, "", UI_ALIGN_LEFT, true, false));
				asset_names->buffer[i]        = asset->name;
			}
		}
		else if (ui_view2d_type == VIEW_2D_TYPE_NODE) {
			ui_text(context_raw->node_preview_name, UI_ALIGN_LEFT, 0x00000000);
		}
		else if (ui_view2d_type == VIEW_2D_TYPE_LAYER) {
			h->text                    = string_copy(l->name);
			l->name                    = string_copy(ui_text_input(h, "", UI_ALIGN_LEFT, true, false));
			ui_view2d_text_input_hover = ui->is_hovered;
		}
		else if (ui_view2d_type == VIEW_2D_TYPE_FONT) {
			h->text                 = string_copy(context_raw->font->name);
			context_raw->font->name = ui_text_input(h, "", UI_ALIGN_LEFT, true, false);
		}

		if (h->changed) {
			ui_base_hwnds->buffer[0]->redraws = 2;
		}
		ui->_x += ui->_w + 3;
		ui->_y = 2 + start_y;
		ui->_w = ew;

		if (ui_view2d_type == VIEW_2D_TYPE_LAYER) {
			ui_handle_t *h_layer_mode = ui_handle(__ID__);
			if (h_layer_mode->init) {
				h_layer_mode->i = ui_view2d_layer_mode;
			}
			string_t_array_t *layer_mode_combo = any_array_create_from_raw(
			    (void *[]){
			        tr("Visible"),
			        tr("Selected"),
			    },
			    2);
			ui_view2d_layer_mode = ui_combo(h_layer_mode, layer_mode_combo, tr("Layers"), false, UI_ALIGN_LEFT, true);
			ui->_x += ew + 3;
			ui->_y = 2 + start_y;

			if (!slot_layer_is_mask(context_raw->layer)) {
				ui_handle_t *h_tex_type = ui_handle(__ID__);
				if (h_tex_type->init) {
					h_tex_type->i = ui_view2d_tex_type;
				}
				string_t_array_t *tex_type_combo = any_array_create_from_raw(
				    (void *[]){
				        tr("Base Color"),
						tr("Opacity"),
				        tr("Normal Map"),
				        tr("Occlusion"),
				        tr("Roughness"),
				        tr("Metallic"),
				        tr("Height"),
				    },
				    7);

				if (config_raw->workflow == WORKFLOW_BASE) {
					array_splice(tex_type_combo, 6, 1);
					array_splice(tex_type_combo, 5, 1);
					array_splice(tex_type_combo, 4, 1);
					array_splice(tex_type_combo, 3, 1);
					array_splice(tex_type_combo, 2, 1);
				}

				ui_view2d_tex_type = ui_combo(h_tex_type, tex_type_combo, tr("Texture"), false, UI_ALIGN_LEFT, true);
				ui->_x += ew + 3;
				ui->_y = 2 + start_y;
			}

			ui->_w                    = math_floor(ew * 0.7 + 3);
			ui_handle_t *h_uvmap_show = ui_handle(__ID__);
			if (h_uvmap_show->init) {
				h_uvmap_show->b = ui_view2d_uvmap_show;
			}
			ui_view2d_uvmap_show = ui_check(h_uvmap_show, tr("UV Map"), "");
			ui->_x += ew * 0.7 + 3;
			ui->_y = 2 + start_y;
		}

		ui->_w                    = math_floor(ew * 0.7 + 3);
		ui_handle_t *h_tiled_show = ui_handle(__ID__);
		if (h_tiled_show->init) {
			h_tiled_show->b = ui_view2d_tiled_show;
		}
		ui_view2d_tiled_show = ui_check(h_tiled_show, tr("Tiled"), "");
		ui->_x += ew * 0.6 + 3;
		ui->_y = 2 + start_y;

		bool full = true;

#ifdef IRON_IOS
		if (config_is_iphone()) {
			full = false;
		}
#endif

		if (full) {
			if (tex != NULL) {
				ui->_w            = math_floor(ew * 0.5 + 3);
				i32 scale_percent = math_round((tw / (float)tex->width) * 100);
				if (ui_text(string("%d%%", scale_percent), UI_ALIGN_LEFT, 0x00000000) == UI_STATE_STARTED) {
					ui_view2d_pan_scale = tex->width / (float)(ui_view2d_ww * 0.95);
				}
				ui->_x += ew * 0.5 + 3;
				ui->_y = 2 + start_y;
			}

			ui->enabled = false;

			if ((ui_view2d_type == VIEW_2D_TYPE_ASSET || ui_view2d_type == VIEW_2D_TYPE_NODE) && tex != NULL) { // Texture resolution
				ui->_w = math_floor(ew * 0.7 + 3);
				ui_text(string("%dx%d", tex->width, tex->height), UI_ALIGN_LEFT, 0x00000000);
				ui->_x += ew * 0.7 + 3;
				ui->_y = 2 + start_y;
			}

			char *view_type = ui_view2d_type == VIEW_2D_TYPE_ASSET  ? "Asset"
			                  : ui_view2d_type == VIEW_2D_TYPE_NODE ? "Node"
			                  : ui_view2d_type == VIEW_2D_TYPE_FONT ? "Font"
			                                                        : "Layer";
			ui->_w          = math_floor(ew * 0.5 + 3);
			ui_text(view_type, UI_ALIGN_LEFT, 0x00000000);
			ui->_x += ew * 0.5 + 3;
			ui->_y = 2 + start_y;

			ui->enabled = true;
		}

		// Picked position
		if (context_raw->tool == TOOL_TYPE_PICKER && (ui_view2d_type == VIEW_2D_TYPE_LAYER || ui_view2d_type == VIEW_2D_TYPE_ASSET)) {
			gpu_texture_t *cursor_img = resource_get("cursor.k");
			f32            hsize      = 16 * UI_SCALE();
			f32            size       = hsize * 2;
			draw_scaled_image(cursor_img, tx + tw * context_raw->uvx_picked - hsize, ty + th * context_raw->uvy_picked - hsize, size, size);
		}
	}
	ui_end();

	ui->input_enabled = true;
}

void ui_view2d_update(void *_) {
	f32 headerh = UI_ELEMENT_H() * 1.4;

	context_raw->paint2d = false;

	if (!base_ui_enabled || !ui_view2d_show || mouse_x < ui_view2d_wx || mouse_x > ui_view2d_wx + ui_view2d_ww || mouse_y < ui_view2d_wy + headerh ||
	    mouse_y > ui_view2d_wy + ui_view2d_wh) {
		if (ui_view2d_controls_down) {
			ui_canvas_control_t *control = ui_nodes_get_canvas_control(ui_view2d_controls_down);
			ui_view2d_controls_down      = control->controls_down;
		}
		return;
	}

	ui_canvas_control_t *control = ui_nodes_get_canvas_control(ui_view2d_controls_down);
	ui_view2d_pan_x += control->pan_x;
	ui_view2d_pan_y += control->pan_y;
	ui_view2d_controls_down = control->controls_down;
	if (control->zoom != 0.0) {
		f32 _pan_x = ui_view2d_pan_x / (float)ui_view2d_pan_scale;
		f32 _pan_y = ui_view2d_pan_y / (float)ui_view2d_pan_scale;
		ui_view2d_pan_scale += control->zoom;
		if (ui_view2d_pan_scale < 0.1) {
			ui_view2d_pan_scale = 0.1;
		}
		if (ui_view2d_pan_scale > 6.0) {
			ui_view2d_pan_scale = 6.0;
		}
		ui_view2d_pan_x = _pan_x * ui_view2d_pan_scale;
		ui_view2d_pan_y = _pan_y * ui_view2d_pan_scale;

		if (ui_touch_control) {
			// Zoom to finger location
			ui_view2d_pan_x -= (ui->input_x - ui->_window_x - ui->_window_w / 2.0) * control->zoom;
			ui_view2d_pan_y -= (ui->input_y - ui->_window_y - ui->_window_h / 2.0) * control->zoom;
		}
		ui_view2d_grid_redraw = true;
	}

	bool decal_mask = context_is_decal_mask_paint();
	bool set_clone_source =
	    context_raw->tool == TOOL_TYPE_CLONE &&
	    operator_shortcut(string("%s+%s", any_map_get(config_keymap, "set_clone_source"), any_map_get(config_keymap, "action_paint")), SHORTCUT_TYPE_DOWN);
	bool bake = context_raw->tool == TOOL_TYPE_BAKE;

	if (!ui->input_down) {
		ui_view2d_layer_touched = false;
	}

	if (ui_view2d_type == VIEW_2D_TYPE_LAYER && !ui_view2d_text_input_hover && !bake &&
	    (operator_shortcut(any_map_get(config_keymap, "action_paint"), SHORTCUT_TYPE_DOWN) ||
	     operator_shortcut(string("%s+%s", any_map_get(config_keymap, "brush_ruler"), any_map_get(config_keymap, "action_paint")), SHORTCUT_TYPE_DOWN) ||
	     decal_mask || set_clone_source || config_raw->brush_live)) {

		if (config_raw->touch_ui) {
			// Paint only when clicking on the layer rect
			slot_layer_t  *layer   = context_raw->layer;
			gpu_texture_t *tex     = layer->texpaint;
			f32            ratio   = tex->height / (float)tex->width;
			f32            wm      = fmin(ui_view2d_ww, ui_view2d_wh);
			f32            tw      = wm * 0.9 * ui_view2d_pan_scale;
			f32            th      = tw * ratio;
			f32            tx      = ui_view2d_ww / 2.0 - tw / 2.0 + ui_view2d_pan_x;
			i32            headerh = config_raw->layout->buffer[LAYOUT_SIZE_HEADER] == 1 ? ui_header_h * 2 : ui_header_h;
			i32            apph    = iron_window_height() - config_raw->layout->buffer[LAYOUT_SIZE_STATUS_H] + headerh;
			f32            ty      = apph / 2.0 - th / 2.0 + ui_view2d_pan_y;
			f32            mx      = mouse_x - ui_view2d_wx;
			f32            my      = mouse_y - ui_view2d_wy;
			if (mx > tx && mx < tx + tw && my > ty && my < ty + th) {
				ui_view2d_layer_touched = true;
			}
			if (ui_view2d_layer_touched) {
				context_raw->paint2d = true;
			}
		}
		else {
			context_raw->paint2d = true;
		}
	}

	if (ui->is_typing) {
		return;
	}

	if (keyboard_started("left")) {
		ui_view2d_pan_x -= 5;
	}
	else if (keyboard_started("right")) {
		ui_view2d_pan_x += 5;
	}
	if (keyboard_started("up")) {
		ui_view2d_pan_y -= 5;
	}
	else if (keyboard_started("down")) {
		ui_view2d_pan_y += 5;
	}

	if (!context_raw->paint2d && config_raw->touch_ui && ui->input_down) {
		ui_view2d_pan_x += ui->input_dx;
		ui_view2d_pan_y += ui->input_dy;
	}

	// Limit panning to keep texture in viewport
	i32 border = 32;
	f32 wm     = fmin(ui_view2d_ww, ui_view2d_wh);
	f32 tw     = ui_view2d_ww * 0.9 * ui_view2d_pan_scale;
	f32 tx     = ui_view2d_ww / 2.0 - tw / 2.0 + ui_view2d_pan_x;
	f32 hh     = sys_h();
	f32 ty     = hh / 2.0 - tw / 2.0 + ui_view2d_pan_y;

	if (tx + border > ui_view2d_ww) {
		ui_view2d_pan_x = ui_view2d_ww / 2.0 + tw / 2.0 - border;
	}
	else if (tx - border < -tw) {
		ui_view2d_pan_x = -tw / 2.0 - ui_view2d_ww / 2.0 + border;
	}
	if (ty + border > hh) {
		ui_view2d_pan_y = hh / 2.0 + tw / 2.0 - border;
	}
	else if (ty - border < -tw) {
		ui_view2d_pan_y = -tw / 2.0 - hh / 2.0 + border;
	}

	if (operator_shortcut(any_map_get(config_keymap, "view_reset"), SHORTCUT_TYPE_STARTED)) {
		ui_view2d_pan_x     = 0.0;
		ui_view2d_pan_y     = 0.0;
		ui_view2d_pan_scale = 1.0;
	}
}
