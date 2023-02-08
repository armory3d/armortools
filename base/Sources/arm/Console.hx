package arm;

import arm.Enums;

@:keep
class Console {

	public static var message = "";
	public static var messageTimer = 0.0;
	public static var messageColor = 0x00000000;
	public static var lastTraces: Array<String> = [""];
	static var haxeTrace: Dynamic->haxe.PosInfos->Void = null;

	public static function toast(s: String, g: kha.graphics2.Graphics = null) {
		// Show a popup message
		function _render(g: kha.graphics2.Graphics) {
			g.color = 0x55000000;
			g.fillRect(0, 0, kha.System.windowWidth(), kha.System.windowHeight());
			var scale = arm.App.getUIs()[0].SCALE();
			var x = kha.System.windowWidth() / 2;
			var y = kha.System.windowHeight() - 200 * scale;
			g.fillRect(x - 200 * scale, y, 400 * scale, 80 * scale);
			g.font = App.font;
			g.fontSize = Std.int(22 * scale);
			g.color = 0xffffffff;
			g.drawString(s, x - g.font.width(g.fontSize, s) / 2, y + 40 * scale - g.font.height(g.fontSize) / 2);
			arm.App.notifyOnNextFrame(function() {
				iron.App.removeRender2D(_render);
			});
		}
		g != null ? _render(g) : iron.App.notifyOnRender2D(_render);
		consoleTrace(s);
	}

	public static function info(s: String) {
		messageTimer = 5.0;
		message = s;
		messageColor = 0x00000000;
		App.redrawStatus();
		consoleTrace(s);
	}

	public static function error(s: String) {
		messageTimer = 8.0;
		message = s;
		messageColor = 0xffaa0000;
		App.redrawStatus();
		consoleTrace(s);
	}

	public static function log(s: String) {
		consoleTrace(s);
	}

	public static function init() {
		if (haxeTrace == null) {
			haxeTrace = haxe.Log.trace;
			haxe.Log.trace = consoleTrace;
		}
	}

	static function consoleTrace(v: Dynamic, ?inf: haxe.PosInfos) {
		App.redrawConsole();
		lastTraces.unshift(Std.string(v));
		if (lastTraces.length > 100) lastTraces.pop();
		haxeTrace(v, inf);
	}
}
