
let ui_sidebar_default_w_mini: i32 = 56;
let ui_sidebar_default_w_full: i32 = 280;

///if (arm_android || arm_ios)
let ui_sidebar_default_w: i32 = ui_sidebar_default_w_mini;
///else
let ui_sidebar_default_w: i32 = ui_sidebar_default_w_full;
///end

let ui_sidebar_tabx: i32 = 0;
let ui_sidebar_hminimized: ui_handle_t = ui_handle_create();
let ui_sidebar_w_mini: i32 = ui_sidebar_default_w_mini;

function ui_sidebar_render_ui() {
	// Tabs
	let mini: bool = config_raw.layout[layout_size_t.SIDEBAR_W] <= ui_sidebar_w_mini;
	let expand_button_offset: i32 = config_raw.touch_ui ? math_floor(UI_ELEMENT_H() + UI_ELEMENT_OFFSET()) : 0;
	ui_sidebar_tabx = iron_window_width() - config_raw.layout[layout_size_t.SIDEBAR_W];

	let _SCROLL_W: i32 = ui.ops.theme.SCROLL_W;
	if (mini) {
		ui.ops.theme.SCROLL_W = ui.ops.theme.SCROLL_MINI_W;
	}

	if (ui_window(ui_base_hwnds[tab_area_t.SIDEBAR0], ui_sidebar_tabx, 0, config_raw.layout[layout_size_t.SIDEBAR_W], config_raw.layout[layout_size_t.SIDEBAR_H0])) {
		let tabs: tab_draw_t[] = ui_base_hwnd_tabs[tab_area_t.SIDEBAR0];
		for (let i: i32 = 0; i < (mini ? 1 : tabs.length); ++i) {
			tabs[i].f(ui_base_htabs[tab_area_t.SIDEBAR0]);
		}
	}
	if (ui_window(ui_base_hwnds[tab_area_t.SIDEBAR1], ui_sidebar_tabx, config_raw.layout[layout_size_t.SIDEBAR_H0], config_raw.layout[layout_size_t.SIDEBAR_W], config_raw.layout[layout_size_t.SIDEBAR_H1] - expand_button_offset)) {
		let tabs: tab_draw_t[] = ui_base_hwnd_tabs[tab_area_t.SIDEBAR1];
		for (let i: i32 = 0; i < (mini ? 1 : tabs.length); ++i) {
			tabs[i].f(ui_base_htabs[tab_area_t.SIDEBAR1]);
		}
	}

	ui_end_window();
	ui.ops.theme.SCROLL_W = _SCROLL_W;

	// Collapse / expand button for mini sidebar
	if (config_raw.touch_ui) {
		let width: i32 = config_raw.layout[layout_size_t.SIDEBAR_W];
		let height: i32 = math_floor(UI_ELEMENT_H() + UI_ELEMENT_OFFSET());
		if (ui_window(ui_handle(__ID__), iron_window_width() - width, iron_window_height() - height, width, height + 1)) {
			ui._w = width;
			let _BUTTON_H: i32 = ui.ops.theme.BUTTON_H;
			let _BUTTON_COL: i32 = ui.ops.theme.BUTTON_COL;
			ui.ops.theme.BUTTON_H = ui.ops.theme.ELEMENT_H;
			ui.ops.theme.BUTTON_COL = ui.ops.theme.WINDOW_BG_COL;
			if (ui_button(mini ? "<<" : ">>")) {
				config_raw.layout[layout_size_t.SIDEBAR_W] = mini ? ui_sidebar_default_w_full : ui_sidebar_default_w_mini;
				config_raw.layout[layout_size_t.SIDEBAR_W] = math_floor(config_raw.layout[layout_size_t.SIDEBAR_W] * UI_SCALE());
			}
			ui.ops.theme.BUTTON_H = _BUTTON_H;
			ui.ops.theme.BUTTON_COL = _BUTTON_COL;
		}
	}

	// Expand button
	if (config_raw.layout[layout_size_t.SIDEBAR_W] == 0) {
		let width: i32 = math_floor(draw_string_width(ui.ops.font, ui.font_size, "<<") + 25 * UI_SCALE());
		if (ui_window(ui_sidebar_hminimized, iron_window_width() - width, 0, width, math_floor(UI_ELEMENT_H() + UI_ELEMENT_OFFSET() + 1))) {
			ui._w = width;
			let _BUTTON_H: i32 = ui.ops.theme.BUTTON_H;
			let _BUTTON_COL: i32 = ui.ops.theme.BUTTON_COL;
			ui.ops.theme.BUTTON_H = ui.ops.theme.ELEMENT_H;
			ui.ops.theme.BUTTON_COL = ui.ops.theme.SEPARATOR_COL;

			if (ui_button("<<")) {
				config_raw.layout[layout_size_t.SIDEBAR_W] = context_raw.maximized_sidebar_width != 0 ? context_raw.maximized_sidebar_width : math_floor(ui_sidebar_default_w * config_raw.window_scale);
			}
			ui.ops.theme.BUTTON_H = _BUTTON_H;
			ui.ops.theme.BUTTON_COL = _BUTTON_COL;
		}
	}
	else if (ui_base_htabs[tab_area_t.SIDEBAR0].changed && ui_base_htabs[tab_area_t.SIDEBAR0].position == context_raw.last_htab0_pos) {
		if (sys_time() - context_raw.select_time < 0.25) {
			context_raw.maximized_sidebar_width = config_raw.layout[layout_size_t.SIDEBAR_W];
			config_raw.layout[layout_size_t.SIDEBAR_W] = 0;
		}
		context_raw.select_time = sys_time();
	}
	context_raw.last_htab0_pos = ui_base_htabs[tab_area_t.SIDEBAR0].position;
}
