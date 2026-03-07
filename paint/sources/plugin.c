plugin_t *plugin_create() {
	plugin_t *p = GC_ALLOC_INIT(plugin_t, {0});
	p->name     = string_copy(_plugin_name);
	any_map_set(plugin_map, p->name, p);
	return p;
}

void plugin_start(char *plugin) {
	buffer_t *blob = data_get_blob(string_join("plugins/", plugin));
	gc_unroot(_plugin_name);
	_plugin_name = string_copy(plugin);
	gc_root(_plugin_name);
	js_eval(string_join(string_join("(1, eval)(`", sys_buffer_to_string(blob)), "`)"));
	data_delete_blob(string_join("plugins/", plugin));
}

void plugin_stop(char *plugin) {
	plugin_t *p = any_map_get(plugin_map, plugin);
	if (p->on_delete != NULL) {
		js_call(p->on_delete);
	}
	map_delete(plugin_map, plugin);
}

void plugin_notify_on_ui(plugin_t *plugin, any f) {
	plugin->on_ui = f;
}

void plugin_notify_on_update(plugin_t *plugin, any f) {
	plugin->on_update = f;
}

void plugin_notify_on_delete(plugin_t *plugin, any f) {
	plugin->on_delete = f;
}
