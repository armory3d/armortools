
class UIStatus {

	static default_status_h: i32 = 33;

	constructor() {
	}

	static get width(): i32 {
		///if (is_paint || is_sculpt)
		return sys_width() - UIToolbar.toolbar_w - Config.raw.layout[layout_size_t.SIDEBAR_W];
		///end
		///if is_lab
		return sys_width();
		///end
	}

	static render_ui = () => {
		let ui: zui_t = UIBase.ui;

		let statush: i32 = Config.raw.layout[layout_size_t.STATUS_H];

		if (zui_window(UIBase.hwnds[tab_area_t.STATUS], app_x(), sys_height() - statush, UIStatus.width, statush)) {
			ui._y += 2;

			// Border
			g2_set_color(ui.t.SEPARATOR_COL);
			g2_fill_rect(0, 0, 1, ui._window_h);
			g2_fill_rect(ui._window_w - 1, 0, 1, ui._window_h);

			// Draw tabs
			for (let draw of UIBase.hwnd_tabs[tab_area_t.STATUS]) draw(UIBase.htabs[tab_area_t.STATUS]);

			let minimized: bool = statush <= UIStatus.default_status_h * Config.raw.window_scale;
			if (UIBase.htabs[tab_area_t.STATUS].changed && (UIBase.htabs[tab_area_t.STATUS].position == Context.raw.last_status_position || minimized)) {
				UIBase.toggle_browser();
			}
			Context.raw.last_status_position = UIBase.htabs[tab_area_t.STATUS].position;
		}
	}

	static draw_version_tab = (htab: zui_handle_t) => {
		// Version label
		if (!Config.raw.touch_ui) {
			let ui: zui_t = UIBase.ui;
			ui.enabled = false;
			zui_tab(UIBase.htabs[tab_area_t.STATUS], manifest_version);
			ui.enabled = true;
		}
	}
}
