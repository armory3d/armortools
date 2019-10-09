package arm;

// ArmorPaint plugin API

@:expose
class Plugin {

	public static var plugins:Array<Plugin> = [];

	public var drawUI:zui.Zui->Void = null;
	public var draw:Void->Void = null;
	public var update:Void->Void = null;

	public static function keep() {}

	public function handle(ops: zui.Zui.HandleOptions = null) {
		return new zui.Zui.Handle(ops);
	}

	public function new() {
		plugins.push(this);
	}

	public function log(s) {
		trace(s);
	}

	public function scene() {
		return iron.Scene.active;
	}
}

@:expose("iron")
class IronBridge {
	public static var App = iron.App;
	public static var Scene = iron.Scene;
	public static var Time = iron.system.Time;
	public static var Input = iron.system.Input;
	public static var Object = iron.object.Object;
	public static var Data = iron.data.Data;
}

@:expose("arm")
class ArmBridge {
	public static var Project = arm.Project;
	public static var Layers = arm.Layers;
	public static var History = arm.History;
	public static var Context = arm.Context;
	public static function log(s:String) { trace(s); };
}

// my_plugin.js
// plugin = new arm.Plugin();
// h = plugin.handle();
// plugin.drawUI = function(ui) {
// 	if (ui.panel(h, "My Plugin")) {
// 		if (ui.button("Hello")) {
// 			plugin.log("World");
// 		}
// 	}
// }
