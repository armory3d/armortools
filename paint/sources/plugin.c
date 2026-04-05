
#include "global.h"

char *_plugin_name;

plugin_t *plugin_create() {
	plugin_t *p = GC_ALLOC_INIT(plugin_t, {0});
	p->name     = string_copy(_plugin_name);
	any_map_set(plugin_map, p->name, p);
	return p;
}

void plugin_start(char *plugin) {
	buffer_t *blob = data_get_blob(string("plugins/%s", plugin));
	gc_unroot(_plugin_name);
	_plugin_name = string_copy(plugin);
	gc_root(_plugin_name);
	minic_ctx_t *ctx = minic_eval_named(sys_buffer_to_string(blob), plugin);
	data_delete_blob(string("plugins/%s", plugin));
	// Store context on the plugin so callbacks can use it and it can be freed on stop
	plugin_t *p = any_map_get(plugin_map, plugin);
	p->ctx      = ctx;
}

void plugin_stop(char *plugin) {
	plugin_t *p = any_map_get(plugin_map, plugin);
	if (p->on_delete != NULL) {
		minic_ctx_call_fn(p->ctx, p->on_delete, NULL, 0);
	}
	minic_ctx_free(p->ctx);
	p->ctx = NULL;
	map_delete(plugin_map, plugin);
}

void plugin_notify_on_ui(plugin_t *plugin, void *f) {
	plugin->on_ui = f;
}

void plugin_notify_on_update(plugin_t *plugin, void *f) {
	plugin->on_update = f;
}

void plugin_notify_on_delete(plugin_t *plugin, void *f) {
	plugin->on_delete = f;
}
