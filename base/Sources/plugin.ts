
class PluginRaw {
	draw_ui: (ui: zui_t)=>void = null;
	draw: ()=>void = null;
	update: ()=>void = null;
	delete: ()=>void = null;
	version: string = "0.1";
	name: string;
}

let plugin_map: Map<string, PluginRaw> = new Map();
let _plugin_name: string;

function plugin_create(): PluginRaw {
	let p: PluginRaw = new PluginRaw();
	p.name = _plugin_name;
	plugin_map.set(p.name, p);
	return p;
}

function plugin_start(plugin: string) {
	try {
		let blob: ArrayBuffer = data_get_blob("plugins/" + plugin);
		_plugin_name = plugin;
		// (1, eval)(sys_buffer_to_string(blob)); // Global scope
		eval(sys_buffer_to_string(blob)); // Local scope
		data_delete_blob("plugins/" + plugin);
	}
	catch (e: any) {
		console_error(tr("Failed to load plugin") + " '" + plugin + "'");
		krom_log(e);
	}
}

function plugin_stop(plugin: string) {
	let p: PluginRaw = plugin_map.get(plugin);
	if (p != null && p.delete != null) {
		p.delete();
	}
	plugin_map.delete(plugin);
}
