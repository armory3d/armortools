
let ui_statusbar_default_h: i32 = 33;
let ui_statusbar_last_tab: i32 = 0;

function ui_statusbar_init() {
}

function ui_statusbar_width(): i32 {
	return iron_window_width() - config_raw.layout[layout_size_t.SIDEBAR_W];
}

function ui_statusbar_render_ui() {
	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];

	if (ui_window(ui_base_hwnds[tab_area_t.STATUS], 0, iron_window_height() - statush, ui_statusbar_width(), statush)) {
		ui._y += 2;

		// Border
		draw_set_color(ui.ops.theme.SEPARATOR_COL);
		draw_filled_rect(0, 0, 1, ui._window_h);
		draw_filled_rect(ui._window_w - 1, 0, 1, ui._window_h);

		// Draw tabs
		let hwnd_draws: tab_draw_t[] = ui_base_hwnd_tabs[tab_area_t.STATUS];
		let htab: ui_handle_t = ui_base_htabs[tab_area_t.STATUS];
		for (let i: i32 = 0; i < hwnd_draws.length; ++i) {
			let draw: tab_draw_t = hwnd_draws[i];
			draw.f(htab);
		}

		if (ui_base_htabs[tab_area_t.STATUS].i < hwnd_draws.length) {
			ui_statusbar_last_tab = ui_base_htabs[tab_area_t.STATUS].i;
		}

		if (!config_raw.touch_ui) {
			let minimized: bool = config_raw.layout[layout_size_t.STATUS_H] <= (ui_statusbar_default_h * config_raw.window_scale);
			if (ui_tab(ui_base_htabs[tab_area_t.STATUS], minimized ? "<" : ">", false, -2)) {
				ui_base_htabs[tab_area_t.STATUS].i = ui_statusbar_last_tab;
				config_raw.layout_tabs[tab_area_t.STATUS] = ui_statusbar_last_tab;
			}
		}

		let minimized: bool = statush <= ui_statusbar_default_h * config_raw.window_scale;
		if (htab.changed && (htab.i == context_raw.last_status_position || minimized)) {
			ui_base_toggle_browser();
		}
		context_raw.last_status_position = htab.i;
	}
}

function ui_statusbar_draw_version_tab(htab: ui_handle_t) {
	// Version label
	if (!config_raw.touch_ui) {
		ui.enabled = false;
		ui_tab(ui_base_htabs[tab_area_t.STATUS], manifest_version);
		ui.enabled = true;
	}
}
