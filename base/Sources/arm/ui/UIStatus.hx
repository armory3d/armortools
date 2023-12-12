package arm.ui;

import iron.System;
import zui.Zui;

class UIStatus {

	public static var inst: UIStatus;
	public static inline var defaultStatusH = 33;

	public function new() {
		inst = this;
	}

	public function renderUI(g: Graphics2) {
		var ui = UIBase.inst.ui;

		var statush = Config.raw.layout[LayoutStatusH];

		#if (is_paint || is_sculpt)
		if (ui.window(UIBase.inst.hwnds[TabStatus], iron.App.x(), System.height - statush, System.width - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSidebarW], statush)) {
		#end

		#if is_lab
		if (ui.window(UIBase.inst.hwnds[TabStatus], iron.App.x(), System.height - statush, System.width, statush)) {
		#end

			ui._y += 2;

			// Border
			ui.g.color = ui.t.SEPARATOR_COL;
			ui.g.fillRect(0, 0, 1, ui._windowH);
			ui.g.fillRect(ui._windowW - 1, 0, 1, ui._windowH);

			// Draw tabs
			for (draw in UIBase.inst.hwndTabs[TabStatus]) draw(UIBase.inst.htabs[TabStatus]);

			var minimized = statush <= defaultStatusH * Config.raw.window_scale;
			if (UIBase.inst.htabs[TabStatus].changed && (UIBase.inst.htabs[TabStatus].position == Context.raw.lastStatusPosition || minimized)) {
				UIBase.inst.toggleBrowser();
			}
			Context.raw.lastStatusPosition = UIBase.inst.htabs[TabStatus].position;
		}
	}

	public static function drawVersionTab(htab: Handle) {
		// Version label
		if (!Config.raw.touch_ui) {
			var ui = UIBase.inst.ui;
			ui.enabled = false;
			ui.tab(UIBase.inst.htabs[TabStatus], Manifest.version);
			ui.enabled = true;
		}
	}
}
