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
				iron.data.Data.deleteBlob("plugins/" + plugin);
			});
		}
		catch(e:Dynamic) { trace("Failed to load plugin '" + plugin + "'"); trace(e); }
	}

	public static function stop(plugin:String) {
		var p = plugins.get(plugin);
		if (p != null && p.delete != null) p.delete();
		plugins.remove(plugin);
	}
}

@:expose("iron")
class IronBridge {
	public static var App = iron.App;
	public static var Scene = iron.Scene;
	public static var Time = iron.system.Time;
	public static var Input = iron.system.Input;
	public static var ArmPack = iron.system.ArmPack;
	public static var Object = iron.object.Object;
	public static var Data = iron.data.Data;
}

@:expose("arm")
class ArmBridge {
	public static var TypeOf = Type.typeof;
	public static var TObject = TObject;
	public static var Json = haxe.Json;
	public static var ReflectFields = Reflect.fields;
	public static var ReflectField = Reflect.field;
	public static var ReflectSetField = Reflect.setField;
	public static var StdIs = Std.is;
	public static var Bytes = haxe.io.Bytes;
	public static var Blob = kha.Blob;
	public static var Image = kha.Image;
	public static var Scheduler = kha.Scheduler;
	public static var App = arm.App;
	public static var Config = arm.Config;
	public static var Context = arm.Context;
	public static var History = arm.History;
	public static var Layers = arm.Layers;
	public static var Log = arm.Log;
	public static var Operator = arm.Operator;
	public static var Plugin = arm.Plugin;
	public static var Project = arm.Project;
	public static var Res = arm.Res;
	public static var Path = arm.util.Path;
	public static var NodesMaterial = arm.nodes.NodesMaterial;
	public static var Material = arm.nodes.Material;
	public static var Exporter = arm.io.Exporter;
	public static var Importer = arm.io.Importer;
	public static var UITrait = arm.ui.UITrait;
	public static var UINodes = arm.ui.UINodes;
	public static var UIFiles = arm.ui.UIFiles;
	public static function colorFromFloats(r:Float, g:Float, b:Float, a:Float) { return kha.Color.fromFloats(r, g, b, a); }
}

@:expose("zui")
class ZuiBridge {
	public static var Handle = zui.Zui.Handle;
}

@:keep
class Keep {
	public function keep() {
		var a = iron.system.ArmPack.decode;
		var b = iron.system.ArmPack.encode;
	}
}
