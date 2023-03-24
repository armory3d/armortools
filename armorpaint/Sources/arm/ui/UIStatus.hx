package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;

class UIStatus {

	public static var inst: UIStatus;
	public static inline var defaultStatusH = 32;

	public function new() {
		inst = this;
	}

	@:access(zui.Zui)
	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UIBase.inst.ui;

		var statush = Config.raw.layout[LayoutStatusH];
		if (ui.window(UIBase.inst.hwnds[2], iron.App.x(), System.windowHeight() - statush, System.windowWidth() - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSidebarW], statush)) {
			ui._y += 2;

			// Border
			ui.g.color = ui.t.SEPARATOR_COL;
			ui.g.fillRect(0, 0, 1, ui._windowH);
			ui.g.fillRect(ui._windowW - 1, 0, 1, ui._windowH);

			for (draw in UIBase.inst.hwndTabs[2]) draw(UIBase.inst.htabs[2]);

			var minimized = statush <= defaultStatusH * Config.raw.window_scale;
			if (UIBase.inst.htabs[2].changed && (UIBase.inst.htabs[2].position == Context.raw.lastStatusPosition || minimized)) {
				UIBase.inst.toggleBrowser();
			}
			Context.raw.lastStatusPosition = UIBase.inst.htabs[2].position;
		}
	}
}
