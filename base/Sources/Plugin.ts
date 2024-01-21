
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

class CoreBridge {
	static colorFromFloats = (r: f32, g: f32, b: f32, a: f32): Color => {
		return color_from_floats(r, g, b, a);
	}
}

class ConsoleBridge {
	static log = Console.log;
	static error = Console.error;
}
