package arm;

import iron.system.Input;

class Operator {

	public static var run = new Map<String, Dynamic>();
	public static var keymap = new Map<String, String>();

	public static function register(name: String, key: String, call: Dynamic) {
		run[name] = call;
		keymap[key] = name;
	}

	public static function update() {}

	public static function shortcut(s: String, type = ShortcutStarted): Bool {
		if (s == "") return false;
		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();
		var shift = s.indexOf("shift") >= 0;
		var ctrl = s.indexOf("ctrl") >= 0;
		var alt = s.indexOf("alt") >= 0;
		var flag = shift == kb.down("shift") &&
				   ctrl == kb.down("control") &&
				   alt == kb.down("alt");
		if (s.indexOf("+") > 0) {
			s = s.substr(s.lastIndexOf("+") + 1);
		}
		else if (shift || ctrl || alt) return flag;
		var key = (s == "left" || s == "right" || s == "middle") ?
			// Mouse
			(type == ShortcutDown ? mouse.down(s) : mouse.started(s)) :
			// Keyboard
			(type == ShortcutRepeat ? kb.repeat(s) : type == ShortcutDown ? kb.down(s) : type == ShortcutReleased ? kb.released(s) : kb.started(s));
		return flag && key;
	}
}

@:enum abstract ShortcutType(Int) from Int to Int {
	var ShortcutStarted = 0;
	var ShortcutRepeat = 1;
	var ShortcutDown = 2;
	var ShortcutReleased = 3;
}
