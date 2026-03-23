#include "global.h"

void *plugin;

void *import_txt(char *path) {
	void *b = data_get_blob(path);

	string_array_t *a = string_split(path, "\\\\");
	char *s = array_pop(a);
	a = string_split(s, "/");
	s = array_pop(a);
	char *filename = s;

	ui_box_show_message(filename, sys_buffer_to_string(b), true);
	data_delete_blob(path);
	return NULL;
}

void on_delete() {
	plugin_unregister_texture("txt", import_txt);
}

void main() {
	plugin = plugin_create();
	gc_root(plugin);
	plugin_notify_on_delete(plugin, on_delete);
	plugin_register_texture("txt", import_txt);
}
