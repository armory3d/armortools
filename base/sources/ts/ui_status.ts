
let ui_status_default_status_h: i32 = 33;

function ui_status_init() {
}

function ui_status_width(): i32 {
	///if (is_paint || is_sculpt)
	return sys_width() - ui_toolbar_w - config_raw.layout[layout_size_t.SIDEBAR_W];
	///end
	///if is_lab
	return sys_width();
	///end
}

function ui_status_render_ui() {
	let ui: ui_t = ui_base_ui;

	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];

	if (ui_window(ui_base_hwnds[tab_area_t.STATUS], app_x(), sys_height() - statush, ui_status_width(), statush)) {
		ui._y += 2;

		// Border
		g2_set_color(ui.ops.theme.SEPARATOR_COL);
		g2_fill_rect(0, 0, 1, ui._window_h);
		g2_fill_rect(ui._window_w - 1, 0, 1, ui._window_h);

		// Draw tabs
		let hwnd_draws: tab_draw_t[] = ui_base_hwnd_tabs[tab_area_t.STATUS];
		let htab: ui_handle_t = ui_base_htabs[tab_area_t.STATUS];
		for (let i: i32 = 0; i < hwnd_draws.length; ++i) {
			let draw: tab_draw_t = hwnd_draws[i];
			draw.f(htab);
		}

		let minimized: bool = statush <= ui_status_default_status_h * config_raw.window_scale;
		if (htab.changed && (htab.position == context_raw.last_status_position || minimized)) {
			ui_base_toggle_browser();
		}
		context_raw.last_status_position = htab.position;
	}
}

function ui_status_draw_version_tab(htab: ui_handle_t) {
	// Version label
	if (!config_raw.touch_ui) {
		let ui: ui_t = ui_base_ui;
		ui.enabled = false;
		ui_tab(ui_base_htabs[tab_area_t.STATUS], manifest_version);
		ui.enabled = true;
	}
}
