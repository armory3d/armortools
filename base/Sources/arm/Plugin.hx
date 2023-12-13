package arm;

import iron.ArmPack;
import iron.System;
import iron.Data;

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

	public static function init() {
		var api = js.Syntax.code("arm");
		#if (is_paint || is_sculpt)
		api.ParserMaterial = ParserMaterial;
		api.NodesMaterial = NodesMaterial;
		api.UIView2D = UIView2D;
		api.UtilRender = UtilRender;
		#end
		#if is_paint
		api.UtilUV = UtilUV;
		#end
		#if is_lab
		api.BrushOutputNode = arm.nodes.BrushOutputNode;
		#end
	}

	public static function start(plugin: String) {
		try {
			Data.getBlob("plugins/" + plugin, function(blob: js.lib.ArrayBuffer) {
				pluginName = plugin;
				// js.Syntax.code("(1, eval)({0})", System.bufferToString(blob)); // Global scope
				js.Syntax.code("eval({0})", System.bufferToString(blob)); // Local scope
				Data.deleteBlob("plugins/" + plugin);
			});
		}
		catch (e: Dynamic) {
			Console.error(tr("Failed to load plugin") + " '" + plugin + "'");
			Krom.log(e);
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
			ArmPack.decode,
			ArmPack.encode,
			UIBox.showMessage
		];
	}
}

#if is_lab
@:keep
class KeepLab {
	public static function keep() {
		var a = Base.uiBox.panel;
		return [a];
	}
}
#end

@:expose("core")
class CoreBridge {
	public static var Json = haxe.Json;
	public static var Image = Image;
	public static var System = System;
	public static function colorFromFloats(r: Float, g: Float, b: Float, a: Float): Color {
		return Color.fromFloats(r, g, b, a);
	}
}

@:expose("iron")
class IronBridge {
	public static var App = iron.App;
	public static var Scene = iron.Scene;
	public static var RenderPath = iron.RenderPath;
	public static var Time = iron.Time;
	public static var Input = iron.Input;
	public static var ArmPack = iron.ArmPack;
	public static var Object = iron.Object;
	public static var Data = iron.Data;
}

@:expose("zui")
class ZuiBridge {
	public static var Handle = zui.Zui.Handle;
	public static var Zui = zui.Zui;
}

@:expose("console")
class ConsoleBridge {
	public static var log = Console.log;
	public static var error = Console.error;
}

@:expose("arm")
class ArmBridge {
	public static var Base = Base;
	public static var Config = Config;
	public static var Context = Context;
	public static var History = History;
	public static var Operator = Operator;
	public static var Plugin = Plugin;
	public static var Project = Project;
	public static var Res = Res;
	public static var Path = Path;
	public static var File = File;
	public static var NodesBrush = NodesBrush;
	public static var ParserLogic = ParserLogic;
	public static var UIBase = UIBase;
	public static var UINodes = UINodes;
	public static var UIFiles = UIFiles;
	public static var UIMenu = UIMenu;
	public static var UIBox = UIBox;
	public static var UtilMesh = UtilMesh;
	public static var Viewport = Viewport;
	#if (krom_direct3d12 || krom_vulkan || krom_metal)
	public static var RenderPathRaytrace = RenderPathRaytrace;
	#end
}
