package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import iron.system.Input;

@:access(zui.Zui)
class UIBox {

	public static var show = false;
	public static var hwnd = new Handle();
	public static var boxTitle = "";
	public static var boxText = "";
	public static var boxCommands: Zui->Void = null;
	public static var clickToHide = true;
	static var modalW = 400;
	static var modalH = 170;
	static var draws = 0;

	public static function render(g: kha.graphics2.Graphics) {
		g.end();

		var ui = App.uibox;
		var appw = System.windowWidth();
		var apph = System.windowHeight();
		var mw = Std.int(modalW * ui.SCALE());
		var mh = Std.int(modalH * ui.SCALE());
		var left = Std.int(appw / 2 - mw / 2);
		//var right = Std.int(appw / 2 + mw / 2);
		var top = Std.int(apph / 2 - mh / 2);
		//var bottom = Std.int(apph / 2 + mh / 2);

		if (boxCommands == null) {
			ui.begin(g);
			if (ui.window(hwnd, left, top, mw, mh, true)) {
				ui._y += 10;
				if (ui.tab(Id.handle(), boxTitle)) {
					for (line in boxText.split("\n")) {
						ui.text(line);
					}

					ui.row([2 / 3, 1 / 3]);
					ui.endElement();
					if (ui.button("OK")) {
						show = false;
						App.redrawUI();
					}
				}
			}
			ui.end();
		}
		else {
			ui.begin(g);
			if (ui.window(hwnd, left, top, mw, mh, true)) {
				ui._y += 10;
				boxCommands(ui);
			}
			ui.end();
		}

		g.begin(false);
		draws++;
	}

	public static function update() {
		if (UIMenu.show) return;
		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();
		var ui = App.uibox;
		var inUse = ui.comboSelectedHandle != null;
		var isEscape = kb.started("escape");
		if (draws > 2 && (ui.inputReleased || isEscape) && !inUse && !ui.isTyping) {
			var appw = System.windowWidth();
			var apph = System.windowHeight();
			var mw = Std.int(modalW * ui.SCALE());
			var mh = Std.int(modalH * ui.SCALE());
			var left = (appw / 2 - mw / 2) + hwnd.dragX;
			var right = (appw / 2 + mw / 2) + hwnd.dragX;
			var top = (apph / 2 - mh / 2) + hwnd.dragY;
			var bottom = (apph / 2 + mh / 2) + hwnd.dragY;
			var mx = mouse.x;
			var my = mouse.y;
			if ((clickToHide && (mx < left || mx > right || my < top || my > bottom)) || isEscape) {
				show = false;
				App.redrawUI();
			}
		}
	}

	public static function showMessage(title: String, text: String) {
		init();
		modalW = 400;
		modalH = 170;
		boxTitle = title;
		boxText = text;
		boxCommands = null;
	}

	public static function showCustom(commands: Zui->Void = null, mw = 400, mh = 200) {
		init();
		modalW = mw;
		modalH = mh;
		boxCommands = commands;
	}

	static function init() {
		hwnd.dragX = 0;
		hwnd.dragY = 0;
		show = true;
		draws = 0;
		clickToHide = true;
	}
}
