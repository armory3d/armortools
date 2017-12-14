package arm;

class App {

	public static function w():Int {
		if (UINodes.show) return Std.int((kha.System.windowWidth() - UITrait.windowW) / 2);
		else return kha.System.windowWidth() - UITrait.windowW;
	}

	public static function h():Int {
		return kha.System.windowHeight();
	}

	public static function realw():Int {
		return kha.System.windowWidth();
	}

	public static function realh():Int {
		return kha.System.windowHeight();
	}
}
