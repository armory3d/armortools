
// @:keep
class Plugin {

	static plugins: Map<string, Plugin> = new Map();
	static pluginName: string;

	drawUI: (ui: Zui)=>void = null;
	draw: ()=>void = null;
	update: ()=>void = null;
	delete: ()=>void = null;
	version = "0.1";
	apiversion = "0.1";
	name: string;

	constructor() {
		this.name = Plugin.pluginName;
		Plugin.plugins.set(this.name, this);
	}

	static init = () => {
		// let api = arm;
		///if (is_paint || is_sculpt)
		// api.ParserMaterial = ParserMaterial;
		// api.NodesMaterial = NodesMaterial;
		// api.UIView2D = UIView2D;
		// api.UtilRender = UtilRender;
		///end
		///if is_paint
		// api.UtilUV = UtilUV;
		///end
		///if is_lab
		// api.BrushOutputNode = BrushOutputNode;
		///end
	}

	static start = (plugin: string) => {
		try {
			Data.getBlob("plugins/" + plugin, (blob: ArrayBuffer) => {
				Plugin.pluginName = plugin;
				// (1, eval)(System.bufferToString(blob)); // Global scope
				eval(System.bufferToString(blob)); // Local scope
				Data.deleteBlob("plugins/" + plugin);
			});
		}
		catch (e: any) {
			Console.error(tr("Failed to load plugin") + " '" + plugin + "'");
			Krom.log(e);
		}
	}

	static stop = (plugin: string) => {
		let p = Plugin.plugins.get(plugin);
		if (p != null && p.delete != null) p.delete();
		Plugin.plugins.delete(plugin);
	}
}

// @:keep
class Keep {
	static keep = () => {
		return [
			ArmPack.decode,
			ArmPack.encode,
			UIBox.showMessage
		];
	}
}

///if is_lab
// @:keep
class KeepLab {
	static keep = () => {
		let a = Base.uiBox.panel;
		return [a];
	}
}
///end

// @:expose("core")
class CoreBridge {
	// static Image = Image;
	// static System = System;
	static colorFromFloats = (r: f32, g: f32, b: f32, a: f32): Color => {
		return color_from_floats(r, g, b, a);
	}
}

// @:expose("iron")
// class IronBridge {
// 	static App = App;
// 	static Scene = Scene;
// 	static RenderPath = RenderPath;
// 	static Time = Time;
// 	static Input = Input;
// 	static ArmPack = ArmPack;
// 	static BaseObject = BaseObject;
// 	static Data = Data;
// }

// @:expose("zui")
// class ZuiBridge {
// 	static Handle = Handle;
// 	static Zui = Zui;
// }

// @:expose("console")
class ConsoleBridge {
	static log = Console.log;
	static error = Console.error;
}

// @:expose("arm")
// class ArmBridge {
// 	static Base = Base;
// 	static Config = Config;
// 	static Context = Context;
// 	static History = History;
// 	static Operator = Operator;
// 	static Plugin = Plugin;
// 	static Project = Project;
// 	static Res = Res;
// 	static Path = Path;
// 	static File = File;
// 	static NodesBrush = NodesBrush;
// 	static ParserLogic = ParserLogic;
// 	static UIBase = UIBase;
// 	static UINodes = UINodes;
// 	static UIFiles = UIFiles;
// 	static UIMenu = UIMenu;
// 	static UIBox = UIBox;
// 	static UtilMesh = UtilMesh;
// 	static Viewport = Viewport;
// 	///if (krom_direct3d12 || krom_vulkan || krom_metal)
// 	static RenderPathRaytrace = RenderPathRaytrace;
// 	///end
// }
