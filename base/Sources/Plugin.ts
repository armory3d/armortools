
class PluginRaw {
	drawUI: (ui: zui_t)=>void = null;
	draw: ()=>void = null;
	update: ()=>void = null;
	delete: ()=>void = null;
	version = "0.1";
	apiversion = "0.1";
	name: string;
}

class Plugin {

	static plugins: Map<string, PluginRaw> = new Map();
	static pluginName: string;

	static create(): PluginRaw {
		let p = new PluginRaw();
		p.name = Plugin.pluginName;
		Plugin.plugins.set(p.name, p);
		return p;
	}

	static start = (plugin: string) => {
		try {
			data_get_blob("plugins/" + plugin, (blob: ArrayBuffer) => {
				Plugin.pluginName = plugin;
				// (1, eval)(sys_buffer_to_string(blob)); // Global scope
				eval(sys_buffer_to_string(blob)); // Local scope
				data_delete_blob("plugins/" + plugin);
			});
		}
		catch (e: any) {
			Console.error(tr("Failed to load plugin") + " '" + plugin + "'");
			krom_log(e);
		}
	}

	static stop = (plugin: string) => {
		let p = Plugin.plugins.get(plugin);
		if (p != null && p.delete != null) p.delete();
		Plugin.plugins.delete(plugin);
	}
}
