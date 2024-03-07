
///if (is_paint || is_sculpt)

class UIToolbar {

	static default_toolbar_w: i32 = 36;

	static toolbar_handle: zui_handle_t = zui_handle_create();
	static toolbar_w: i32 = UIToolbar.default_toolbar_w;
	static last_tool: i32 = 0;

	static tool_names: string[] = [
		_tr("Brush"),
		_tr("Eraser"),
		_tr("Fill"),
		_tr("Decal"),
		_tr("Text"),
		_tr("Clone"),
		_tr("Blur"),
		_tr("Smudge"),
		_tr("Particle"),
		_tr("ColorID"),
		_tr("Picker"),
		_tr("Bake"),
		_tr("Gizmo"),
		_tr("Material"),
	];

	constructor() {
	}

	static render_ui = () => {
		let ui: zui_t = UIBase.ui;

		if (Config.raw.touch_ui) {
			UIToolbar.toolbar_w = UIToolbar.default_toolbar_w + 6;
		}
		else {
			UIToolbar.toolbar_w = UIToolbar.default_toolbar_w;
		}
		UIToolbar.toolbar_w = Math.floor(UIToolbar.toolbar_w * zui_SCALE(ui));

		if (zui_window(UIToolbar.toolbar_handle, 0, UIHeader.headerh, UIToolbar.toolbar_w, sys_height() - UIHeader.headerh)) {
			ui._y -= 4 * zui_SCALE(ui);

			ui.image_scroll_align = false;
			let img: image_t = Res.get("icons.k");

			let col: i32 = ui.t.WINDOW_BG_COL;
			if (col < 0) col += 4294967296;
			let light: bool = col > (0xff666666 + 4294967296);
			let iconAccent: i32 = light ? 0xff666666 : -1;

			// Properties icon
			if (Config.raw.layout[layout_size_t.HEADER] == 1) {
				let rect: rect_t = Res.tile50(img, 7, 1);
				if (zui_image(img, light ? 0xff666666 : ui.t.BUTTON_COL, -1.0, rect.x, rect.y, rect.w, rect.h) == zui_state_t.RELEASED) {
					Config.raw.layout[layout_size_t.HEADER] = 0;
				}
			}
			// Draw ">>" button if header is hidden
			else {
				let _ELEMENT_H: i32 = ui.t.ELEMENT_H;
				let _BUTTON_H: i32 = ui.t.BUTTON_H;
				let _BUTTON_COL: i32 = ui.t.BUTTON_COL;
				let _fontOffsetY: i32 = ui.font_offset_y;
				ui.t.ELEMENT_H = Math.floor(ui.t.ELEMENT_H * 1.5);
				ui.t.BUTTON_H = ui.t.ELEMENT_H;
				ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;
				let fontHeight: i32 = g2_font_height(ui.font, ui.font_size);
				ui.font_offset_y = (zui_ELEMENT_H(ui) - fontHeight) / 2;
				let _w: i32 = ui._w;
				ui._w = UIToolbar.toolbar_w;

				if (zui_button(">>")) {
					UIToolbar.tool_properties_menu();
				}

				ui._w = _w;
				ui.t.ELEMENT_H = _ELEMENT_H;
				ui.t.BUTTON_H = _BUTTON_H;
				ui.t.BUTTON_COL = _BUTTON_COL;
				ui.font_offset_y = _fontOffsetY;
			}
			if (ui.is_hovered) zui_tooltip(tr("Toggle header"));
			ui._y -= 4 * zui_SCALE(ui);

			let keys: string[] = [
				"(" + Config.keymap.tool_brush + ") - " + tr("Hold {action_paint} to paint\nHold {key} and press {action_paint} to paint a straight line (ruler mode)", new Map([["key", Config.keymap.brush_ruler], ["action_paint", Config.keymap.action_paint]])),
				"(" + Config.keymap.tool_eraser + ") - " + tr("Hold {action_paint} to erase\nHold {key} and press {action_paint} to erase a straight line (ruler mode)", new Map([["key", Config.keymap.brush_ruler], ["action_paint", Config.keymap.action_paint]])),
				"(" + Config.keymap.tool_fill + ")",
				"(" + Config.keymap.tool_decal + ") - " + tr("Hold {key} to paint on a decal mask", new Map([["key", Config.keymap.decal_mask]])),
				"(" + Config.keymap.tool_text + ") - " + tr("Hold {key} to use the text as a mask", new Map([["key", Config.keymap.decal_mask]])),
				"(" + Config.keymap.tool_clone + ") - " + tr("Hold {key} to set source", new Map([["key", Config.keymap.set_clone_source]])),
				"(" + Config.keymap.tool_blur + ")",
				"(" + Config.keymap.tool_smudge + ")",
				"(" + Config.keymap.tool_particle + ")",
				"(" + Config.keymap.tool_colorid + ")",
				"(" + Config.keymap.tool_picker + ")",
				"(" + Config.keymap.tool_bake + ")",
				"(" + Config.keymap.tool_gizmo + ")",
				"(" + Config.keymap.tool_material + ")",
			];

			let drawTool = (i: i32) => {
				ui._x += 2;
				if (Context.raw.tool == i) UIToolbar.draw_highlight();
				let tileY: i32 = Math.floor(i / 12);
				let tileX: i32 = tileY % 2 == 0 ? i % 12 : (11 - (i % 12));
				let rect: rect_t = Res.tile50(img, tileX, tileY);
				let _y: i32 = ui._y;

				let imageState: zui_state_t = zui_image(img, iconAccent, -1.0, rect.x, rect.y, rect.w, rect.h);
				if (imageState == zui_state_t.STARTED) {
					Context.select_tool(i);
				}
				else if (imageState == zui_state_t.RELEASED && Config.raw.layout[layout_size_t.HEADER] == 0) {
					if (UIToolbar.last_tool == i) {
						UIToolbar.tool_properties_menu();
					}
					UIToolbar.last_tool = i;
				}

				///if is_paint
				if (i == workspace_tool_t.COLORID && Context.raw.colorid_picked) {
					g2_draw_scaled_sub_image(render_path_render_targets.get("texpaint_colorid")._image, 0, 0, 1, 1, 0, _y + 1.5 * zui_SCALE(ui), 5 * zui_SCALE(ui), 34 * zui_SCALE(ui));
				}
				///end

				if (ui.is_hovered) zui_tooltip(tr(UIToolbar.tool_names[i]) + " " + keys[i]);
				ui._x -= 2;
				ui._y += 2;
			}

			drawTool(workspace_tool_t.BRUSH);
			///if is_paint
			drawTool(workspace_tool_t.ERASER);
			drawTool(workspace_tool_t.FILL);
			drawTool(workspace_tool_t.DECAL);
			drawTool(workspace_tool_t.TEXT);
			drawTool(workspace_tool_t.CLONE);
			drawTool(workspace_tool_t.BLUR);
			drawTool(workspace_tool_t.SMUDGE);
			drawTool(workspace_tool_t.PARTICLE);
			drawTool(workspace_tool_t.COLORID);
			drawTool(workspace_tool_t.PICKER);
			drawTool(workspace_tool_t.BAKE);
			drawTool(workspace_tool_t.MATERIAL);
			///end

			///if is_forge
			drawTool(workspace_tool_t.GIZMO);
			///end

			ui.image_scroll_align = true;
		}

		if (Config.raw.touch_ui) {
			// Hide scrollbar
			let _SCROLL_W: i32 = ui.t.SCROLL_W;
			ui.t.SCROLL_W = 0;
			zui_end_window();
			ui.t.SCROLL_W = _SCROLL_W;
		}
	}

	static tool_properties_menu = () => {
		let ui: zui_t = UIBase.ui;
		let _x: i32 = ui._x;
		let _y: i32 = ui._y;
		let _w: i32 = ui._w;
		UIMenu.draw((ui: zui_t) => {
			let startY: i32 = ui._y;
			ui.changed = false;

			UIHeader.draw_tool_properties(ui);

			if (ui.changed) {
				UIMenu.keep_open = true;
			}

			if (zui_button(tr("Pin to Header"), zui_align_t.LEFT)) {
				Config.raw.layout[layout_size_t.HEADER] = 1;
			}

			let h: i32 = ui._y - startY;
			UIMenu.menu_elements = Math.floor(h / zui_ELEMENT_H(ui));
			UIMenu.menu_x = Math.floor(_x + _w + 2);
			UIMenu.menu_y = Math.floor(_y - 6 * zui_SCALE(ui));
			UIMenu.fit_to_screen();

		}, 0);

		// First draw out of screen, then align the menu based on menu height
		UIMenu.menu_x = -sys_width();
		UIMenu.menu_y = -sys_height();
	}

	static draw_highlight = () => {
		let ui: zui_t = UIBase.ui;
		let size: i32 = UIToolbar.toolbar_w - 4;
		g2_set_color(ui.t.HIGHLIGHT_COL);
		zui_draw_rect(true, ui._x + -1,  ui._y + 2, size + 2, size + 2);
	}
}

///end
