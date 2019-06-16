package arm.util;

class Path {

	public static function toRelative(from:String, to:String):String {
		from = haxe.io.Path.normalize(from);
		to = haxe.io.Path.normalize(to);
		var a = from.split("/");
		var b = to.split("/");
		while (a[0] == b[0]) {
			a.shift();
			b.shift();
			if (a.length == 0 || b.length == 0) break;
		}
		var base = "";
		for (i in 0...a.length - 1) base += "../";
		base += b.join("/");
		return haxe.io.Path.normalize(base);
	}

	public static function baseDir(path:String):String {
		path = haxe.io.Path.normalize(path);
		var base = path.substr(0, path.lastIndexOf("/") + 1);
		#if krom_windows
		// base = StringTools.replace(base, "/", "\\");
		base = base.substr(0, 2) + "\\" + base.substr(3);
		#end
		return base;
	}
}
