package arm.render;

import iron.RenderPath;

class Inc {

	static var path:RenderPath;
	public static var superSample = 1.0;

	static var pointIndex = 0;
	static var spotIndex = 0;
	static var lastFrame = -1;

	#if rp_voxelao
	static var voxelsCreated = false;
	#end

	public static function init(_path:RenderPath) {
		path = _path;
		var config = Config.raw;
		superSample = config.rp_supersample;
	}
	
	public static function applyConfig() {
		var config = Config.raw;
		if (superSample != config.rp_supersample) {
			superSample = config.rp_supersample;
			for (rt in path.renderTargets) {
				if (rt.raw.width == 0 && rt.raw.scale != null) {
					rt.raw.scale = getSuperSampling();
				}
			}
			path.resize();
		}
		// Init voxels
		#if rp_voxelao
		if (!voxelsCreated) initGI();
		#end
	}

	#if rp_voxelao
	public static function initGI(tname = "voxels") {
		var config = Config.raw;
		if (config.rp_gi != true || voxelsCreated) return;
		voxelsCreated = true;

		var t = new RenderTargetRaw();
		t.name = tname;
		t.format = "R8";
		var res = getVoxelRes();
		var resZ =  getVoxelResZ();
		t.width = res;
		t.height = res;
		t.depth = Std.int(res * resZ);
		t.is_image = true;
		t.mipmaps = true;
		path.createRenderTarget(t);

		#if arm_voxelgi_temporal
		{
			var tB = new RenderTargetRaw();
			tB.name = t.name + "B";
			tB.format = t.format;
			tB.width = t.width;
			tB.height = t.height;
			tB.depth = t.depth;
			tB.is_image = t.is_image;
			tB.mipmaps = t.mipmaps;
			path.createRenderTarget(tB);
		}
		#end
	}
	#end

	public static inline function getVoxelRes():Int {
		return 256;
	}

	public static inline function getVoxelResZ():Float {
		return 1.0;
	}

	public static inline function getSuperSampling():Float {
		return superSample;
	}

	public static inline function getHdrFormat():String {
		return "RGBA64";
	}
}
