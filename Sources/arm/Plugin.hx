package arm;

// ArmorPaint plugin API

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
		catch (e: Dynamic) { trace("Failed to load plugin '" + plugin + "'"); trace(e); }
	}

	public static function stop(plugin: String) {
		var p = plugins.get(plugin);
		if (p != null && p.delete != null) p.delete();
		plugins.remove(plugin);
	}
}

@:expose("core")
class CoreBridge {
	public static var Json = haxe.Json;
	public static var ReflectFields = Reflect.fields;
	public static var ReflectField = Reflect.field;
	public static var ReflectSetField = Reflect.setField;
	public static var StdIs = Std.is;
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

@:expose("arm")
class ArmBridge {
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
	public static var Path = arm.sys.Path;
	public static var File = arm.sys.File;
	public static var NodesMaterial = arm.shader.NodesMaterial;
	public static var NodesBrush = arm.node.NodesBrush;
	public static var MaterialParser = arm.shader.MaterialParser;
	public static var MakeMaterial = arm.node.MakeMaterial;
	public static var Brush = arm.node.Brush;
	public static var UISidebar = arm.ui.UISidebar;
	public static var UINodes = arm.ui.UINodes;
	public static var UIFiles = arm.ui.UIFiles;
	public static var UIMenu = arm.ui.UIMenu;
	public static var UIView2D = arm.ui.UIView2D;
	public static var MeshUtil = arm.util.MeshUtil;
	public static var MaterialUtil = arm.util.MaterialUtil;
	public static var RenderUtil = arm.util.RenderUtil;
	public static var UVUtil = arm.util.UVUtil;
	public static var ViewportUtil = arm.util.ViewportUtil;
	public static var PngWriter = arm.format.PngWriter;
	public static var PngTools = arm.format.PngTools;
}

@:expose("zui")
class ZuiBridge {
	public static var Handle = zui.Zui.Handle;
	public static var Zui = zui.Zui;
	public static var Ext = zui.Ext;
}

@:keep
class Keep {
	public static function keep() {
		var x = iron.system.ArmPack.decode;
		var y = iron.system.ArmPack.encode;
		return [x, y];
	}
}
