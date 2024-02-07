
class UIStatus {

	static defaultStatusH = 33;

	constructor() {
	}

	static get width(): i32 {
		///if (is_paint || is_sculpt)
		return sys_width() - UIToolbar.toolbarw - Config.raw.layout[LayoutSize.LayoutSidebarW];
		///end
		///if is_lab
		return sys_width();
		///end
	}

	static renderUI = () => {
		let ui = UIBase.ui;

		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];

		if (zui_window(UIBase.hwnds[TabArea.TabStatus], app_x(), sys_height() - statush, UIStatus.width, statush)) {
			ui._y += 2;

			// Border
			g2_set_color(ui.t.SEPARATOR_COL);
			g2_fill_rect(0, 0, 1, ui._window_h);
			g2_fill_rect(ui._window_w - 1, 0, 1, ui._window_h);

			// Draw tabs
			for (let draw of UIBase.hwndTabs[TabArea.TabStatus]) draw(UIBase.htabs[TabArea.TabStatus]);

			let minimized = statush <= UIStatus.defaultStatusH * Config.raw.window_scale;
			if (UIBase.htabs[TabArea.TabStatus].changed && (UIBase.htabs[TabArea.TabStatus].position == Context.raw.lastStatusPosition || minimized)) {
				UIBase.toggleBrowser();
			}
			Context.raw.lastStatusPosition = UIBase.htabs[TabArea.TabStatus].position;
		}
	}

	static drawVersionTab = (htab: zui_handle_t) => {
		// Version label
		if (!Config.raw.touch_ui) {
			let ui = UIBase.ui;
			ui.enabled = false;
			zui_tab(UIBase.htabs[TabArea.TabStatus], manifest_version);
			ui.enabled = true;
		}
	}
}
