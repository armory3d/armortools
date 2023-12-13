package arm;

import iron.System;
import iron.Data;

class Res {

	public static var bundled: Map<String, Image> = new Map();

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

	public static function tile50(img: Image, x: Int, y: Int): TRect {
		var size = Config.raw.window_scale > 1 ? 100 : 50;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	public static function tile25(img: Image, x: Int, y: Int): TRect {
		var size = Config.raw.window_scale > 1 ? 50 : 25;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	public static function tile18(img: Image, x: Int, y: Int): TRect {
		var size = Config.raw.window_scale > 1 ? 36 : 18;
		return { x: x * size, y: img.height - (y + 1) * size, w: size, h: size };
	}

	#if arm_snapshot
	public static function embedRaw(handle: String, name: String, file: js.lib.ArrayBuffer) {
		Data.cachedBlobs.set(name, file);
		Data.getSceneRaw(handle, function(_) {});
		Data.cachedBlobs.remove(name);
	}

	public static function embedBlob(name: String, file: js.lib.ArrayBuffer) {
		Data.cachedBlobs.set(name, file);
	}

	public static function embedFont(name: String, file: js.lib.ArrayBuffer) {
		Data.cachedFonts.set(name, new Font(file));
	}
	#end
}

typedef TRect = {
	public var x: Int;
	public var y: Int;
	public var w: Int;
	public var h: Int;
}
