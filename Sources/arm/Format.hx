package arm;

class Format {
	public static function checkMeshFormat(path:String):Bool {
		var p = path.toLowerCase();
		return StringTools.endsWith(p, ".obj") ||
			   StringTools.endsWith(p, ".fbx") ||
			   StringTools.endsWith(p, ".blend") ||
			   StringTools.endsWith(p, ".gltf");
	}

	public static function checkTextureFormat(path:String):Bool {
		var p = path.toLowerCase();
		return StringTools.endsWith(p, ".jpg") ||
			   StringTools.endsWith(p, ".png") ||
			   StringTools.endsWith(p, ".tga") ||
			   StringTools.endsWith(p, ".hdr");
	}

	public static function checkFontFormat(path:String):Bool {
		var p = path.toLowerCase();
		return StringTools.endsWith(p, ".ttf");
	}

	public static function checkProjectFormat(path:String):Bool {
		var p = path.toLowerCase();
		return StringTools.endsWith(p, ".arm");
	}

	public static function checkBaseTex(p:String):Bool {
		return StringTools.endsWith(p, "_albedo") ||
			   StringTools.endsWith(p, "_alb") ||
			   StringTools.endsWith(p, "_basecol") ||
			   StringTools.endsWith(p, "_basecolor") ||
			   StringTools.endsWith(p, "_diffuse") ||
			   StringTools.endsWith(p, "_diff") ||
			   StringTools.endsWith(p, "_base") ||
			   StringTools.endsWith(p, "_bc") ||
			   StringTools.endsWith(p, "_d") ||
			   StringTools.endsWith(p, "_color") ||
			   StringTools.endsWith(p, "_col");
	}

	public static function checkOpacTex(p:String):Bool {
		return StringTools.endsWith(p, "_opac") ||
			   StringTools.endsWith(p, "_alpha") ||
			   StringTools.endsWith(p, "_opacity") ||
			   StringTools.endsWith(p, "_mask");
	}

	public static function checkNorTex(p:String):Bool {
		return StringTools.endsWith(p, "_normal") ||
			   StringTools.endsWith(p, "_nor") ||
			   StringTools.endsWith(p, "_n") ||
			   StringTools.endsWith(p, "_nrm");
	}

	public static function checkOccTex(p:String):Bool {
		return StringTools.endsWith(p, "_ao") ||
			   StringTools.endsWith(p, "_occlusion") ||
			   StringTools.endsWith(p, "_o") ||
			   StringTools.endsWith(p, "_occ");
	}

	public static function checkRoughTex(p:String):Bool {
		return StringTools.endsWith(p, "_roughness") ||
			   StringTools.endsWith(p, "_roug") ||
			   StringTools.endsWith(p, "_r") ||
			   StringTools.endsWith(p, "_rough") ||
			   StringTools.endsWith(p, "_rgh");
	}

	public static function checkMetTex(p:String):Bool {
		return StringTools.endsWith(p, "_metallic") ||
			   StringTools.endsWith(p, "_metal") ||
			   StringTools.endsWith(p, "_metalness") ||
			   StringTools.endsWith(p, "_m") ||
			   StringTools.endsWith(p, "_met");
	}

	public static function checkDispTex(p:String):Bool {
		return StringTools.endsWith(p, "_displacement") ||
			   StringTools.endsWith(p, "_height") ||
			   StringTools.endsWith(p, "_h") ||
			   StringTools.endsWith(p, "_disp");
	}
}
