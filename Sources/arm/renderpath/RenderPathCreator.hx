package arm.renderpath;

import iron.RenderPath;
import armory.renderpath.Inc;

class RenderPathCreator {

	public static var drawMeshes:Void->Void = armory.renderpath.RenderPathDeferred.drawMeshes;
	public static var applyConfig:Void->Void = armory.renderpath.RenderPathDeferred.applyConfig;

	public static function get():RenderPath {
		var path = new RenderPath();
		Inc.init(path);
		RenderPathDeferred.init(path);
		path.commands = RenderPathDeferred.commands;
		return path;
	}

	#if (rp_gi != "Off")
	public static var voxelFrame = 0;
	public static var voxelFreq = 6; // Revoxelizing frequency
	#end

	public static var finalTarget:RenderTarget = null;
}
