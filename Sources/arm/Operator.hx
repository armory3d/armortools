package arm;

import iron.system.Input;

class Operator {

	public static var run = new Map<String, Dynamic>();
	public static var keymap = new Map<String, String>();

	public static function register(name:String, key:String, call:Dynamic) {
		run[name] = call;
		keymap[key] = name;
	}

	public static function update() {

	}

	public static function shortcut(s:String):Bool {
		var kb = Input.getKeyboard();
		var mouse = Input.getMouse();
		var flag = true;
		var plus = s.indexOf("+");
		if (plus > 0) {
			var shift = s.indexOf("shift") >= 0;
			var ctrl = s.indexOf("ctrl") >= 0;
			// var alt = s.indexOf("alt") >= 0;
			flag = shift == kb.down("shift") && ctrl == kb.down("control");
			s = s.substr(s.lastIndexOf("+") + 1);
		}
		var key = (s == "left" || s == "right" || s == "middle") ? mouse.down(s) : kb.started(s);
		return flag && key;
	}
}
