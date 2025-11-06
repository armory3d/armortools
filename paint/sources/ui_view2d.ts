
let ui_view2d_pipe: gpu_pipeline_t;
let ui_view2d_channel_loc: i32;
let ui_view2d_text_input_hover: bool           = false;
let ui_view2d_uvmap_show: bool                 = false;
let ui_view2d_tex_type: paint_tex_t            = paint_tex_t.BASE;
let ui_view2d_layer_mode: view_2d_layer_mode_t = view_2d_layer_mode_t.SELECTED;
let ui_view2d_type: view_2d_type_t             = view_2d_type_t.LAYER;
let ui_view2d_show: bool                       = false;
let ui_view2d_wx: i32;
let ui_view2d_wy: i32;
let ui_view2d_ww: i32;
let ui_view2d_wh: i32;
let ui_view2d_hwnd: ui_handle_t   = ui_handle_create();
let ui_view2d_pan_x: f32          = 0.0;
let ui_view2d_pan_y: f32          = 0.0;
let ui_view2d_pan_scale: f32      = 1.0;
let ui_view2d_tiled_show: bool    = false;
let ui_view2d_controls_down: bool = false;
let _ui_view2d_render_tex: gpu_texture_t;
let _ui_view2d_render_x: f32;
let _ui_view2d_render_y: f32;
let _ui_view2d_render_tw: f32;
let _ui_view2d_render_th: f32;
let ui_view2d_grid: gpu_texture_t = null;
let ui_view2d_grid_redraw: bool   = true;
let ui_view2d_htab: ui_handle_t   = ui_handle_create();

function ui_view2d_init() {
	ui_view2d_pipe                 = gpu_create_pipeline();
	ui_view2d_pipe.vertex_shader   = sys_get_shader("layer_view.vert");
	ui_view2d_pipe.fragment_shader = sys_get_shader("layer_view.frag");
	let vs: gpu_vertex_structure_t = {};
	gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
	ui_view2d_pipe.input_layout                            = vs;
	ui_view2d_pipe.blend_source                            = blend_factor_t.BLEND_ONE;
	ui_view2d_pipe.blend_destination                       = blend_factor_t.BLEND_ZERO;
	ARRAY_ACCESS(ui_view2d_pipe.color_write_mask_alpha, 0) = false;
	gpu_pipeline_compile(ui_view2d_pipe);
	pipes_offset = 0;
	pipes_get_constant_location("float4");
	pipes_get_constant_location("float4");
	pipes_get_constant_location("float4");
	ui_view2d_channel_loc = pipes_get_constant_location("int");
}

function ui_view2d_draw_image(image: gpu_texture_t, dx: f32, dy: f32, dw: f32, dh: f32, channel: i32) {
	if (ui_view2d_type == view_2d_type_t.LAYER) {
		gpu_set_int(ui_view2d_channel_loc, channel);
	}
	draw_scaled_image(image, dx, dy, dw, dh);
}

function ui_view2d_render() {
	ui_view2d_ww = config_raw.layout[layout_size_t.NODES_W];
	ui_view2d_wx = math_floor(sys_w()) + ui_toolbar_w(true);
	ui_view2d_wy = 0;

	if (!ui_base_show) {
		ui_view2d_ww += config_raw.layout[layout_size_t.SIDEBAR_W] + ui_toolbar_w(true);
		ui_view2d_wx -= ui_toolbar_w(true);
	}
	if (!base_view3d_show) {
		ui_view2d_ww += base_view3d_w();
	}

	if (!ui_view2d_show) {
		return;
	}

	if (context_raw.pdirty >= 0) {
		ui_view2d_hwnd.redraws = 2; // Paint was active
	}

	// Cache grid
	if (ui_view2d_grid_redraw) {
		if (ui_view2d_grid != null) {
			gpu_delete_texture(ui_view2d_grid);
		}
		ui_view2d_grid        = ui_nodes_draw_grid(ui_view2d_pan_scale);
		ui_view2d_grid_redraw = false;
	}

	// Ensure UV map is drawn
	if (ui_view2d_uvmap_show) {
		util_uv_cache_uv_map();
	}

	// Ensure font image is drawn
	if (context_raw.font.image == null) {
		util_render_make_font_preview();
	}

	ui_begin(ui);

	let headerh: i32 = config_raw.layout[layout_size_t.HEADER] == 1 ? ui_header_h * 2 : ui_header_h;
	let apph: i32    = iron_window_height() - config_raw.layout[layout_size_t.STATUS_H] + headerh;
	if (!base_view3d_show) {
		apph = base_h();
	}
	ui_view2d_wh = iron_window_height() - config_raw.layout[layout_size_t.STATUS_H];

	if (ui_nodes_show) {
		ui_view2d_wh -= config_raw.layout[layout_size_t.NODES_H];
		if (config_raw.touch_ui) {
			ui_view2d_wh += ui_header_h;
		}
	}

	if (ui_window(ui_view2d_hwnd, ui_view2d_wx, ui_view2d_wy, ui_view2d_ww, ui_view2d_wh)) {

		ui_tab(ui_view2d_htab, tr("2D View"), false, -1, !base_view3d_show);

		if (ui_tab(ui_view2d_htab, tr("+"))) {
			ui_view2d_htab.i = 0;
		}

		// Grid
		draw_set_color(0xffffffff);
		let step: f32 = ui_nodes_grid_cell_w * ui_view2d_pan_scale;
		let x: f32    = math_fmod(ui_view2d_pan_x, step) - step;
		let y: f32    = math_fmod(ui_view2d_pan_y, step) - step;
		draw_image(ui_view2d_grid, x, y);

		// Texture
		let tex: gpu_texture_t = null;
		let l: slot_layer_t    = context_raw.layer;
		let channel: i32       = 0;

		let tw: f32 = ui_view2d_ww * 0.95 * ui_view2d_pan_scale;
		let tx: f32 = ui_view2d_ww / 2 - tw / 2 + ui_view2d_pan_x;
		let ty: f32 = apph / 2 - tw / 2 + ui_view2d_pan_y;

		if (ui_view2d_type == view_2d_type_t.ASSET) {
			tex = project_get_image(context_raw.texture);
		}
		else if (ui_view2d_type == view_2d_type_t.NODE) {
			let nodes: ui_nodes_t   = ui_nodes_get_nodes();
			let c: ui_node_canvas_t = ui_nodes_get_canvas(true);
			if (nodes.nodes_selected_id.length > 0) {
				let sel: ui_node_t     = ui_get_node(c.nodes, nodes.nodes_selected_id[0]);
				let img: gpu_texture_t = ui_nodes_get_node_preview_image(sel);
				if (img != null) {
					tex = ui_nodes_get_node_preview_image(sel);
				}
			}
		}
		else if (ui_view2d_type == view_2d_type_t.LAYER) {
			let layer: slot_layer_t = l;

			if (config_raw.brush_live && render_path_paint_live_layer_drawn > 0) {
				layer = render_path_paint_live_layer;
			}

			if (context_raw.tool == tool_type_t.MATERIAL) {
				layer = render_path_paint_live_layer;
			}

			if (ui_view2d_layer_mode == view_2d_layer_mode_t.VISIBLE) {
				let current: gpu_texture_t = _draw_current;
				let in_use: bool           = gpu_in_use;
				if (in_use)
					draw_end();
				layer = layers_flatten();
				if (in_use)
					draw_begin(current);
			}
			else if (slot_layer_is_group(layer)) {
				let current: gpu_texture_t = _draw_current;
				let in_use: bool           = gpu_in_use;
				if (in_use)
					draw_end();
				layer = layers_flatten(false, slot_layer_get_children(layer));
				if (in_use)
					draw_begin(current);
			}

			tex = slot_layer_is_mask(context_raw.layer)       ? layer.texpaint
			      : ui_view2d_tex_type == paint_tex_t.BASE    ? layer.texpaint
			      : ui_view2d_tex_type == paint_tex_t.OPACITY ? layer.texpaint
			      : ui_view2d_tex_type == paint_tex_t.NORMAL  ? layer.texpaint_nor
			                                                  : layer.texpaint_pack;

			channel = slot_layer_is_mask(context_raw.layer)         ? 1
			          : ui_view2d_tex_type == paint_tex_t.OCCLUSION ? 1
			          : ui_view2d_tex_type == paint_tex_t.ROUGHNESS ? 2
			          : ui_view2d_tex_type == paint_tex_t.METALLIC  ? 3
			          : ui_view2d_tex_type == paint_tex_t.OPACITY   ? 4
			          : ui_view2d_tex_type == paint_tex_t.HEIGHT    ? 4
			          : ui_view2d_tex_type == paint_tex_t.NORMAL    ? 5
			                                                        : 0;
		}
		else if (ui_view2d_type == view_2d_type_t.FONT) {
			tex = context_raw.font.image;
		}

		let th: f32 = tw;
		if (tex != null) {
			th = tw * (tex.height / tex.width);
			ty = apph / 2 - th / 2 + ui_view2d_pan_y;
			if (ui_view2d_type == view_2d_type_t.LAYER) {
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

			if (ui_view2d_type == view_2d_type_t.LAYER) {
				draw_set_pipeline(null);
			}

			// Texture and node preview color picking
			if ((context_in_2d_view(view_2d_type_t.ASSET) || context_in_2d_view(view_2d_type_t.NODE)) && context_raw.tool == tool_type_t.PICKER &&
			    ui.input_down) {
				_ui_view2d_render_tex = tex;
				_ui_view2d_render_x   = ui.input_x - tx - ui_view2d_wx;
				;
				_ui_view2d_render_y  = ui.input_y - ty - ui_view2d_wy;
				_ui_view2d_render_tw = tw;
				_ui_view2d_render_th = th;

				sys_notify_on_next_frame(function() {
					let rt: render_target_t            = map_get(render_path_render_targets, "texpaint_picker");
					let texpaint_picker: gpu_texture_t = rt._image;
					draw_begin(texpaint_picker);
					draw_scaled_image(_ui_view2d_render_tex, -_ui_view2d_render_x, -_ui_view2d_render_y, _ui_view2d_render_tw, _ui_view2d_render_th);
					draw_end();
					let a: buffer_t = gpu_get_texture_pixels(texpaint_picker);
					/// if IRON_BGRA
					let i0: i32 = 2;
					let i1: i32 = 1;
					let i2: i32 = 0;
					/// else
					let i0: i32 = 0;
					let i1: i32 = 1;
					let i2: i32 = 2;
					/// end

					context_raw.picked_color.base = color_set_rb(context_raw.picked_color.base, buffer_get_u8(a, i0));
					context_raw.picked_color.base = color_set_gb(context_raw.picked_color.base, buffer_get_u8(a, i1));
					context_raw.picked_color.base = color_set_bb(context_raw.picked_color.base, buffer_get_u8(a, i2));
					ui_header_handle.redraws      = 2;

					if (context_raw.color_picker_callback != null) {
						context_raw.color_picker_callback(context_raw.picked_color);
					}
				});
			}
		}

		// UV map
		if (ui_view2d_type == view_2d_type_t.LAYER && ui_view2d_uvmap_show) {
			draw_scaled_image(util_uv_uvmap, tx, ty, tw, th);
		}

		// Menu
		let ew: i32 = math_floor(UI_ELEMENT_W());
		draw_set_color(ui.ops.theme.WINDOW_BG_COL);
		draw_filled_rect(0, UI_ELEMENT_H(), ui_view2d_ww, UI_ELEMENT_H() + UI_ELEMENT_OFFSET() * 2);
		draw_set_color(0xffffffff);

		let start_y: f32 = UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
		ui._x            = 2;
		ui._y            = 2 + start_y;
		ui._w            = ew;

		// Editable layer name
		let h: ui_handle_t = ui_handle(__ID__);
		let text: string   = ui_view2d_type == view_2d_type_t.NODE ? context_raw.node_preview_name : h.text;

		ui._w = math_floor(math_min(draw_string_width(ui.ops.font, ui.font_size, text) + 15 * UI_SCALE(), 100 * UI_SCALE()));

		if (ui_view2d_type == view_2d_type_t.ASSET) {
			let asset: asset_t = context_raw.texture;
			if (asset != null) {
				let asset_names: string[] = project_asset_names;
				let i: i32                = array_index_of(asset_names, asset.name);
				h.text                    = asset.name;
				asset.name                = ui_text_input(h, "");
				asset_names[i]            = asset.name;
			}
		}
		else if (ui_view2d_type == view_2d_type_t.NODE) {
			ui_text(context_raw.node_preview_name);
		}
		else if (ui_view2d_type == view_2d_type_t.LAYER) {
			h.text                     = l.name;
			l.name                     = ui_text_input(h, "");
			ui_view2d_text_input_hover = ui.is_hovered;
		}
		else if (ui_view2d_type == view_2d_type_t.FONT) {
			h.text                = context_raw.font.name;
			context_raw.font.name = ui_text_input(h, "");
		}

		if (h.changed) {
			ui_base_hwnds[0].redraws = 2;
		}
		ui._x += ui._w + 3;
		ui._y = 2 + start_y;
		ui._w = ew;

		if (ui_view2d_type == view_2d_type_t.LAYER) {
			let h_layer_mode: ui_handle_t = ui_handle(__ID__);
			if (h_layer_mode.init) {
				h_layer_mode.i = ui_view2d_layer_mode;
			}
			let layer_mode_combo: string[] = [ tr("Visible"), tr("Selected") ];
			ui_view2d_layer_mode           = ui_combo(h_layer_mode, layer_mode_combo, tr("Layers"));
			ui._x += ew + 3;
			ui._y = 2 + start_y;

			if (!slot_layer_is_mask(context_raw.layer)) {
				let h_tex_type: ui_handle_t = ui_handle(__ID__);
				if (h_tex_type.init) {
					h_tex_type.i = ui_view2d_tex_type;
				}
				let tex_type_combo: string[] = [
					tr("Base Color"),
					tr("Normal Map"),
					tr("Occlusion"),
					tr("Roughness"),
					tr("Metallic"),
					tr("Opacity"),
					tr("Height"),
				];
				ui_view2d_tex_type = ui_combo(h_tex_type, tex_type_combo, tr("Texture"));
				ui._x += ew + 3;
				ui._y = 2 + start_y;
			}

			ui._w                         = math_floor(ew * 0.7 + 3);
			let h_uvmap_show: ui_handle_t = ui_handle(__ID__);
			if (h_uvmap_show.init) {
				h_uvmap_show.b = ui_view2d_uvmap_show;
			}
			ui_view2d_uvmap_show = ui_check(h_uvmap_show, tr("UV Map"));
			ui._x += ew * 0.7 + 3;
			ui._y = 2 + start_y;
		}

		let h_tiled_show: ui_handle_t = ui_handle(__ID__);
		if (h_tiled_show.init) {
			h_tiled_show.b = ui_view2d_tiled_show;
		}
		ui_view2d_tiled_show = ui_check(h_tiled_show, tr("Tiled"));
		ui._x += ew * 0.7 + 3;
		ui._y = 2 + start_y;

		if ((ui_view2d_type == view_2d_type_t.ASSET || ui_view2d_type == view_2d_type_t.NODE) && tex != null) { // Texture resolution
			ui_text(tex.width + "x" + tex.height);
			ui._x += ew * 0.7 + 3;
			ui._y = 2 + start_y;
		}

		ui.enabled            = false;
		let view_type: string = ui_view2d_type == view_2d_type_t.ASSET  ? "Asset"
		                        : ui_view2d_type == view_2d_type_t.NODE ? "Node"
		                        : ui_view2d_type == view_2d_type_t.FONT ? "Font"
		                                                                : "Layer";
		ui_text(view_type);
		ui.enabled = true;

		// Picked position
		if (context_raw.tool == tool_type_t.PICKER && (ui_view2d_type == view_2d_type_t.LAYER || ui_view2d_type == view_2d_type_t.ASSET)) {
			let cursor_img: gpu_texture_t = resource_get("cursor.k");
			let hsize: f32                = 16 * UI_SCALE();
			let size: f32                 = hsize * 2;
			draw_scaled_image(cursor_img, tx + tw * context_raw.uvx_picked - hsize, ty + th * context_raw.uvy_picked - hsize, size, size);
		}
	}
	ui_end();
}

function ui_view2d_update() {
	let headerh: f32 = UI_ELEMENT_H() * 1.4;

	context_raw.paint2d = false;

	if (!base_ui_enabled || !ui_view2d_show || mouse_x < ui_view2d_wx || mouse_x > ui_view2d_wx + ui_view2d_ww || mouse_y < ui_view2d_wy + headerh ||
	    mouse_y > ui_view2d_wy + ui_view2d_wh) {
		if (ui_view2d_controls_down) {
			let control: ui_canvas_control_t = ui_nodes_get_canvas_control(ui_view2d_controls_down);
			ui_view2d_controls_down          = control.controls_down;
		}
		return;
	}

	let control: ui_canvas_control_t = ui_nodes_get_canvas_control(ui_view2d_controls_down);
	ui_view2d_pan_x += control.pan_x;
	ui_view2d_pan_y += control.pan_y;
	ui_view2d_controls_down = control.controls_down;
	if (control.zoom != 0.0) {
		let _pan_x: f32 = ui_view2d_pan_x / ui_view2d_pan_scale;
		let _pan_y: f32 = ui_view2d_pan_y / ui_view2d_pan_scale;
		ui_view2d_pan_scale += control.zoom;
		if (ui_view2d_pan_scale < 0.1) {
			ui_view2d_pan_scale = 0.1;
		}
		if (ui_view2d_pan_scale > 6.0) {
			ui_view2d_pan_scale = 6.0;
		}
		ui_view2d_pan_x = _pan_x * ui_view2d_pan_scale;
		ui_view2d_pan_y = _pan_y * ui_view2d_pan_scale;

		if (ui_touch_scroll) {
			// Zoom to finger location
			ui_view2d_pan_x -= (ui.input_x - ui._window_x - ui._window_w / 2) * control.zoom;
			ui_view2d_pan_y -= (ui.input_y - ui._window_y - ui._window_h / 2) * control.zoom;
		}
		ui_view2d_grid_redraw = true;
	}

	let decal_mask: bool = context_is_decal_mask_paint();
	let set_clone_source: bool =
	    context_raw.tool == tool_type_t.CLONE &&
	    operator_shortcut(map_get(config_keymap, "set_clone_source") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN);
	let bake: bool = context_raw.tool == tool_type_t.BAKE;

	if (ui_view2d_type == view_2d_type_t.LAYER && !ui_view2d_text_input_hover && !bake &&
	    (operator_shortcut(map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) ||
	     operator_shortcut(map_get(config_keymap, "brush_ruler") + "+" + map_get(config_keymap, "action_paint"), shortcut_type_t.DOWN) || decal_mask ||
	     set_clone_source || config_raw.brush_live)) {

		// Same mapping for paint and rotate (predefined in touch keymap)
		let paint_key: string  = map_get(config_keymap, "action_paint");
		let rotate_key: string = map_get(config_keymap, "action_rotate");
		if (paint_key == rotate_key) {
			// Paint only when clicking on the layer rect
			let layer: slot_layer_t = context_raw.layer;
			let tex: gpu_texture_t  = layer.texpaint;
			let ratio: f32          = tex.height / tex.width;
			let tw: f32             = ui_view2d_ww * 0.95 * ui_view2d_pan_scale;
			let th: f32             = tw * ratio;
			let tx: f32             = ui_view2d_ww / 2 - tw / 2 + ui_view2d_pan_x;
			let headerh: i32        = config_raw.layout[layout_size_t.HEADER] == 1 ? ui_header_h * 2 : ui_header_h;
			let apph: i32           = iron_window_height() - config_raw.layout[layout_size_t.STATUS_H] + headerh;
			let ty: f32             = apph / 2 - th / 2 + ui_view2d_pan_y;
			let mx: f32             = mouse_x - ui_view2d_wx;
			let my: f32             = mouse_y - ui_view2d_wy;
			if (mx > tx && mx < tx + tw && my > ty && my < ty + th) {
				context_raw.paint2d = true;
			}
		}
		else {
			context_raw.paint2d = true;
		}
	}

	if (ui.is_typing) {
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

	// Limit panning to keep texture in viewport
	let border: i32 = 32;
	let tw: f32     = ui_view2d_ww * 0.95 * ui_view2d_pan_scale;
	let tx: f32     = ui_view2d_ww / 2 - tw / 2 + ui_view2d_pan_x;
	let hh: f32     = sys_h();
	let ty: f32     = hh / 2 - tw / 2 + ui_view2d_pan_y;

	if (tx + border > ui_view2d_ww) {
		ui_view2d_pan_x = ui_view2d_ww / 2 + tw / 2 - border;
	}
	else if (tx - border < -tw) {
		ui_view2d_pan_x = -tw / 2 - ui_view2d_ww / 2 + border;
	}
	if (ty + border > hh) {
		ui_view2d_pan_y = hh / 2 + tw / 2 - border;
	}
	else if (ty - border < -tw) {
		ui_view2d_pan_y = -tw / 2 - hh / 2 + border;
	}

	if (operator_shortcut(map_get(config_keymap, "view_reset"))) {
		ui_view2d_pan_x     = 0.0;
		ui_view2d_pan_y     = 0.0;
		ui_view2d_pan_scale = 1.0;
	}
}
