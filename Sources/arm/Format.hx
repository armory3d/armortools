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
}
