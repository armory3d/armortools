package arm.util;

using StringTools;

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
		base = base.substr(0, 2) + "\\" + base.substr(3);
		#end
		return base;
	}

	public static function checkMeshFormat(path:String):Bool {
		var p = path.toLowerCase();
		return p.endsWith(".obj") ||
			   p.endsWith(".fbx") ||
			   p.endsWith(".blend");
	}

	public static function checkTextureFormat(path:String):Bool {
		var p = path.toLowerCase();
		return p.endsWith(".jpg") ||
			   p.endsWith(".png") ||
			   p.endsWith(".tga") ||
			   p.endsWith(".hdr");
	}

	public static function checkFontFormat(path:String):Bool {
		var p = path.toLowerCase();
		return p.endsWith(".ttf");
	}

	public static function checkProjectFormat(path:String):Bool {
		var p = path.toLowerCase();
		return p.endsWith(".arm");
	}

	public static function checkBaseTex(p:String):Bool {
		return p.endsWith("_albedo") ||
			   p.endsWith("-albedo") ||
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

	public static function checkOpacTex(p:String):Bool {
		return p.endsWith("_opac") ||
			   p.endsWith("_alpha") ||
			   p.endsWith("_opacity") ||
			   p.endsWith("_mask");
	}

	public static function checkNorTex(p:String):Bool {
		return p.endsWith("_normal") ||
			   p.endsWith("-normal") ||
			   p.endsWith("-normal-dx") ||
			   p.endsWith("_normal-dx") ||
			   p.endsWith("-normal4-ue") ||
			   p.endsWith("-normal-ogl") ||
			   p.endsWith("_normal-ogl") ||
			   p.endsWith("_nor") ||
			   p.endsWith("_n") ||
			   p.endsWith("_nrm");
	}

	public static function checkOccTex(p:String):Bool {
		return p.endsWith("_ao") ||
			   p.endsWith("-ao") ||
			   p.endsWith("_occlusion") ||
			   p.endsWith("_o") ||
			   p.endsWith("_occ");
	}

	public static function checkRoughTex(p:String):Bool {
		return p.endsWith("_roughness") ||
			   p.endsWith("-roughness") ||
			   p.endsWith("_roug") ||
			   p.endsWith("_r") ||
			   p.endsWith("_rough") ||
			   p.endsWith("_rgh");
	}

	public static function checkMetTex(p:String):Bool {
		return p.endsWith("_metallic") ||
			   p.endsWith("-metallic") ||
			   p.endsWith("_metal") ||
			   p.endsWith("_metalness") ||
			   p.endsWith("-metalness") ||
			   p.endsWith("_m") ||
			   p.endsWith("_met");
	}

	public static function checkDispTex(p:String):Bool {
		return p.endsWith("_displacement") ||
			   p.endsWith("-height") ||
			   p.endsWith("_height") ||
			   p.endsWith("_h") ||
			   p.endsWith("_disp");
	}
}
