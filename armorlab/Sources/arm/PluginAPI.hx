package arm;

class PluginAPI {
	public static function init() {
		var api = js.Syntax.code("arm");
		api.BrushOutputNode = arm.logic.BrushOutputNode;
	}
}

@:keep
class KeepLab {
	public static function keep() {
		var a = App.uiBox.panel;
		return [a];
	}
}
