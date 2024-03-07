
class PluginRaw {
	draw_ui: (ui: zui_t)=>void = null;
	draw: ()=>void = null;
	update: ()=>void = null;
	delete: ()=>void = null;
	version: string = "0.1";
	apiversion: string = "0.1";
	name: string;
}

class Plugin {

	static plugins: Map<string, PluginRaw> = new Map();
	static plugin_name: string;

	static create(): PluginRaw {
		let p: PluginRaw = new PluginRaw();
		p.name = Plugin.plugin_name;
		Plugin.plugins.set(p.name, p);
		return p;
	}

	static start = (plugin: string) => {
		try {
			let blob: ArrayBuffer = data_get_blob("plugins/" + plugin);
			Plugin.plugin_name = plugin;
			// (1, eval)(sys_buffer_to_string(blob)); // Global scope
			eval(sys_buffer_to_string(blob)); // Local scope
			data_delete_blob("plugins/" + plugin);
		}
		catch (e: any) {
			Console.error(tr("Failed to load plugin") + " '" + plugin + "'");
			krom_log(e);
		}
	}

	static stop = (plugin: string) => {
		let p: PluginRaw = Plugin.plugins.get(plugin);
		if (p != null && p.delete != null) p.delete();
		Plugin.plugins.delete(plugin);
	}
}
