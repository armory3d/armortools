
class UIView2D {

	///if (is_paint || is_sculpt)
	static pipe: pipeline_t;
	static channelLocation: kinc_const_loc_t;
	static textInputHover = false;
	static uvmapShow = false;
	static texType = PaintTex.TexBase;
	static layerMode = View2DLayerMode.View2DSelected;
	///end

	///if (is_paint || is_sculpt)
	static type = View2DType.View2DLayer;
	///else
	static type = View2DType.View2DAsset;
	///end

	static show = false;
	static wx: i32;
	static wy: i32;
	static ww: i32;
	static wh: i32;
	static ui: zui_t;
	static hwnd = zui_handle_create();
	static panX = 0.0;
	static panY = 0.0;
	static panScale = 1.0;
	static tiledShow = false;
	static controlsDown = false;

	constructor() {
		///if (is_paint || is_sculpt)
		UIView2D.pipe = g4_pipeline_create();
		UIView2D.pipe.vertex_shader = sys_get_shader("layer_view.vert");
		UIView2D.pipe.fragment_shader = sys_get_shader("layer_view.frag");
		let vs = g4_vertex_struct_create();
		g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.F32_2X);
		g4_vertex_struct_add(vs, "col", vertex_data_t.U8_4X_NORM);
		UIView2D.pipe.input_layout = [vs];
		UIView2D.pipe.blend_source = blend_factor_t.BLEND_ONE;
		UIView2D.pipe.blend_dest = blend_factor_t.BLEND_ZERO;
		UIView2D.pipe.color_write_masks_alpha[0] = false;
		g4_pipeline_compile(UIView2D.pipe);
		UIView2D.channelLocation = g4_pipeline_get_const_loc(UIView2D.pipe, "channel");
		///end

		let scale = Config.raw.window_scale;
		UIView2D.ui = zui_create({ theme: Base.theme, font: Base.font, color_wheel: Base.colorWheel, black_white_gradient: Base.colorWheelGradient, scaleFactor: scale });
		UIView2D.ui.scroll_enabled = false;
	}

	static render = () => {

		UIView2D.ww = Config.raw.layout[LayoutSize.LayoutNodesW];

		///if (is_paint || is_sculpt)
		UIView2D.wx = Math.floor(app_w()) + UIToolbar.toolbarw;
		///else
		UIView2D.wx = Math.floor(app_w());
		///end

		UIView2D.wy = 0;

		///if (is_paint || is_sculpt)
		if (!UIBase.show) {
			UIView2D.ww += Config.raw.layout[LayoutSize.LayoutSidebarW] + UIToolbar.toolbarw;
			UIView2D.wx -= UIToolbar.toolbarw;
		}
		///end

		if (!UIView2D.show) return;
		if (sys_width() == 0 || sys_height() == 0) return;

		if (Context.raw.pdirty >= 0) UIView2D.hwnd.redraws = 2; // Paint was active

		g2_end();

		// Cache grid
		if (UINodes.grid == null) UINodes.drawGrid();

		// Ensure UV map is drawn
		///if (is_paint || is_sculpt)
		if (UIView2D.uvmapShow) UtilUV.cacheUVMap();
		///end

		// Ensure font image is drawn
		///if (is_paint || is_sculpt)
		if (Context.raw.font.image == null) UtilRender.makeFontPreview();
		///end

		zui_begin(UIView2D.ui);

		let headerh = Config.raw.layout[LayoutSize.LayoutHeader] == 1 ? UIHeader.headerh * 2 : UIHeader.headerh;
		let apph = sys_height() - Config.raw.layout[LayoutSize.LayoutStatusH] + headerh;
		UIView2D.wh = sys_height() - Config.raw.layout[LayoutSize.LayoutStatusH];

		if (UINodes.show) {
			UIView2D.wh -= Config.raw.layout[LayoutSize.LayoutNodesH];
			if (Config.raw.touch_ui) UIView2D.wh += UIHeader.headerh;
		}

		if (zui_window(UIView2D.hwnd, UIView2D.wx, UIView2D.wy, UIView2D.ww, UIView2D.wh)) {

			zui_tab(zui_handle("uiview2d_0"), tr("2D View"));

			// Grid
			g2_set_color(0xffffffff);
			g2_draw_image(UINodes.grid, (UIView2D.panX * UIView2D.panScale) % 100 - 100, (UIView2D.panY * UIView2D.panScale) % 100 - 100);

			// Texture
			let tex: image_t = null;

			///if (is_paint || is_sculpt)
			let l = Context.raw.layer;
			let channel = 0;
			///end

			let tw = UIView2D.ww * 0.95 * UIView2D.panScale;
			let tx = UIView2D.ww / 2 - tw / 2 + UIView2D.panX;
			let ty = apph / 2 - tw / 2 + UIView2D.panY;

			if (UIView2D.type == View2DType.View2DAsset) {
				tex = Project.getImage(Context.raw.texture);
			}
			else if (UIView2D.type == View2DType.View2DNode) {
				///if (is_paint || is_sculpt)

				tex = Context.raw.nodePreview;

				///else

				let nodes = UINodes.getNodes();
				if (nodes.nodesSelectedId.length > 0) {
					let sel = zui_get_node(UINodes.getCanvas(true).nodes, nodes.nodesSelectedId[0]);
					let brushNode = ParserLogic.getLogicNode(sel);
					if (brushNode != null) {
						tex = brushNode.getCachedImage();
					}
				}

				///end
			}
			///if is_paint
			else if (UIView2D.type == View2DType.View2DLayer) {
				let layer = l;

				if (Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0) {
					layer = RenderPathPaint.liveLayer;
				}

				if (UIView2D.layerMode == View2DLayerMode.View2DVisible) {
					let current = _g2_current;
					if (current != null) g2_end();
					layer = Base.flatten();
					if (current != null) g2_begin(current, false);
				}
				else if (SlotLayer.isGroup(layer)) {
					let current = _g2_current;
					if (current != null) g2_end();
					layer = Base.flatten(false, SlotLayer.getChildren(layer));
					if (current != null) g2_begin(current, false);
				}

				tex =
					SlotLayer.isMask(Context.raw.layer) ? layer.texpaint :
					UIView2D.texType == PaintTex.TexBase     ? layer.texpaint :
					UIView2D.texType == PaintTex.TexOpacity  ? layer.texpaint :
					UIView2D.texType == PaintTex.TexNormal   ? layer.texpaint_nor :
														   layer.texpaint_pack;

				channel =
					SlotLayer.isMask(Context.raw.layer)  ? 1 :
					UIView2D.texType == PaintTex.TexOcclusion ? 1 :
					UIView2D.texType == PaintTex.TexRoughness ? 2 :
					UIView2D.texType == PaintTex.TexMetallic  ? 3 :
					UIView2D.texType == PaintTex.TexOpacity   ? 4 :
					UIView2D.texType == PaintTex.TexHeight    ? 4 :
					UIView2D.texType == PaintTex.TexNormal    ? 5 :
															0;
			}
			else if (UIView2D.type == View2DType.View2DFont) {
				tex = Context.raw.font.image;
			}
			///end

			let th = tw;
			if (tex != null) {
				th = tw * (tex.height / tex.width);
				ty = apph / 2 - th / 2 + UIView2D.panY;

				///if (is_paint || is_sculpt)
				if (UIView2D.type == View2DType.View2DLayer) {
					///if (!krom_opengl)
					g2_set_pipeline(UIView2D.pipe);
					///end
					if (!Context.raw.textureFilter) {
						g2_set_bilinear_filter(false);
					}
					///if krom_opengl
					krom_g4_set_pipeline(UIView2D.pipe.pipeline_);
					///end
					krom_g4_set_int(UIView2D.channelLocation, channel);
				}
				///end

				g2_draw_scaled_image(tex, tx, ty, tw, th);

				if (UIView2D.tiledShow) {
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
				if (UIView2D.type == View2DType.View2DLayer) {
					g2_set_pipeline(null);
					if (!Context.raw.textureFilter) {
						g2_set_bilinear_filter(true);
					}
				}

				// Texture and node preview color picking
				if ((Context.in2dView(View2DType.View2DAsset) || Context.in2dView(View2DType.View2DNode)) && Context.raw.tool == WorkspaceTool.ToolPicker && UIView2D.ui.input_down) {
					let x = UIView2D.ui.input_x - tx - UIView2D.wx;
					let y = UIView2D.ui.input_y - ty - UIView2D.wy;
					Base.notifyOnNextFrame(() => {
						let texpaint_picker = render_path_render_targets.get("texpaint_picker").image;
						g2_begin(texpaint_picker, false);
						g2_draw_scaled_image(tex, -x, -y, tw, th);
						g2_end();
						let a = new DataView(image_get_pixels(texpaint_picker));
						///if (krom_metal || krom_vulkan)
						let i0 = 2;
						let i1 = 1;
						let i2 = 0;
						///else
						let i0 = 0;
						let i1 = 1;
						let i2 = 2;
						///end

						Context.raw.pickedColor.base = color_set_rb(Context.raw.pickedColor.base, a.getUint8(i0));
						Context.raw.pickedColor.base = color_set_gb(Context.raw.pickedColor.base, a.getUint8(i1));
						Context.raw.pickedColor.base = color_set_bb(Context.raw.pickedColor.base, a.getUint8(i2));
						UIHeader.headerHandle.redraws = 2;
					});
				}
				///end
			}

			///if (is_paint || is_sculpt)
			// UV map
			if (UIView2D.type == View2DType.View2DLayer && UIView2D.uvmapShow) {
				g2_draw_scaled_image(UtilUV.uvmap, tx, ty, tw, th);
			}
			///end

			// Menu
			let ew = Math.floor(zui_ELEMENT_W(UIView2D.ui));
			g2_set_color(UIView2D.ui.t.SEPARATOR_COL);
			g2_fill_rect(0, zui_ELEMENT_H(UIView2D.ui), UIView2D.ww, zui_ELEMENT_H(UIView2D.ui) + zui_ELEMENT_OFFSET(UIView2D.ui) * 2);
			g2_set_color(0xffffffff);

			let startY = zui_ELEMENT_H(UIView2D.ui) + zui_ELEMENT_OFFSET(UIView2D.ui);
			UIView2D.ui._x = 2;
			UIView2D.ui._y = 2 + startY;
			UIView2D.ui._w = ew;

			// Editable layer name
			let h = zui_handle("uiview2d_1");

			///if (is_paint || is_sculpt)
			let text = UIView2D.type == View2DType.View2DNode ? Context.raw.nodePreviewName : h.text;
			///else
			let text = h.text;
			///end

			UIView2D.ui._w = Math.floor(Math.min(g2_font_width(UIView2D.ui.font, UIView2D.ui.font_size, text) + 15 * zui_SCALE(UIView2D.ui), 100 * zui_SCALE(UIView2D.ui)));

			if (UIView2D.type == View2DType.View2DAsset) {
				let asset = Context.raw.texture;
				if (asset != null) {
					let assetNames = Project.assetNames;
					let i = assetNames.indexOf(asset.name);
					h.text = asset.name;
					asset.name = zui_text_input(h, "");
					assetNames[i] = asset.name;
				}
			}
			else if (UIView2D.type == View2DType.View2DNode) {
				///if (is_paint || is_sculpt)

				zui_text(Context.raw.nodePreviewName);

				///else

				let nodes = UINodes.getNodes();
				if (nodes.nodesSelectedId.length > 0) {
					zui_text(zui_get_node(UINodes.getCanvas(true).nodes, nodes.nodesSelectedId[0]).name);
				}

				///end
			}
			///if (is_paint || is_sculpt)
			else if (UIView2D.type == View2DType.View2DLayer) {
				h.text = l.name;
				l.name = zui_text_input(h, "");
				UIView2D.textInputHover = UIView2D.ui.is_hovered;
			}
			else if (UIView2D.type == View2DType.View2DFont) {
				h.text = Context.raw.font.name;
				Context.raw.font.name = zui_text_input(h, "");
			}
			///end

			if (h.changed) UIBase.hwnds[0].redraws = 2;
			UIView2D.ui._x += UIView2D.ui._w + 3;
			UIView2D.ui._y = 2 + startY;
			UIView2D.ui._w = ew;

			///if (is_paint || is_sculpt)
			if (UIView2D.type == View2DType.View2DLayer) {
				UIView2D.layerMode = zui_combo(zui_handle("uiview2d_2", { position: UIView2D.layerMode }), [
					tr("Visible"),
					tr("Selected"),
				], tr("Layers"));
				UIView2D.ui._x += ew + 3;
				UIView2D.ui._y = 2 + startY;

				if (!SlotLayer.isMask(Context.raw.layer)) {
					UIView2D.texType = zui_combo(zui_handle("uiview2d_3", { position: UIView2D.texType }), [
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
				UIView2D.uvmapShow = zui_check(zui_handle("uiview2d_4", { selected: UIView2D.uvmapShow }), tr("UV Map"));
				UIView2D.ui._x += ew * 0.7 + 3;
				UIView2D.ui._y = 2 + startY;
			}
			///end

			UIView2D.tiledShow = zui_check(zui_handle("uiview2d_5", { selected: UIView2D.tiledShow }), tr("Tiled"));
			UIView2D.ui._x += ew * 0.7 + 3;
			UIView2D.ui._y = 2 + startY;

			if (UIView2D.type == View2DType.View2DAsset && tex != null) { // Texture resolution
				zui_text(tex.width + "x" + tex.height);
			}

			// Picked position
			///if (is_paint || is_sculpt)
			if (Context.raw.tool == WorkspaceTool.ToolPicker && (UIView2D.type == View2DType.View2DLayer || UIView2D.type == View2DType.View2DAsset)) {
				let cursorImg = Res.get("cursor.k");
				let hsize = 16 * zui_SCALE(UIView2D.ui);
				let size = hsize * 2;
				g2_draw_scaled_image(cursorImg, tx + tw * Context.raw.uvxPicked - hsize, ty + th * Context.raw.uvyPicked - hsize, size, size);
			}
			///end
		}
		zui_end();
		g2_begin(null, false);
	}

	static update = () => {
		let headerh = zui_ELEMENT_H(UIView2D.ui) * 1.4;

		///if (is_paint || is_sculpt)
		Context.raw.paint2d = false;
		///end

		if (!Base.uiEnabled ||
			!UIView2D.show ||
			mouse_x < UIView2D.wx ||
			mouse_x > UIView2D.wx + UIView2D.ww ||
			mouse_y < UIView2D.wy + headerh ||
			mouse_y > UIView2D.wy + UIView2D.wh) {
			if (UIView2D.controlsDown) {
				UINodes.getCanvasControl(UIView2D.ui, UIView2D);
			}
			return;
		}

		let control = UINodes.getCanvasControl(UIView2D.ui, UIView2D);
		UIView2D.panX += control.panX;
		UIView2D.panY += control.panY;
		if (control.zoom != 0) {
			let _panX = UIView2D.panX / UIView2D.panScale;
			let _panY = UIView2D.panY / UIView2D.panScale;
			UIView2D.panScale += control.zoom;
			if (UIView2D.panScale < 0.1) UIView2D.panScale = 0.1;
			if (UIView2D.panScale > 6.0) UIView2D.panScale = 6.0;
			UIView2D.panX = _panX * UIView2D.panScale;
			UIView2D.panY = _panY * UIView2D.panScale;

			if (zui_touch_scroll()) {
				// Zoom to finger location
				UIView2D.panX -= (UIView2D.ui.input_x - UIView2D.ui._window_x - UIView2D.ui._window_w / 2) * control.zoom;
				UIView2D.panY -= (UIView2D.ui.input_y - UIView2D.ui._window_y - UIView2D.ui._window_h / 2) * control.zoom;
			}
		}

		///if (is_paint || is_sculpt)
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
		let setCloneSource = Context.raw.tool == WorkspaceTool.ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);

		if (UIView2D.type == View2DType.View2DLayer &&
			!UIView2D.textInputHover &&
			(Operator.shortcut(Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 decalMask ||
			 setCloneSource ||
			 Config.raw.brush_live)) {
			Context.raw.paint2d = true;
		}
		///end

		if (UIView2D.ui.is_typing) return;

		if (keyboard_started("left")) UIView2D.panX -= 5;
		else if (keyboard_started("right")) UIView2D.panX += 5;
		if (keyboard_started("up")) UIView2D.panY -= 5;
		else if (keyboard_started("down")) UIView2D.panY += 5;

		// Limit panning to keep texture in viewport
		let border = 32;
		let tw = UIView2D.ww * 0.95 * UIView2D.panScale;
		let tx = UIView2D.ww / 2 - tw / 2 + UIView2D.panX;
		let hh = app_h();
		let ty = hh / 2 - tw / 2 + UIView2D.panY;

		if      (tx + border >  UIView2D.ww) UIView2D.panX =  UIView2D.ww / 2 + tw / 2 - border;
		else if (tx - border < -tw) UIView2D.panX = -tw / 2 - UIView2D.ww / 2 + border;
		if      (ty + border >  hh) UIView2D.panY =  hh / 2 + tw / 2 - border;
		else if (ty - border < -tw) UIView2D.panY = -tw / 2 - hh / 2 + border;

		if (Operator.shortcut(Config.keymap.view_reset)) {
			UIView2D.panX = 0.0;
			UIView2D.panY = 0.0;
			UIView2D.panScale = 1.0;
		}
	}
}
