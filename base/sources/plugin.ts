
type plugin_t = {
	on_ui?: any; // JSValue *
	on_draw?: any; // JSValue *
	on_update?: any; // JSValue *
	on_delete?: any; // JSValue *
	version?: string;
	name?: string;
};

let plugin_map: map_t<string, plugin_t> = map_create();
let _plugin_name: string;

function plugin_create(): plugin_t {
	let p: plugin_t = {};
	p.name = _plugin_name;
	map_set(plugin_map, p.name, p);
	return p;
}

function plugin_start(plugin: string) {
	let blob: buffer_t = data_get_blob("plugins/" + plugin);
	_plugin_name = plugin;
	js_eval(sys_buffer_to_string(blob));
	data_delete_blob("plugins/" + plugin);
}

function plugin_stop(plugin: string) {
	let p: plugin_t = map_get(plugin_map, plugin);
	if (p.on_delete != null) {
		js_call(p.on_delete);
	}
	map_delete(plugin_map, plugin);
}

function plugin_notify_on_ui(plugin: plugin_t, f: any): void {
	plugin.on_ui = f;
}

function plugin_notify_on_update(plugin: plugin_t, f: any): void {
	plugin.on_update = f;
}

function plugin_notify_on_delete(plugin: plugin_t, f: any): void {
	plugin.on_delete = f;
}
