package arm;

class Log {

	public static var message = "";
	public static var messageTimer = 0.0;
	public static var messageColor = 0x00000000;

	public static function showMessage(s:String) {
		messageTimer = 8.0;
		message = s;
		messageColor = 0x00000000;
		arm.ui.UITrait.inst.statusHandle.redraws = 2;
	}

	public static function showError(s:String) {
		messageTimer = 8.0;
		message = s;
		messageColor = 0xffff0000;
		arm.ui.UITrait.inst.statusHandle.redraws = 2;
	}

	public static function trace(s:String) {
		trace(s);
	}
}
