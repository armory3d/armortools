
let ui_toolbar_default_w: i32       = 36;
let ui_toolbar_handle: ui_handle_t  = ui_handle_create();
let ui_toolbar_last_tool: i32       = 0;
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
	_tr("Material"),
	_tr("Gizmo"),
];
let ui_toolbar_tooltip_extras: string[] = [
	_tr("Hold {action_paint} to paint\nHold {brush_ruler} and press {action_paint} to paint a straight line (ruler mode)"),
	_tr("Hold {action_paint} to erase\nHold {brush_ruler} and press {action_paint} to erase a straight line (ruler mode)"),
	"",
	_tr("Hold {decal_mask} to paint on a decal mask"),
	_tr("Hold {decal_mask} to use the text as a mask"),
	_tr("Hold {set_clone_source} to set source"),
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
];

let _ui_toolbar_i: i32;

function ui_toolbar_init() {}

function ui_toolbar_draw_tool(tool: i32, img: gpu_texture_t, icon_accent: i32) {
	ui._x += 2;
	if (context_raw.tool == tool) {
		ui_toolbar_draw_highlight();
	}
	let tile_y: i32  = math_floor(tool / 12);
	let tile_x: i32  = tile_y % 2 == 0 ? tool % 12 : (11 - (tool % 12));
	let tile_i: i32  = tile_y * 12 + tile_x;
	let rect: rect_t = resource_tile50(img, tile_i);
	let _y: i32      = ui._y;

	let visible: bool = true;
	if (context_is_floating_toolbar()) {
		let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
		let statusy: i32 = iron_window_height() - statush;
		visible          = ui._y + ui._w * 2 < statusy;
	}

	let image_state: ui_state_t = ui_sub_image(img, icon_accent, -1.0, rect.x, rect.y, rect.w, rect.h);
	if (image_state == ui_state_t.STARTED && visible) {
		_ui_toolbar_i = tool;
		sys_notify_on_next_frame(function() {
			context_select_tool(_ui_toolbar_i);
		});
	}
	else if (image_state == ui_state_t.RELEASED && context_is_floating_toolbar() && visible) {
		if (ui_toolbar_last_tool == tool) {
			ui_toolbar_tool_properties_menu();
		}
		ui_toolbar_last_tool = tool;
	}

	if (tool == tool_type_t.COLORID && context_raw.colorid_picked) {
		let rt: render_target_t = map_get(render_path_render_targets, "texpaint_colorid");
		draw_scaled_sub_image(rt._image, 0, 0, 1, 1, 0, _y + 1.5 * UI_SCALE(), 5 * UI_SCALE(), 34 * UI_SCALE());
	}

	if (ui.is_hovered) {
		let tooltip: string = tr(ui_toolbar_tool_names[tool]);
		let key: string     = map_get(config_keymap, "tool_" + to_lower_case(ui_toolbar_tool_names[tool]));
		if (key != "") {
			tooltip += " (" + key + ")";
		}
		let extra: string = ui_toolbar_tooltip_extras[tool];
		if (extra != "") {
			tooltip += " - " + tr(extra, config_keymap);
		}
		ui_tooltip(tooltip);
	}
	ui._x -= 2;
	ui._y += 2;
}

function ui_toolbar_w(screen_size_request: bool = false): i32 {
	if (screen_size_request && context_is_floating_toolbar()) {
		return 0;
	}
	if (!base_view3d_show && !ui_view2d_show) {
		return 0;
	}

	let w: i32 = ui_toolbar_default_w;
	if (config_raw.touch_ui) {
		w = ui_toolbar_default_w + 6;
	}
	w = math_floor(w * config_raw.window_scale);
	return w;
}

function ui_toolbar_x(): i32 {
	return 5 * UI_SCALE();
}

function ui_toolbar_draw_show_3d_view() {
	if (context_is_floating_toolbar()) {
		let toolbar_w: i32 = ui_toolbar_default_w * UI_SCALE() + 14 * UI_SCALE();
		let _WINDOW_BG_COL: i32 = ui.ops.theme.WINDOW_BG_COL;
		// ui.ops.theme.WINDOW_BG_COL = ui.ops.theme.SEPARATOR_COL;
		let y: i32 = ui_header_h + 8 * UI_SCALE();

		if ((ui_view2d_show || ui_nodes_show) && !config_raw.touch_ui) {
			y += toolbar_w;
		}

		if (ui_window(ui_toolbar_handle, ui_toolbar_x(), y, toolbar_w, toolbar_w)) {
			let _ELEMENT_H: i32     = ui.ops.theme.ELEMENT_H;
			let _BUTTON_H: i32      = ui.ops.theme.BUTTON_H;
			let _BUTTON_COL: i32    = ui.ops.theme.BUTTON_COL;
			let _fontOffsetY: i32   = ui.font_offset_y;
			ui.ops.theme.ELEMENT_H  = math_floor(ui.ops.theme.ELEMENT_H * 1.5);
			ui.ops.theme.BUTTON_H   = ui.ops.theme.ELEMENT_H;
			ui.ops.theme.BUTTON_COL = ui.ops.theme.WINDOW_BG_COL;
			let font_height: i32    = draw_font_height(ui.ops.font, ui.font_size);
			ui.font_offset_y        = (UI_ELEMENT_H() - font_height) / 2;
			let _w: i32             = ui._w;
			ui._w                   = toolbar_w;
			if (ui_icon_button("", icon_t.CUBE)) {
				ui_base_show_3d_view();
			}
			ui._w                   = _w;
			ui.ops.theme.ELEMENT_H  = _ELEMENT_H;
			ui.ops.theme.BUTTON_H   = _BUTTON_H;
			ui.ops.theme.BUTTON_COL = _BUTTON_COL;
			ui.font_offset_y        = _fontOffsetY;
		}
		ui.ops.theme.WINDOW_BG_COL = _WINDOW_BG_COL;
	}
}

function ui_toolbar_render_ui() {
	let x: i32              = 0;
	let y: i32              = ui_header_h;
	let h: i32              = iron_window_height() - ui_header_h - config_raw.layout[layout_size_t.STATUS_H];
	let _WINDOW_BG_COL: i32 = ui.ops.theme.WINDOW_BG_COL;

	if (!base_view3d_show && !ui_view2d_show) {
		ui_toolbar_draw_show_3d_view();
		return;
	}

	if (!base_view3d_show && ui_view2d_show && ui_view2d_type != view_2d_type_t.LAYER) {
		ui_toolbar_draw_show_3d_view();
		return;
	}

	if (context_is_floating_toolbar()) {
		x += ui_toolbar_x();
		y += ui_toolbar_x() + 3 * UI_SCALE();
		h                          = (ui_toolbar_tool_names.length + 1) * (ui_toolbar_w() + 2);
		ui.ops.theme.WINDOW_BG_COL = ui.ops.theme.SEPARATOR_COL;

		if (!base_view3d_show && ui_view2d_show && !config_raw.touch_ui) {
			y += ui_toolbar_w();
		}
	}

	if (ui_window(ui_toolbar_handle, x, y, ui_toolbar_w(), h)) {
		ui._y -= 4 * UI_SCALE();
		ui.image_scroll_align  = false;
		let img: gpu_texture_t = resource_get("icons.k");
		let col: u32           = ui.ops.theme.WINDOW_BG_COL;
		let light: bool        = col > 0xff666666;
		let icon_accent: i32   = light ? 0xff666666 : -1;

		// Properties icon
		if (!context_is_floating_toolbar()) {
			let rect: rect_t = resource_tile50(img, icon_t.PROPERTIES);
			if (ui_sub_image(img, light ? 0xff666666 : ui.ops.theme.BUTTON_COL, -1.0, rect.x, rect.y, rect.w, rect.h) == ui_state_t.RELEASED) {
				config_raw.layout[layout_size_t.HEADER] = 0;
			}
		}
		// Draw ">" button if header is hidden
		else {
			let _ELEMENT_H: i32     = ui.ops.theme.ELEMENT_H;
			let _BUTTON_H: i32      = ui.ops.theme.BUTTON_H;
			let _BUTTON_COL: i32    = ui.ops.theme.BUTTON_COL;
			let _fontOffsetY: i32   = ui.font_offset_y;
			ui.ops.theme.ELEMENT_H  = math_floor(ui.ops.theme.ELEMENT_H * 1.5);
			ui.ops.theme.BUTTON_H   = ui.ops.theme.ELEMENT_H;
			ui.ops.theme.BUTTON_COL = ui.ops.theme.WINDOW_BG_COL;
			let font_height: i32    = draw_font_height(ui.ops.font, ui.font_size);
			ui.font_offset_y        = (UI_ELEMENT_H() - font_height) / 2;
			let _w: i32             = ui._w;
			ui._w                   = ui_toolbar_w();

			if (ui_button(">")) {
				ui_toolbar_tool_properties_menu();
			}

			ui._w                   = _w;
			ui.ops.theme.ELEMENT_H  = _ELEMENT_H;
			ui.ops.theme.BUTTON_H   = _BUTTON_H;
			ui.ops.theme.BUTTON_COL = _BUTTON_COL;
			ui.font_offset_y        = _fontOffsetY;
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Toggle header"));
		}
		ui._y -= 4 * UI_SCALE();

		for (let i: i32 = 0; i < ui_toolbar_tool_names.length; ++i) {
			ui_toolbar_draw_tool(i, img, icon_accent);
		}

		ui.image_scroll_align = true;
	}

	if (context_is_floating_toolbar()) {
		ui.ops.theme.WINDOW_BG_COL = _WINDOW_BG_COL;
	}

	if (config_raw.touch_ui) {
		// Hide scrollbar
		let _SCROLL_W: i32    = ui.ops.theme.SCROLL_W;
		ui.ops.theme.SCROLL_W = 0;
		ui_end_window();
		ui.ops.theme.SCROLL_W = _SCROLL_W;
	}
}

function ui_toolbar_tool_properties_menu() {
	let y: i32 = ui._y - 2 * UI_SCALE();
	if (!base_view3d_show) {
		y += ui_toolbar_w();
	}

	ui_menu_draw(function() {
		ui.changed = false;
		ui_header_draw_tool_properties();
		if (ui.changed || ui.is_typing) {
			ui_menu_keep_open = true;
		}
		if (base_view3d_show && ui_button(tr("Pin to Header"), ui_align_t.LEFT)) {
			config_raw.layout[layout_size_t.HEADER] = 1;
		}

		if (ui_button(base_view3d_show ? tr("Hide 3D View") : tr("Show 3D View"), ui_align_t.LEFT)) {
			ui_base_show_3d_view();
		}

	}, ui._x + ui._w + 6 * UI_SCALE(), y);
}

function ui_toolbar_draw_highlight() {
	let size: i32 = ui_toolbar_w() - 4;
	draw_set_color(ui.ops.theme.HIGHLIGHT_COL);
	ui_draw_rect(true, ui._x + -1, ui._y + 2, size + 2, size + 2);
}
