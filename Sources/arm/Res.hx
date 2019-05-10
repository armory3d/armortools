package arm;

class Res {

	static var bundled:Map<String, kha.Image> = new Map();

	public static function load(names:Array<String>, done:Void->Void) {
		var loaded = 0;
		for (s in names) {
			iron.data.Data.getImage(s, function(image:kha.Image) {
				bundled.set(s, image);
				loaded++;
				if (loaded == names.length) done();
			});
		}
	}

	public static inline function get(name:String):kha.Image {
		return bundled.get(name);
	}
}
