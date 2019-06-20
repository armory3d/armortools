package arm;

import kha.Image;
import iron.data.Data;

class Res {

	static var bundled:Map<String, Image> = new Map();

	public static function load(names:Array<String>, done:Void->Void) {
		var loaded = 0;
		for (s in names) {
			Data.getImage(s, function(image:Image) {
				bundled.set(s, image);
				loaded++;
				if (loaded == names.length) done();
			});
		}
	}

	public static inline function get(name:String):Image {
		return bundled.get(name);
	}
}
