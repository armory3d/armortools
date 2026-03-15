
#include "global.h"

void import_theme_run(char *path) {
	if (!path_is_json(path)) {
		console_error(strings_unknown_asset_format());
		return;
	}

	char *filename = substring(path, string_last_index_of(path, PATH_SEP) + 1, string_length(path));
	char *dst_path = string("%s%sthemes%s%s", path_data(), PATH_SEP, PATH_SEP, filename);
	file_copy(path, dst_path);      // Copy to preset folder
	box_preferences_fetch_themes(); // Refresh file list
	config_raw->theme          = string_copy(filename);
	box_preferences_h_theme->i = box_preferences_get_theme_index();
	config_load_theme(config_raw->theme, true);
	console_info(string("%s %s", tr("Theme imported:"), filename));
}
