package arm;

import arm.ui.*;

class Config {

	public static function applyConfig() {
		var C = UITrait.inst.C;
		C.rp_ssgi = UITrait.inst.hssgi.selected;
		C.rp_ssr = UITrait.inst.hssr.selected;
		C.rp_bloom = UITrait.inst.hbloom.selected;
		C.rp_gi = UITrait.inst.hvxao.selected;
		C.rp_supersample = getSuperSampleSize(UITrait.inst.hsupersample.position);
		
		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();
		
		armory.data.Config.save();
		armory.renderpath.RenderPathCreator.applyConfig();
		
		if (current != null) current.begin(false);
		UITrait.inst.ddirty = 2;
	}

	public static inline function getSuperSampleQuality(f:Float):Int {
		return f == 1.0 ? 0 : f == 1.5 ? 1 : f == 2.0 ? 2 : 3;
	}

	public static inline function getSuperSampleSize(i:Int):Float {
		return i == 0 ? 1.0 : i == 1 ? 1.5 : i == 2 ? 2.0 : 4.0;
	}

	public static function getTextureRes():Int {
		var resHandle = UITrait.inst.resHandle;
		if (resHandle.position == 0) return 1024;
		if (resHandle.position == 1) return 2048;
		if (resHandle.position == 2) return 4096;
		if (resHandle.position == 3) return 8192;
		if (resHandle.position == 4) return 16384;
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
		return 0;
	}
}
