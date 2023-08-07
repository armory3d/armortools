package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;

class UIStatus {

	public static var inst: UIStatus;
	public static inline var defaultStatusH = 33;

	public function new() {
		inst = this;
	}

	@:access(zui.Zui)
	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UIBase.inst.ui;

		var statush = Config.raw.layout[LayoutStatusH];

		#if (is_paint || is_sculpt)
		if (ui.window(UIBase.inst.hwnds[TabStatus], iron.App.x(), System.windowHeight() - statush, System.windowWidth() - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSidebarW], statush)) {
		#end

		#if is_lab
		if (ui.window(UIBase.inst.hwnds[TabStatus], iron.App.x(), System.windowHeight() - statush, System.windowWidth(), statush)) {
		#end

			ui._y += 2;

			// Border
			ui.g.color = ui.t.SEPARATOR_COL;
			ui.g.fillRect(0, 0, 1, ui._windowH);
			ui.g.fillRect(ui._windowW - 1, 0, 1, ui._windowH);

			// Draw tabs
			for (draw in UIBase.inst.hwndTabs[TabStatus]) draw(UIBase.inst.htabs[TabStatus]);

			// Version label
			if (!Config.raw.touch_ui) {
				ui.enabled = false;
				ui.tab(UIBase.inst.htabs[TabStatus], Manifest.version);
				ui.enabled = true;
			}

			var minimized = statush <= defaultStatusH * Config.raw.window_scale;
			if (UIBase.inst.htabs[TabStatus].changed && (UIBase.inst.htabs[TabStatus].position == Context.raw.lastStatusPosition || minimized)) {
				UIBase.inst.toggleBrowser();
			}
			Context.raw.lastStatusPosition = UIBase.inst.htabs[TabStatus].position;
		}
	}
}
