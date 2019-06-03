package arm.ui;

import zui.Zui;
import zui.Id;
import arm.App;
import arm.util.ViewportUtil;

class UIBox {

	public static var modalW = 625;
	public static var modalH = 545;
	public static var boxText = "";
	public static var boxCommands:Zui->Void = null;

	@:access(zui.Zui)
	public static function render(g:kha.graphics2.Graphics) {

		var uibox = App.uibox;
		var appw = kha.System.windowWidth();
		var apph = kha.System.windowHeight();
		var modalW = Std.int(300 * uibox.SCALE);
		var modalH = Std.int(115 * uibox.SCALE);
		var left = Std.int(appw / 2 - modalW / 2);
		var right = Std.int(appw / 2 + modalW / 2);
		var top = Std.int(apph / 2 - modalH / 2);
		var bottom = Std.int(apph / 2 + modalH / 2);
		
		g.color = uibox.t.SEPARATOR_COL;
		g.fillRect(left, top, modalW, modalH);
		g.end();

		if (boxCommands == null) {
			uibox.begin(g);
			if (uibox.window(App.whandle, left, top, modalW, modalH)) {
				uibox._y += 10;
				for (line in boxText.split("\n")) {
					uibox.text(line);
				}
			}
			uibox.end(false);
			uibox.beginLayout(g, right - Std.int(uibox.ELEMENT_W()), bottom - Std.int(uibox.ELEMENT_H() * 1.2), Std.int(uibox.ELEMENT_W()));
			if (uibox.button("OK")) {
				App.showBox = false;
				App.redrawUI();
			}
			uibox.endLayout(false);
		}
		else {
			uibox.begin(g);
			if (uibox.window(App.whandle, left, top, modalW, modalH)) {
				uibox._y += 20;
				boxCommands(uibox);
			}
			uibox.end();
		}
		
		g.begin(false);
	}

	public static function update() {
		var mouse = iron.system.Input.getMouse();
		if (mouse.released()) {
			var appw = kha.System.windowWidth();
			var apph = kha.System.windowHeight();
			var left = appw / 2 - modalW / 2;
			var right = appw / 2 + modalW / 2;
			var top = apph / 2 - modalH / 2;
			var bottom = apph / 2 + modalH / 2;
			var mx = mouse.x + iron.App.x();
			var my = mouse.y + iron.App.y();
			if (mx < left || mx > right || my < top || my > bottom) {
				App.showFiles = App.showBox = false;
				App.redrawUI();
			}
		}
	}

	public static function newProject() {
		showCustom(function(ui:Zui) {
			ui.text("New Project");
			ui.row([1/2, 1/2]);
			UITrait.inst.newObject = ui.combo(Id.handle(), UITrait.inst.newObjectNames, "Default Object");
			if (ui.button("OK")) {
				Project.projectNew();
				ViewportUtil.scaleToBounds();
				App.showBox = false;
				App.redrawUI();
			}
		});
	}

	public static function showMessage(text:String) {
		App.showBox = true;
		boxText = text;
		boxCommands = null;
	}

	public static function showCustom(commands:Zui->Void = null) {
		App.showBox = true;
		boxCommands = commands;
	}
}
