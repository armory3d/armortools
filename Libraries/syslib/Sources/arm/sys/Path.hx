package arm.sys;

import iron.data.Data;

class Path {

	#if krom_windows // no inline for plugin access
	public static var sep = "\\";
	#else
	public static var sep = "/";
	#end

	public static var meshFormats = ["obj", "fbx", "blend"];
	public static var textureFormats = ["jpg", "jpeg", "png", "tga", "bmp", "psd", "gif", "hdr"];

	public static var meshImporters = new Map<String, String->(Dynamic->Void)->Void>();
	public static var textureImporters = new Map<String, String->(kha.Image->Void)->Void>();

	public static var baseColorExt = ["albedo", "alb", "basecol", "basecolor", "diffuse", "diff", "base", "bc", "d", "color", "col"];
	public static var opacityExt = ["opac", "opacity", "alpha"];
	public static var normalMapExt = ["normal", "nor", "n", "nrm"];
	public static var occlusionExt = ["ao", "occlusion", "ambientOcclusion", "o", "occ"];
	public static var roughnessExt = ["roughness", "rough", "r", "rgh"];
	public static var metallicExt = ["metallic", "metal", "metalness", "m", "met"];
	public static var displacementExt = ["displacement", "height", "h", "disp"];

	public static function data(): String {
		return Krom.getFilesLocation() + Path.sep + Data.dataPath;
	}

	public static function toRelative(from: String, to: String): String {
		var a = from.split(Path.sep);
		var b = to.split(Path.sep);
		while (a[0] == b[0]) {
			a.shift();
			b.shift();
			if (a.length == 0 || b.length == 0) break;
		}
		var base = "";
		for (i in 0...a.length - 1) base += ".." + Path.sep;
		base += b.join(Path.sep);
		return base;
	}

	public static function baseDir(path: String): String {
		return path.substr(0, path.lastIndexOf(Path.sep) + 1);
	}

	public static function workingDir(): String {
		#if krom_windows
		var cmd = "cd";
		#else
		var cmd = "echo $PWD";
		#end
		var save = data() + sep + "tmp.txt";
		Krom.sysCommand(cmd + ' > "' + save + '"');
		return haxe.io.Bytes.ofData(Krom.loadBlob(save)).toString().rtrim();
	}

	public static function isMesh(path: String): Bool {
		var p = path.toLowerCase();
		for (s in meshFormats) if (p.endsWith("." + s)) return true;
		return false;
	}

	public static function isTexture(path: String): Bool {
		var p = path.toLowerCase();
		for (s in textureFormats) if (p.endsWith("." + s)) return true;
		return false;
	}

	public static function isFont(path: String): Bool {
		var p = path.toLowerCase();
		return p.endsWith(".ttf") ||
				p.endsWith(".ttc") ||
				p.endsWith(".otf");
	}

	public static function isProject(path: String): Bool {
		var p = path.toLowerCase();
		return p.endsWith(".arm");
	}

	public static function isPlugin(path: String): Bool {
		var p = path.toLowerCase();
		return p.endsWith(".js");
			   // p.endsWith(".wasm") ||
			   // p.endsWith(".zip");
	}

	public static function isJson(path: String): Bool {
		var p = path.toLowerCase();
		return p.endsWith(".json");
	}

	public static function isKnown(path: String): Bool {
		return isMesh(path) || isTexture(path) || isFont(path) || isProject(path) || isPlugin(path);
	}

	static function checkExt(p: String, exts: Array<String>): Bool {
		p = p.replace("-", "_");
		for (ext in exts) if (p.endsWith("_" + ext)) return true;
		return false;
	}

	public static inline function isBaseColorTex(p: String): Bool { return checkExt(p, baseColorExt); }
	public static inline function isOpacityTex(p: String): Bool { return checkExt(p, opacityExt); }
	public static inline function isNormalMapTex(p: String): Bool { return checkExt(p, normalMapExt); }
	public static inline function isOcclusionTex(p: String): Bool { return checkExt(p, occlusionExt); }
	public static inline function isRoughnessTex(p: String): Bool { return checkExt(p, roughnessExt); }
	public static inline function isMetallicTex(p: String): Bool { return checkExt(p, metallicExt); }
	public static inline function isDisplacementTex(p: String): Bool { return checkExt(p, displacementExt); }

	public static function isFolder(p: String): Bool {
		return p.indexOf(".") == -1;
	}

	public static function isProtected(): Bool {
		#if krom_windows
		return Krom.getFilesLocation().indexOf("Program Files") >= 0;
		#else
		return false;
		#end
	}
}
