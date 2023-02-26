package arm;

class PluginAPI {
	public static function init() {
		var api = js.Syntax.code("arm");
		api.MaterialParser = arm.shader.MaterialParser;
		api.NodesMaterial = arm.shader.NodesMaterial;
		api.UIView2D = arm.ui.UIView2D;
		api.RenderUtil = arm.util.RenderUtil;
		api.UVUtil = arm.util.UVUtil;
	}
}
