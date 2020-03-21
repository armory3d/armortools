package arm;

@:keep
class Log {

	public static var message = "";
	public static var messageTimer = 0.0;
	public static var messageColor = 0x00000000;
	public static var lastTraces: Array<String> = [""];
	static var haxeTrace: Dynamic->haxe.PosInfos->Void = null;

	public static function info(s: String) {
		messageTimer = 5.0;
		message = s;
		messageColor = 0x00000000;
		trace(s);
	}

	public static function error(s: String) {
		messageTimer = 8.0;
		message = s;
		messageColor = 0xffaa0000;
		trace(s);
	}

	public static function trace(s: String) {
		trace(s);
	}

	public static function init() {
		if (haxeTrace == null) {
			haxeTrace = haxe.Log.trace;
			haxe.Log.trace = consoleTrace;
		}
	}

	static function consoleTrace(v: Dynamic, ?inf: haxe.PosInfos) {
		lastTraces.unshift(Std.string(v));
		if (lastTraces.length > 10) lastTraces.pop();
		haxeTrace(v, inf);
		if (arm.ui.UIStatus.inst != null) {
			arm.ui.UIStatus.inst.statusHandle.redraws = 2;
		}
	}
}
