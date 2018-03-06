// Reference: https://github.com/armory3d/armory_docs/blob/master/dev/renderpath.md
package arm.renderpath;

import iron.RenderPath;
import armory.renderpath.Inc;

class RenderPathCreator {

	static var path:RenderPath;

	public static function get():RenderPath {
		path = new RenderPath();
		Inc.init(path);
		#if (rp_renderer == "Forward")
		RenderPathForward.init(path);
		path.commands = RenderPathForward.commands;
		#else
		RenderPathDeferred.init(path);
		path.commands = RenderPathDeferred.commands;
		#end
		return path;
	}

	#if (rp_gi != "Off")
	public static var voxelFrame = 0;
	public static var voxelFreq = 6; // Revoxelizing frequency
	#end
}
