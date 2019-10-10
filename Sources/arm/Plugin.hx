package arm;

// ArmorPaint plugin API

@:keep
class Plugin {

	public static var plugins:Map<String, Plugin> = [];
	static var pluginName:String;

	public var drawUI:zui.Zui->Void = null;
	public var draw:Void->Void = null;
	public var update:Void->Void = null;
	public var delete:Void->Void = null;
	var name:String;

	public function new() {
		name = pluginName;
		plugins.set(name, this);
	}

	public static function start(plugin:String) {
		try {
			iron.data.Data.getBlob("plugins/" + plugin, function(blob:kha.Blob) {
				pluginName = plugin;
				#if js
				untyped __js__("(1, eval)({0})", blob.toString());
				#end
			});
		}
		catch(e:Dynamic) { trace("Failed to load plugin '" + plugin + "'"); trace(e); }
	}

	public static function stop(plugin:String) {
		var p = plugins.get(plugin);
		if (p.delete != null) p.delete();
		plugins.remove(plugin);
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
	public static var Plugin = arm.Plugin;
	public static var Project = arm.Project;
	public static var Layers = arm.Layers;
	public static var History = arm.History;
	public static var Context = arm.Context;
	public static var Log = arm.Log;
}

@:expose("zui")
class ZuiBridge {
	public static var Handle = zui.Zui.Handle;
}
