package arm.renderpath;

import iron.RenderPath;
import armory.renderpath.Inc;

class RenderPathCreator {

	public static var drawMeshes:Void->Void = RenderPathDeferred.drawMeshes;
	public static var applyConfig:Void->Void = RenderPathDeferred.applyConfig;
	public static var finalTarget:RenderTarget = null;

	public static function get():RenderPath {
		var path = new RenderPath();
		Inc.init(path);
		RenderPathDeferred.init(path);
		path.commands = RenderPathDeferred.commands;
		return path;
	}
}
