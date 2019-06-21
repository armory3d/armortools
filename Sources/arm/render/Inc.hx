package arm.render;

import iron.RenderPath;

class Inc {

	static var path:RenderPath;
	public static var superSample = 1.0;

	static var pointIndex = 0;
	static var spotIndex = 0;
	static var lastFrame = -1;

	#if (rp_voxelao && arm_config)
	static var voxelsCreated = false;
	#end

	public static function init(_path:RenderPath) {
		path = _path;

		#if arm_config
		var config = Config.raw;
		superSample = config.rp_supersample;
		#else
		
		#if (rp_supersampling == 1.5)
		superSample = 1.5;
		#elseif (rp_supersampling == 2)
		superSample = 2.0;
		#elseif (rp_supersampling == 4)
		superSample = 4.0;
		#end
		
		#end
	}
	
	public static function applyConfig() {
		#if arm_config
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
		#end // arm_config
	}

	#if rp_voxelao
	public static function initGI(tname = "voxels") {
		#if arm_config
		var config = Config.raw;
		if (config.rp_gi != true || voxelsCreated) return;
		voxelsCreated = true;
		#end

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
		#if (rp_voxelgi_resolution == 512)
		return 512;
		#elseif (rp_voxelgi_resolution == 256)
		return 256;
		#elseif (rp_voxelgi_resolution == 128)
		return 128;
		#elseif (rp_voxelgi_resolution == 64)
		return 64;
		#elseif (rp_voxelgi_resolution == 32)
		return 32;
		#else
		return 0;
		#end
	}

	public static inline function getVoxelResZ():Float {
		#if (rp_voxelgi_resolution_z == 1.0)
		return 1.0;
		#elseif (rp_voxelgi_resolution_z == 0.5)
		return 0.5;
		#elseif (rp_voxelgi_resolution_z == 0.25)
		return 0.25;
		#else
		return 0.0;
		#end
	}

	public static inline function getSuperSampling():Float {
		return superSample;
	}

	public static inline function getHdrFormat():String {
		#if rp_hdr
		return "RGBA64";
		#else
		return "RGBA32";
		#end
	}
}
