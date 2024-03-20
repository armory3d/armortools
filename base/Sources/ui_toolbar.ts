
///if (is_paint || is_sculpt)

let ui_toolbar_default_w: i32 = 36;

let ui_toolbar_handle: zui_handle_t = zui_handle_create();
let ui_toolbar_w: i32 = ui_toolbar_default_w;
let ui_toolbar_last_tool: i32 = 0;

let ui_toolbar_tool_names: string[] = [
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

function ui_toolbar_init() {
}

function ui_toolbar_render_ui() {
	let ui: zui_t = ui_base_ui;

	if (config_raw.touch_ui) {
		ui_toolbar_w = ui_toolbar_default_w + 6;
	}
	else {
		ui_toolbar_w = ui_toolbar_default_w;
	}
	ui_toolbar_w = math_floor(ui_toolbar_w * zui_SCALE(ui));

	if (zui_window(ui_toolbar_handle, 0, ui_header_h, ui_toolbar_w, sys_height() - ui_header_h)) {
		ui._y -= 4 * zui_SCALE(ui);

		ui.image_scroll_align = false;
		let img: image_t = resource_get("icons.k");

		let col: i32 = ui.t.WINDOW_BG_COL;
		if (col < 0) {
			col += 4294967296;
		}
		let light: bool = col > (0xff666666 + 4294967296);
		let icon_accent: i32 = light ? 0xff666666 : -1;

		// Properties icon
		if (config_raw.layout[layout_size_t.HEADER] == 1) {
			let rect: rect_t = resource_tile50(img, 7, 1);
			if (zui_image(img, light ? 0xff666666 : ui.t.BUTTON_COL, -1.0, rect.x, rect.y, rect.w, rect.h) == zui_state_t.RELEASED) {
				config_raw.layout[layout_size_t.HEADER] = 0;
			}
		}
		// Draw ">>" button if header is hidden
		else {
			let _ELEMENT_H: i32 = ui.t.ELEMENT_H;
			let _BUTTON_H: i32 = ui.t.BUTTON_H;
			let _BUTTON_COL: i32 = ui.t.BUTTON_COL;
			let _fontOffsetY: i32 = ui.font_offset_y;
			ui.t.ELEMENT_H = math_floor(ui.t.ELEMENT_H * 1.5);
			ui.t.BUTTON_H = ui.t.ELEMENT_H;
			ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;
			let font_height: i32 = g2_font_height(ui.font, ui.font_size);
			ui.font_offset_y = (zui_ELEMENT_H(ui) - font_height) / 2;
			let _w: i32 = ui._w;
			ui._w = ui_toolbar_w;

			if (zui_button(">>")) {
				ui_toolbar_tool_properties_menu();
			}

			ui._w = _w;
			ui.t.ELEMENT_H = _ELEMENT_H;
			ui.t.BUTTON_H = _BUTTON_H;
			ui.t.BUTTON_COL = _BUTTON_COL;
			ui.font_offset_y = _fontOffsetY;
		}
		if (ui.is_hovered) {
			zui_tooltip(tr("Toggle header"));
		}
		ui._y -= 4 * zui_SCALE(ui);

		let keys: string[] = [
			"(" + config_keymap.tool_brush + ") - " + tr("Hold {action_paint} to paint\nHold {key} and press {action_paint} to paint a straight line (ruler mode)", new map_t([["key", config_keymap.brush_ruler], ["action_paint", config_keymap.action_paint]])),
			"(" + config_keymap.tool_eraser + ") - " + tr("Hold {action_paint} to erase\nHold {key} and press {action_paint} to erase a straight line (ruler mode)", new map_t([["key", config_keymap.brush_ruler], ["action_paint", config_keymap.action_paint]])),
			"(" + config_keymap.tool_fill + ")",
			"(" + config_keymap.tool_decal + ") - " + tr("Hold {key} to paint on a decal mask", new map_t([["key", config_keymap.decal_mask]])),
			"(" + config_keymap.tool_text + ") - " + tr("Hold {key} to use the text as a mask", new map_t([["key", config_keymap.decal_mask]])),
			"(" + config_keymap.tool_clone + ") - " + tr("Hold {key} to set source", new map_t([["key", config_keymap.set_clone_source]])),
			"(" + config_keymap.tool_blur + ")",
			"(" + config_keymap.tool_smudge + ")",
			"(" + config_keymap.tool_particle + ")",
			"(" + config_keymap.tool_colorid + ")",
			"(" + config_keymap.tool_picker + ")",
			"(" + config_keymap.tool_bake + ")",
			"(" + config_keymap.tool_gizmo + ")",
			"(" + config_keymap.tool_material + ")",
		];

		let draw_tool = function (i: i32) {
			ui._x += 2;
			if (context_raw.tool == i) {
				ui_toolbar_draw_highlight();
			}
			let tile_y: i32 = math_floor(i / 12);
			let tile_x: i32 = tile_y % 2 == 0 ? i % 12 : (11 - (i % 12));
			let rect: rect_t = resource_tile50(img, tile_x, tile_y);
			let _y: i32 = ui._y;

			let image_state: zui_state_t = zui_image(img, icon_accent, -1.0, rect.x, rect.y, rect.w, rect.h);
			if (image_state == zui_state_t.STARTED) {
				context_select_tool(i);
			}
			else if (image_state == zui_state_t.RELEASED && config_raw.layout[layout_size_t.HEADER] == 0) {
				if (ui_toolbar_last_tool == i) {
					ui_toolbar_tool_properties_menu();
				}
				ui_toolbar_last_tool = i;
			}

			///if is_paint
			if (i == workspace_tool_t.COLORID && context_raw.colorid_picked) {
				g2_draw_scaled_sub_image(map_get(render_path_render_targets, "texpaint_colorid")._image, 0, 0, 1, 1, 0, _y + 1.5 * zui_SCALE(ui), 5 * zui_SCALE(ui), 34 * zui_SCALE(ui));
			}
			///end

			if (ui.is_hovered) {
				zui_tooltip(tr(ui_toolbar_tool_names[i]) + " " + keys[i]);
			}
			ui._x -= 2;
			ui._y += 2;
		}

		draw_tool(workspace_tool_t.BRUSH);
		///if is_paint
		draw_tool(workspace_tool_t.ERASER);
		draw_tool(workspace_tool_t.FILL);
		draw_tool(workspace_tool_t.DECAL);
		draw_tool(workspace_tool_t.TEXT);
		draw_tool(workspace_tool_t.CLONE);
		draw_tool(workspace_tool_t.BLUR);
		draw_tool(workspace_tool_t.SMUDGE);
		draw_tool(workspace_tool_t.PARTICLE);
		draw_tool(workspace_tool_t.COLORID);
		draw_tool(workspace_tool_t.PICKER);
		draw_tool(workspace_tool_t.BAKE);
		draw_tool(workspace_tool_t.MATERIAL);
		///end

		///if is_forge
		draw_tool(workspace_tool_t.GIZMO);
		///end

		ui.image_scroll_align = true;
	}

	if (config_raw.touch_ui) {
		// Hide scrollbar
		let _SCROLL_W: i32 = ui.t.SCROLL_W;
		ui.t.SCROLL_W = 0;
		zui_end_window();
		ui.t.SCROLL_W = _SCROLL_W;
	}
}

function ui_toolbar_tool_properties_menu() {
	let ui: zui_t = ui_base_ui;
	let _x: i32 = ui._x;
	let _y: i32 = ui._y;
	let _w: i32 = ui._w;
	ui_menu_draw(function (ui: zui_t) {
		let start_y: i32 = ui._y;
		ui.changed = false;

		ui_header_draw_tool_properties(ui);

		if (ui.changed) {
			ui_menu_keep_open = true;
		}

		if (zui_button(tr("Pin to Header"), zui_align_t.LEFT)) {
			config_raw.layout[layout_size_t.HEADER] = 1;
		}

		let h: i32 = ui._y - start_y;
		ui_menu_elements = math_floor(h / zui_ELEMENT_H(ui));
		ui_menu_x = math_floor(_x + _w + 2);
		ui_menu_y = math_floor(_y - 6 * zui_SCALE(ui));
		ui_menu_fit_to_screen();

	}, 0);

	// First draw out of screen, then align the menu based on menu height
	ui_menu_x = -sys_width();
	ui_menu_y = -sys_height();
}

function ui_toolbar_draw_highlight() {
	let ui: zui_t = ui_base_ui;
	let size: i32 = ui_toolbar_w - 4;
	g2_set_color(ui.t.HIGHLIGHT_COL);
	zui_draw_rect(true, ui._x + -1,  ui._y + 2, size + 2, size + 2);
}

///end
