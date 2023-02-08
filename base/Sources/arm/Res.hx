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
		var size = Config.raw.window_scale > 1 ? 100 :  50;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	public static function tile18(img: kha.Image, x: Int, y: Int): TRect {
		var size = Config.raw.window_scale > 1 ? 36 : 18;
		return { x: x * size, y: img.height - (y + 1) * size, w: size, h: size };
	}

	#if arm_snapshot
	public static function embedRaw(handle: String, name: String, file: js.lib.ArrayBuffer) {
		iron.data.Data.cachedBlobs.set(name, kha.Blob.fromBytes(haxe.io.Bytes.ofData(file)));
		iron.data.Data.getSceneRaw(handle, function(_) {});
		iron.data.Data.cachedBlobs.remove(name);
	}
	public static function embedBlob(name: String, file: js.lib.ArrayBuffer) {
		iron.data.Data.cachedBlobs.set(name, kha.Blob.fromBytes(haxe.io.Bytes.ofData(file)));
	}
	public static function embedFont(name: String, file: js.lib.ArrayBuffer) {
		iron.data.Data.cachedFonts.set(name, new kha.Font(kha.Blob.fromBytes(haxe.io.Bytes.ofData(file))));
	}
	#end
}

typedef TRect = {
	public var x: Int;
	public var y: Int;
	public var w: Int;
	public var h: Int;
}
