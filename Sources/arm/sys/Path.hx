package arm.sys;

import iron.data.Data;
using StringTools;

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

	public static function data(): String {
		#if krom_windows
		var path = Data.dataPath.replace("/", "\\");
		#else
		var path = Data.dataPath;
		#end
		return Krom.getFilesLocation() + Path.sep + path;
	}

	public static function toRelative(from: String, to: String): String {
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

	public static function baseDir(path: String): String {
		path = haxe.io.Path.normalize(path);
		var base = path.substr(0, path.lastIndexOf("/") + 1);
		#if krom_windows
		base = base.substr(0, 2) + "\\" + base.substr(3);
		#end
		return base;
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
		return p.endsWith(".ttf");
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

	public static function isBaseTex(p: String): Bool {
		return p.endsWith("_albedo") ||
			   p.endsWith("_alb") ||
			   p.endsWith("_basecol") ||
			   p.endsWith("_basecolor") ||
			   p.endsWith("_diffuse") ||
			   p.endsWith("_diff") ||
			   p.endsWith("_base") ||
			   p.endsWith("_bc") ||
			   p.endsWith("_d") ||
			   p.endsWith("_color") ||
			   p.endsWith("_col");
	}

	public static function isOpacTex(p: String): Bool {
		return p.endsWith("_opac") ||
			   p.endsWith("_alpha") ||
			   p.endsWith("_opacity");
	}

	public static function isNorTex(p: String): Bool {
		return p.endsWith("_normal") ||
			   p.endsWith("_nor") ||
			   p.endsWith("_n") ||
			   p.endsWith("_nrm");
	}

	public static function isOccTex(p: String): Bool {
		return p.endsWith("_ao") ||
			   p.endsWith("_occlusion") ||
			   p.endsWith("_ambientOcclusion") ||
			   p.endsWith("_o") ||
			   p.endsWith("_occ");
	}

	public static function isRoughTex(p: String): Bool {
		return p.endsWith("_roughness") ||
			   p.endsWith("_roug") ||
			   p.endsWith("_r") ||
			   p.endsWith("_rough") ||
			   p.endsWith("_rgh");
	}

	public static function isMetTex(p: String): Bool {
		return p.endsWith("_metallic") ||
			   p.endsWith("_metal") ||
			   p.endsWith("_metalness") ||
			   p.endsWith("_m") ||
			   p.endsWith("_met");
	}

	public static function isDispTex(p: String): Bool {
		return p.endsWith("_displacement") ||
			   p.endsWith("_height") ||
			   p.endsWith("_h") ||
			   p.endsWith("_disp");
	}

	public static function isFolder(p: String): Bool {
		return p.indexOf(".") == -1;
	}

	#if krom_windows
	public static function isAscii(s: String): Bool {
		for (i in 0...s.length) if (s.charCodeAt(i) > 127) return false;
		return true;
	}

	public static function shortPath(s: String): String {
		var cmd = 'for %I in ("' + s + '") do echo %~sI';
		var save = data() + sep + "tmp.txt";
		Krom.sysCommand(cmd + ' > "' + save + '"');
		return haxe.io.Bytes.ofData(Krom.loadBlob(save)).toString().rtrim();
	}
	#end
}
