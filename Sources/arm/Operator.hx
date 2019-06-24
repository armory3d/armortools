package arm;

class Operator {

	public static var run = new Map<String, Dynamic>();
	public static var keymap = new Map<String, String>();

	public static function register(name:String, key:String, call:Dynamic) {
		run[name] = call;
		keymap[key] = name;
	}

	public static function update() {

	}
}
