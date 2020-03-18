package arm;

import kha.Image;
import iron.data.Data;

class Res {

	static var bundled: Map<String, Image> = new Map();

	public static function load(names: Array<String>, done: Void->Void) {
		var loaded = 0;
		for (s in names) {
			Data.getImage(s, function(image: Image) {
				bundled.set(s, image);
				loaded++;
				if (loaded == names.length) done();
			});
		}
	}

	public static inline function get(name: String): Image {
		return bundled.get(name);
	}

	public static function tile50(img: kha.Image, x: Int, y: Int): TRect {
		var size = arm.ui.UISidebar.inst.ui.SCALE() > 1 ? 100 :  50;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	public static function tile18(img: kha.Image, x: Int, y: Int): TRect {
		var size = arm.ui.UISidebar.inst.ui.SCALE() > 1 ? 36 : 18;
		return { x: x * size, y: img.height - (y + 1) * size, w: size, h: size };
	}
}

typedef TRect = {
	public var x: Int;
	public var y: Int;
	public var w: Int;
	public var h: Int;
}
