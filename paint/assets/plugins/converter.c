#include "global.h"

void *plugin;
ui_handle_t *h1;

void on_arm_to_json(char *path) {
	buffer_t *b = data_get_blob(path);
	any_map_t *parsed = armpack_decode_to_map(b);

	json_encode_begin();
	json_encode_map(parsed);
	char *s = json_encode_end();

	buffer_t *out = sys_string_to_buffer(s);
	char *p = substring(path, 0, string_length(path) - 3);
	p = string("%sjson", p);
	iron_file_save_bytes(p, out, 0);
}

void on_json_to_arm(char *path) {
	buffer_t *b = data_get_blob(path);

	// void *parsed = json_parse(sys_buffer_to_string(b));
	// let out = armpack_encode(parsed);

	char *p = substring(path, 0, string_length(path) - 4);
	p = string("%sarm", p);
	// iron_file_save_bytes(p, out, 0);
}

void on_ui() {
	if (ui_panel(h1, "Converter", false, false, false)) {
		ui_row2();
		if (ui_button(".arm to .json", UI_ALIGN_CENTER, "")) {
			ui_files_show2("arm", false, true, on_arm_to_json);
		}
		if (ui_button(".json to .arm", UI_ALIGN_CENTER, "")) {
			ui_files_show2("json", false, true, on_json_to_arm);
		}
	}
};

void main() {
	plugin = plugin_create();
	h1 = ui_handle_create();
	gc_root(plugin);
	gc_root(h1);

	plugin_notify_on_ui(plugin, on_ui);
}
