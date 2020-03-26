package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import arm.Enums;

class UIStatus {

	public static var inst: UIStatus;

	public static inline var defaultStatusH = 32;

	public var statusHandle = new Handle();
	public var statush = defaultStatusH;
	public var statustab = Id.handle();

	public function new() {
		inst = this;
	}

	@:access(zui.Zui)
	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UISidebar.inst.ui;

		if (ui.window(statusHandle, iron.App.x(), System.windowHeight() - statush, System.windowWidth() - UIToolbar.inst.toolbarw - UISidebar.inst.windowW, statush)) {
			ui._y += 2;

			TabBrowser.draw();
			TabScript.draw();
			TabConsole.draw();

			var minimized = statush <= defaultStatusH * Config.raw.window_scale;
			if (statustab.changed && (statustab.position == Context.lastStatusPosition || minimized)) {
				UISidebar.inst.toggleBrowser();
			}
			Context.lastStatusPosition = statustab.position;
		}
	}
}
