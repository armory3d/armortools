package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import iron.system.Input;
import arm.util.ViewportUtil;

class UIBox {

	public static var show = false;
	public static var hwnd = new Handle();
	public static var boxTitle = "";
	public static var boxText = "";
	public static var boxCommands:Zui->Void = null;
	static var modalW = 400;
	static var modalH = 170;

	@:access(zui.Zui)
	public static function render(g:kha.graphics2.Graphics) {

		var uibox = App.uibox;
		var appw = System.windowWidth();
		var apph = System.windowHeight();
		modalW = Std.int(400 * uibox.SCALE);
		modalH = Std.int(170 * uibox.SCALE);
		var left = Std.int(appw / 2 - modalW / 2);
		var right = Std.int(appw / 2 + modalW / 2);
		var top = Std.int(apph / 2 - modalH / 2);
		var bottom = Std.int(apph / 2 + modalH / 2);

		g.end();

		if (boxCommands == null) {
			uibox.begin(g);
			if (uibox.window(hwnd, left, top, modalW, modalH)) {
				uibox._y += 10;
				if (uibox.tab(Id.handle(), boxTitle)) {
					for (line in boxText.split("\n")) {
						uibox.text(line);
					}

					uibox.row([2/3, 1/3]);
					uibox.endElement();
					if (uibox.button("OK")) {
						UIBox.show = false;
						App.redrawUI();
					}
				}
			}
			uibox.end();
		}
		else {
			uibox.begin(g);
			if (uibox.window(hwnd, left, top, modalW, modalH)) {
				uibox._y += 10;
				boxCommands(uibox);
			}
			uibox.end();
		}

		g.begin(false);
	}

	public static function update() {
		var mouse = Input.getMouse();
		var inUse = @:privateAccess App.uibox.comboSelectedHandle != null;
		if (App.uibox.inputReleased && !inUse) {
			var appw = System.windowWidth();
			var apph = System.windowHeight();
			var left = appw / 2 - modalW / 2;
			var right = appw / 2 + modalW / 2;
			var top = apph / 2 - modalH / 2;
			var bottom = apph / 2 + modalH / 2;
			var mx = mouse.x;
			var my = mouse.y;
			if (mx < left || mx > right || my < top || my > bottom) {
				UIBox.show = false;
				App.redrawUI();
			}
		}
	}

	public static function showMessage(title:String, text:String) {
		UIBox.show = true;
		boxTitle = title;
		boxText = text;
		boxCommands = null;
	}

	public static function showCustom(commands:Zui->Void = null) {
		UIBox.show = true;
		boxCommands = commands;
	}
}
