
type plugin_t = {
	draw_ui?: (ui: zui_t)=>void;
	draw?: ()=>void;
	update?: ()=>void;
	delete?: ()=>void;
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
	if (p != null && p.delete != null) {
		p.delete();
	}
	map_delete(plugin_map, plugin);
}
