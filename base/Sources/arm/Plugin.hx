package arm;

import arm.PluginAPI; // Keep included

@:keep
class Plugin {

	public static var plugins: Map<String, Plugin> = [];
	static var pluginName: String;

	public var drawUI: zui.Zui->Void = null;
	public var draw: Void->Void = null;
	public var update: Void->Void = null;
	public var delete: Void->Void = null;
	public var version = "0.1";
	public var apiversion = "0.1";
	var name: String;

	public function new() {
		name = pluginName;
		plugins.set(name, this);
	}

	public static function start(plugin: String) {
		try {
			iron.data.Data.getBlob("plugins/" + plugin, function(blob: kha.Blob) {
				pluginName = plugin;
				#if js
				// js.Syntax.code("(1, eval)({0})", blob.toString()); // Global scope
				js.Syntax.code("eval({0})", blob.toString()); // Local scope
				#end
				iron.data.Data.deleteBlob("plugins/" + plugin);
			});
		}
		catch (e: Dynamic) {
			trace("Failed to load plugin '" + plugin + "'");
			trace(e);
		}
	}

	public static function stop(plugin: String) {
		var p = plugins.get(plugin);
		if (p != null && p.delete != null) p.delete();
		plugins.remove(plugin);
	}
}

@:keep
class Keep {
	public static function keep() {
		return untyped [
			iron.system.ArmPack.decode,
			iron.system.ArmPack.encode,
			arm.ui.UIBox.showMessage
		];
	}
}

@:expose("core")
class CoreBridge {
	public static var Json = haxe.Json;
	public static var ReflectFields = Reflect.fields;
	public static var ReflectField = Reflect.field;
	public static var ReflectSetField = Reflect.setField;
	public static var StdIs = Std.isOfType;
	public static var Bytes = haxe.io.Bytes;
	public static var BytesInput = haxe.io.BytesInput;
	public static var BytesOutput = haxe.io.BytesOutput;
	public static var Blob = kha.Blob;
	public static var Image = kha.Image;
	public static var Scheduler = kha.Scheduler;
	public static function colorFromFloats(r: Float, g: Float, b: Float, a: Float): kha.Color {
		return kha.Color.fromFloats(r, g, b, a);
	}
}

@:expose("iron")
class IronBridge {
	public static var App = iron.App;
	public static var Scene = iron.Scene;
	public static var RenderPath = iron.RenderPath;
	public static var Time = iron.system.Time;
	public static var Input = iron.system.Input;
	public static var ArmPack = iron.system.ArmPack;
	public static var Object = iron.object.Object;
	public static var Data = iron.data.Data;
}

@:expose("zui")
class ZuiBridge {
	public static var Handle = zui.Zui.Handle;
	public static var Zui = zui.Zui;
	public static var Ext = zui.Ext;
}

@:expose("console")
class ConsoleBridge {
	public static var log = arm.Console.log;
	public static var error = arm.Console.error;
}
