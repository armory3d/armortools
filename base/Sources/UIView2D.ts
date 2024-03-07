
class UIView2D {

	///if (is_paint || is_sculpt)
	static pipe: pipeline_t;
	static channel_location: kinc_const_loc_t;
	static text_input_hover: bool = false;
	static uvmap_show: bool = false;
	static tex_type: paint_tex_t = paint_tex_t.BASE;
	static layer_mode: view_2d_layer_mode_t = view_2d_layer_mode_t.SELECTED;
	///end

	///if (is_paint || is_sculpt)
	static type: view_2d_type_t = view_2d_type_t.LAYER;
	///else
	static type: view_2d_type_t = view_2d_type_t.ASSET;
	///end

	static show: bool = false;
	static wx: i32;
	static wy: i32;
	static ww: i32;
	static wh: i32;
	static ui: zui_t;
	static hwnd: zui_handle_t = zui_handle_create();
	static pan_x: f32 = 0.0;
	static pan_y: f32 = 0.0;
	static pan_scale: f32 = 1.0;
	static tiled_show: bool = false;
	static controls_down: bool = false;

	constructor() {
		///if (is_paint || is_sculpt)
		UIView2D.pipe = g4_pipeline_create();
		UIView2D.pipe.vertex_shader = sys_get_shader("layer_view.vert");
		UIView2D.pipe.fragment_shader = sys_get_shader("layer_view.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		UIView2D.pipe.input_layout = [vs];
		UIView2D.pipe.blend_source = blend_factor_t.BLEND_ONE;
		UIView2D.pipe.blend_dest = blend_factor_t.BLEND_ZERO;
		UIView2D.pipe.color_write_masks_alpha[0] = false;
		g4_pipeline_compile(UIView2D.pipe);
		UIView2D.channel_location = g4_pipeline_get_const_loc(UIView2D.pipe, "channel");
		///end

		let scale: f32 = Config.raw.window_scale;
		UIView2D.ui = zui_create({ theme: Base.theme, font: Base.font, color_wheel: Base.color_wheel, black_white_gradient: Base.color_wheel_gradient, scale_factor: scale });
		UIView2D.ui.scroll_enabled = false;
	}

	static render = () => {

		UIView2D.ww = Config.raw.layout[layout_size_t.NODES_W];

		///if (is_paint || is_sculpt)
		UIView2D.wx = Math.floor(app_w()) + UIToolbar.toolbar_w;
		///else
		UIView2D.wx = Math.floor(app_w());
		///end

		UIView2D.wy = 0;

		///if (is_paint || is_sculpt)
		if (!UIBase.show) {
			UIView2D.ww += Config.raw.layout[layout_size_t.SIDEBAR_W] + UIToolbar.toolbar_w;
			UIView2D.wx -= UIToolbar.toolbar_w;
		}
		///end

		if (!UIView2D.show) return;
		if (sys_width() == 0 || sys_height() == 0) return;

		if (Context.raw.pdirty >= 0) UIView2D.hwnd.redraws = 2; // Paint was active

		g2_end();

		// Cache grid
		if (UINodes.grid == null) UINodes.draw_grid();

		// Ensure UV map is drawn
		///if (is_paint || is_sculpt)
		if (UIView2D.uvmap_show) UtilUV.cache_uv_map();
		///end

		// Ensure font image is drawn
		///if (is_paint || is_sculpt)
		if (Context.raw.font.image == null) UtilRender.make_font_preview();
		///end

		zui_begin(UIView2D.ui);

		let headerh: i32 = Config.raw.layout[layout_size_t.HEADER] == 1 ? UIHeader.headerh * 2 : UIHeader.headerh;
		let apph: i32 = sys_height() - Config.raw.layout[layout_size_t.STATUS_H] + headerh;
		UIView2D.wh = sys_height() - Config.raw.layout[layout_size_t.STATUS_H];

		if (UINodes.show) {
			UIView2D.wh -= Config.raw.layout[layout_size_t.NODES_H];
			if (Config.raw.touch_ui) UIView2D.wh += UIHeader.headerh;
		}

		if (zui_window(UIView2D.hwnd, UIView2D.wx, UIView2D.wy, UIView2D.ww, UIView2D.wh)) {

			zui_tab(zui_handle("uiview2d_0"), tr("2D View"));

			// Grid
			g2_set_color(0xffffffff);
			g2_draw_image(UINodes.grid, (UIView2D.pan_x * UIView2D.pan_scale) % 100 - 100, (UIView2D.pan_y * UIView2D.pan_scale) % 100 - 100);

			// Texture
			let tex: image_t = null;

			///if (is_paint || is_sculpt)
			let l: SlotLayerRaw = Context.raw.layer;
			let channel: i32 = 0;
			///end

			let tw: f32 = UIView2D.ww * 0.95 * UIView2D.pan_scale;
			let tx: f32 = UIView2D.ww / 2 - tw / 2 + UIView2D.pan_x;
			let ty: f32 = apph / 2 - tw / 2 + UIView2D.pan_y;

			if (UIView2D.type == view_2d_type_t.ASSET) {
				tex = Project.get_image(Context.raw.texture);
			}
			else if (UIView2D.type == view_2d_type_t.NODE) {
				///if (is_paint || is_sculpt)

				tex = Context.raw.node_preview;

				///else

				let nodes: zui_nodes_t = UINodes.get_nodes();
				if (nodes.nodesSelectedId.length > 0) {
					let sel: zui_node_t = zui_get_node(UINodes.get_canvas(true).nodes, nodes.nodesSelectedId[0]);
					let brushNode: LogicNode = ParserLogic.get_logic_node(sel);
					if (brushNode != null) {
						tex = brushNode.get_cached_image();
					}
				}

				///end
			}
			///if is_paint
			else if (UIView2D.type == view_2d_type_t.LAYER) {
				let layer: SlotLayerRaw = l;

				if (Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0) {
					layer = RenderPathPaint.liveLayer;
				}

				if (UIView2D.layer_mode == view_2d_layer_mode_t.VISIBLE) {
					let current: image_t = _g2_current;
					if (current != null) g2_end();
					layer = Base.flatten();
					if (current != null) g2_begin(current);
				}
				else if (SlotLayer.is_group(layer)) {
					let current: image_t = _g2_current;
					if (current != null) g2_end();
					layer = Base.flatten(false, SlotLayer.get_children(layer));
					if (current != null) g2_begin(current);
				}

				tex =
					SlotLayer.is_mask(Context.raw.layer) ? layer.texpaint :
					UIView2D.tex_type == paint_tex_t.BASE     ? layer.texpaint :
					UIView2D.tex_type == paint_tex_t.OPACITY  ? layer.texpaint :
					UIView2D.tex_type == paint_tex_t.NORMAL   ? layer.texpaint_nor :
														   layer.texpaint_pack;

				channel =
					SlotLayer.is_mask(Context.raw.layer)  ? 1 :
					UIView2D.tex_type == paint_tex_t.OCCLUSION ? 1 :
					UIView2D.tex_type == paint_tex_t.ROUGHNESS ? 2 :
					UIView2D.tex_type == paint_tex_t.METALLIC  ? 3 :
					UIView2D.tex_type == paint_tex_t.OPACITY   ? 4 :
					UIView2D.tex_type == paint_tex_t.HEIGHT    ? 4 :
					UIView2D.tex_type == paint_tex_t.NORMAL    ? 5 :
															0;
			}
			else if (UIView2D.type == view_2d_type_t.FONT) {
				tex = Context.raw.font.image;
			}
			///end

			let th: f32 = tw;
			if (tex != null) {
				th = tw * (tex.height / tex.width);
				ty = apph / 2 - th / 2 + UIView2D.pan_y;

				///if (is_paint || is_sculpt)
				if (UIView2D.type == view_2d_type_t.LAYER) {
					///if (!krom_opengl)
					g2_set_pipeline(UIView2D.pipe);
					///end
					if (!Context.raw.texture_filter) {
						g2_set_bilinear_filter(false);
					}
					///if krom_opengl
					krom_g4_set_pipeline(UIView2D.pipe.pipeline_);
					///end
					krom_g4_set_int(UIView2D.channel_location, channel);
				}
				///end

				g2_draw_scaled_image(tex, tx, ty, tw, th);

				if (UIView2D.tiled_show) {
					g2_draw_scaled_image(tex, tx - tw, ty, tw, th);
					g2_draw_scaled_image(tex, tx - tw, ty - th, tw, th);
					g2_draw_scaled_image(tex, tx - tw, ty + th, tw, th);
					g2_draw_scaled_image(tex, tx + tw, ty, tw, th);
					g2_draw_scaled_image(tex, tx + tw, ty - th, tw, th);
					g2_draw_scaled_image(tex, tx + tw, ty + th, tw, th);
					g2_draw_scaled_image(tex, tx, ty - th, tw, th);
					g2_draw_scaled_image(tex, tx, ty + th, tw, th);
				}

				///if (is_paint || is_sculpt)
				if (UIView2D.type == view_2d_type_t.LAYER) {
					g2_set_pipeline(null);
					if (!Context.raw.texture_filter) {
						g2_set_bilinear_filter(true);
					}
				}

				// Texture and node preview color picking
				if ((Context.in_2d_view(view_2d_type_t.ASSET) || Context.in_2d_view(view_2d_type_t.NODE)) && Context.raw.tool == workspace_tool_t.PICKER && UIView2D.ui.input_down) {
					let x: f32 = UIView2D.ui.input_x - tx - UIView2D.wx;
					let y: f32 = UIView2D.ui.input_y - ty - UIView2D.wy;
					Base.notify_on_next_frame(() => {
						let texpaint_picker: image_t = render_path_render_targets.get("texpaint_picker")._image;
						g2_begin(texpaint_picker);
						g2_draw_scaled_image(tex, -x, -y, tw, th);
						g2_end();
						let a: DataView = new DataView(image_get_pixels(texpaint_picker));
						///if (krom_metal || krom_vulkan)
						let i0: i32 = 2;
						let i1: i32 = 1;
						let i2: i32 = 0;
						///else
						let i0: i32 = 0;
						let i1: i32 = 1;
						let i2: i32 = 2;
						///end

						Context.raw.picked_color.base = color_set_rb(Context.raw.picked_color.base, a.getUint8(i0));
						Context.raw.picked_color.base = color_set_gb(Context.raw.picked_color.base, a.getUint8(i1));
						Context.raw.picked_color.base = color_set_bb(Context.raw.picked_color.base, a.getUint8(i2));
						UIHeader.header_handle.redraws = 2;
					});
				}
				///end
			}

			///if (is_paint || is_sculpt)
			// UV map
			if (UIView2D.type == view_2d_type_t.LAYER && UIView2D.uvmap_show) {
				g2_draw_scaled_image(UtilUV.uvmap, tx, ty, tw, th);
			}
			///end

			// Menu
			let ew: i32 = Math.floor(zui_ELEMENT_W(UIView2D.ui));
			g2_set_color(UIView2D.ui.t.SEPARATOR_COL);
			g2_fill_rect(0, zui_ELEMENT_H(UIView2D.ui), UIView2D.ww, zui_ELEMENT_H(UIView2D.ui) + zui_ELEMENT_OFFSET(UIView2D.ui) * 2);
			g2_set_color(0xffffffff);

			let startY: f32 = zui_ELEMENT_H(UIView2D.ui) + zui_ELEMENT_OFFSET(UIView2D.ui);
			UIView2D.ui._x = 2;
			UIView2D.ui._y = 2 + startY;
			UIView2D.ui._w = ew;

			// Editable layer name
			let h: zui_handle_t = zui_handle("uiview2d_1");

			///if (is_paint || is_sculpt)
			let text: string = UIView2D.type == view_2d_type_t.NODE ? Context.raw.node_preview_name : h.text;
			///else
			let text: string = h.text;
			///end

			UIView2D.ui._w = Math.floor(Math.min(g2_font_width(UIView2D.ui.font, UIView2D.ui.font_size, text) + 15 * zui_SCALE(UIView2D.ui), 100 * zui_SCALE(UIView2D.ui)));

			if (UIView2D.type == view_2d_type_t.ASSET) {
				let asset: asset_t = Context.raw.texture;
				if (asset != null) {
					let assetNames: string[] = Project.asset_names;
					let i: i32 = assetNames.indexOf(asset.name);
					h.text = asset.name;
					asset.name = zui_text_input(h, "");
					assetNames[i] = asset.name;
				}
			}
			else if (UIView2D.type == view_2d_type_t.NODE) {
				///if (is_paint || is_sculpt)

				zui_text(Context.raw.node_preview_name);

				///else

				let nodes: zui_nodes_t = UINodes.get_nodes();
				if (nodes.nodesSelectedId.length > 0) {
					zui_text(zui_get_node(UINodes.get_canvas(true).nodes, nodes.nodesSelectedId[0]).name);
				}

				///end
			}
			///if (is_paint || is_sculpt)
			else if (UIView2D.type == view_2d_type_t.LAYER) {
				h.text = l.name;
				l.name = zui_text_input(h, "");
				UIView2D.text_input_hover = UIView2D.ui.is_hovered;
			}
			else if (UIView2D.type == view_2d_type_t.FONT) {
				h.text = Context.raw.font.name;
				Context.raw.font.name = zui_text_input(h, "");
			}
			///end

			if (h.changed) UIBase.hwnds[0].redraws = 2;
			UIView2D.ui._x += UIView2D.ui._w + 3;
			UIView2D.ui._y = 2 + startY;
			UIView2D.ui._w = ew;

			///if (is_paint || is_sculpt)
			if (UIView2D.type == view_2d_type_t.LAYER) {
				UIView2D.layer_mode = zui_combo(zui_handle("uiview2d_2", { position: UIView2D.layer_mode }), [
					tr("Visible"),
					tr("Selected"),
				], tr("Layers"));
				UIView2D.ui._x += ew + 3;
				UIView2D.ui._y = 2 + startY;

				if (!SlotLayer.is_mask(Context.raw.layer)) {
					UIView2D.tex_type = zui_combo(zui_handle("uiview2d_3", { position: UIView2D.tex_type }), [
						tr("Base Color"),
						tr("Normal Map"),
						tr("Occlusion"),
						tr("Roughness"),
						tr("Metallic"),
						tr("Opacity"),
						tr("Height"),
					], tr("Texture"));
					UIView2D.ui._x += ew + 3;
					UIView2D.ui._y = 2 + startY;
				}

				UIView2D.ui._w = Math.floor(ew * 0.7 + 3);
				UIView2D.uvmap_show = zui_check(zui_handle("uiview2d_4", { selected: UIView2D.uvmap_show }), tr("UV Map"));
				UIView2D.ui._x += ew * 0.7 + 3;
				UIView2D.ui._y = 2 + startY;
			}
			///end

			UIView2D.tiled_show = zui_check(zui_handle("uiview2d_5", { selected: UIView2D.tiled_show }), tr("Tiled"));
			UIView2D.ui._x += ew * 0.7 + 3;
			UIView2D.ui._y = 2 + startY;

			if (UIView2D.type == view_2d_type_t.ASSET && tex != null) { // Texture resolution
				zui_text(tex.width + "x" + tex.height);
			}

			// Picked position
			///if (is_paint || is_sculpt)
			if (Context.raw.tool == workspace_tool_t.PICKER && (UIView2D.type == view_2d_type_t.LAYER || UIView2D.type == view_2d_type_t.ASSET)) {
				let cursorImg: image_t = Res.get("cursor.k");
				let hsize: f32 = 16 * zui_SCALE(UIView2D.ui);
				let size: f32 = hsize * 2;
				g2_draw_scaled_image(cursorImg, tx + tw * Context.raw.uvx_picked - hsize, ty + th * Context.raw.uvy_picked - hsize, size, size);
			}
			///end
		}
		zui_end();
		g2_begin(null);
	}

	static update = () => {
		let headerh: f32 = zui_ELEMENT_H(UIView2D.ui) * 1.4;

		///if (is_paint || is_sculpt)
		Context.raw.paint2d = false;
		///end

		if (!Base.ui_enabled ||
			!UIView2D.show ||
			mouse_x < UIView2D.wx ||
			mouse_x > UIView2D.wx + UIView2D.ww ||
			mouse_y < UIView2D.wy + headerh ||
			mouse_y > UIView2D.wy + UIView2D.wh) {
			if (UIView2D.controls_down) {
				UINodes.get_canvas_control(UIView2D.ui, UIView2D);
			}
			return;
		}

		let control: zui_canvas_control_t = UINodes.get_canvas_control(UIView2D.ui, UIView2D);
		UIView2D.pan_x += control.pan_x;
		UIView2D.pan_y += control.pan_y;
		if (control.zoom != 0) {
			let _panX: f32 = UIView2D.pan_x / UIView2D.pan_scale;
			let _panY: f32 = UIView2D.pan_y / UIView2D.pan_scale;
			UIView2D.pan_scale += control.zoom;
			if (UIView2D.pan_scale < 0.1) UIView2D.pan_scale = 0.1;
			if (UIView2D.pan_scale > 6.0) UIView2D.pan_scale = 6.0;
			UIView2D.pan_x = _panX * UIView2D.pan_scale;
			UIView2D.pan_y = _panY * UIView2D.pan_scale;

			if (zui_touch_scroll()) {
				// Zoom to finger location
				UIView2D.pan_x -= (UIView2D.ui.input_x - UIView2D.ui._window_x - UIView2D.ui._window_w / 2) * control.zoom;
				UIView2D.pan_y -= (UIView2D.ui.input_y - UIView2D.ui._window_y - UIView2D.ui._window_h / 2) * control.zoom;
			}
		}

		///if (is_paint || is_sculpt)
		let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
		let decalMask: bool = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
		let setCloneSource: bool = Context.raw.tool == workspace_tool_t.CLONE && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);

		if (UIView2D.type == view_2d_type_t.LAYER &&
			!UIView2D.text_input_hover &&
			(Operator.shortcut(Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 decalMask ||
			 setCloneSource ||
			 Config.raw.brush_live)) {
			Context.raw.paint2d = true;
		}
		///end

		if (UIView2D.ui.is_typing) return;

		if (keyboard_started("left")) UIView2D.pan_x -= 5;
		else if (keyboard_started("right")) UIView2D.pan_x += 5;
		if (keyboard_started("up")) UIView2D.pan_y -= 5;
		else if (keyboard_started("down")) UIView2D.pan_y += 5;

		// Limit panning to keep texture in viewport
		let border: i32 = 32;
		let tw: f32 = UIView2D.ww * 0.95 * UIView2D.pan_scale;
		let tx: f32 = UIView2D.ww / 2 - tw / 2 + UIView2D.pan_x;
		let hh: f32 = app_h();
		let ty: f32 = hh / 2 - tw / 2 + UIView2D.pan_y;

		if      (tx + border >  UIView2D.ww) UIView2D.pan_x =  UIView2D.ww / 2 + tw / 2 - border;
		else if (tx - border < -tw) UIView2D.pan_x = -tw / 2 - UIView2D.ww / 2 + border;
		if      (ty + border >  hh) UIView2D.pan_y =  hh / 2 + tw / 2 - border;
		else if (ty - border < -tw) UIView2D.pan_y = -tw / 2 - hh / 2 + border;

		if (Operator.shortcut(Config.keymap.view_reset)) {
			UIView2D.pan_x = 0.0;
			UIView2D.pan_y = 0.0;
			UIView2D.pan_scale = 1.0;
		}
	}
}
