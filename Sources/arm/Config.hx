package arm;

class Config {

	public static function applyConfig() {
		var C = UITrait.inst.C;
		C.rp_ssgi = UITrait.inst.hssgi.selected;
		C.rp_ssr = UITrait.inst.hssr.selected;
		C.rp_bloom = UITrait.inst.hbloom.selected;
		// var wasOff = C.rp_shadowmap_cascade == 1;
		// C.rp_shadowmap_cascade = getShadowMapSize(UITrait.inst.hshadowmap.position);
		// var light = iron.Scene.active.lights[0];
		// if (C.rp_shadowmap_cascade == 1) {
			// light.data.raw.strength = 0;
		// }
		// else if (wasOff) {
			// light.data.raw.strength = 6.5;
		// }
		C.rp_supersample = getSuperSampleSize(UITrait.inst.hsupersample.position);
		UITrait.inst.ui.g.end();
		armory.data.Config.save();
		armory.renderpath.RenderPathCreator.applyConfig();
		UITrait.inst.ui.g.begin(false);
		UITrait.inst.ddirty = 2;
	}

	// public static inline function getShadowQuality(i:Int):Int {
	// 	// 0 - Ultra, 1- High, 2 - Medium, 3 - Low, 4 - Off
	// 	return i == 8192 ? 0 : i == 4096 ? 1 : i == 2048 ? 2 : i == 1024 ? 3 : 4;
	// }

	// public static inline function getShadowMapSize(i:Int):Int {
	// 	return i == 0 ? 8192 : i == 1 ? 4096 : i == 2 ? 2048 : i == 3 ? 1024 : 1;
	// }

	public static inline function getSuperSampleQuality(f:Float):Int {
		return f == 0.5 ? 0 : f == 1.0 ? 1 : f == 1.5 ? 2 : f == 2.0 ? 3 : 4;
	}

	public static inline function getSuperSampleSize(i:Int):Float {
		return i == 0 ? 0.5 : i == 1 ? 1.0 : i == 2 ? 1.5 : i == 3 ? 2.0 : 4.0;
	}

	public static function getTextureRes():Int {
		var resHandle = UITrait.inst.resHandle;
		if (resHandle.position == 0) return 1024;
		if (resHandle.position == 1) return 2048;
		if (resHandle.position == 2) return 4096;
		if (resHandle.position == 3) return 8192;
		if (resHandle.position == 4) return 16384;
		if (resHandle.position == 5) return 20480;
		return 0;
	}

	public static function getTextureResBias():Float {
		var resHandle = UITrait.inst.resHandle;
		if (resHandle.position == 0) return 2.0;
		if (resHandle.position == 1) return 1.5;
		if (resHandle.position == 2) return 1.0;
		if (resHandle.position == 3) return 0.5;
		if (resHandle.position == 4) return 0.25;
		if (resHandle.position == 5) return 0.125;
		return 1.0;
	}

	public static function getTextureResPos(i:Int):Int {
		if (i == 1024) return 0;
		if (i == 2048) return 1;
		if (i == 4096) return 2;
		if (i == 8192) return 3;
		if (i == 16384) return 4;
		if (i == 20480) return 5;
		return 0;
	}
}
