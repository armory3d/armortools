
#include "global.h"

void import_texture_run_on_next_frame(import_texture_data_t *itd) {
	import_envmap_run(itd->path, itd->image);
}

void import_texture_run(char *path, bool hdr_as_envmap) {
	if (!path_is_texture(path)) {
		if (!context_enable_import_plugin(path)) {
			console_error(strings_unknown_asset_format());
			return;
		}
	}

	for (i32 i = 0; i < project_assets->length; ++i) {
		asset_t *a = project_assets->buffer[i];
		// Already imported
		if (string_equals(a->file, path)) {
			// Set as envmap
			if (hdr_as_envmap && ends_with(to_lower_case(path), ".hdr")) {
				gpu_texture_t         *image = data_get_image(path);
				import_texture_data_t *itd   = GC_ALLOC_INIT(import_texture_data_t, {.path = path, .image = image});
				sys_notify_on_next_frame(&import_texture_run_on_next_frame, itd); // Make sure file browser process did finish
			}
			console_info(strings_asset_already_imported());
			return;
		}
	}

	char          *ext      = substring(path, string_last_index_of(path, ".") + 1, string_length(path));
	void          *importer = any_map_get(import_texture_importers, ext);    // JSValue -> (s: string)=>gpu_texture_t
	bool           cached   = any_map_get(data_cached_images, path) != NULL; // Already loaded or pink texture for missing file
	gpu_texture_t *image;
	if (importer == NULL || cached) {
		image = import_texture_default_importer(path);
	}
	else {
		image = js_pcall_str(importer, path);
	}

	if (image == NULL) {
		return;
	}

	any_map_set(data_cached_images, path, image);
	string_t_array_t *ar    = string_split(path, PATH_SEP);
	char             *name  = ar->buffer[ar->length - 1];
	asset_t          *asset = GC_ALLOC_INIT(asset_t, {.name = name, .file = path, .id = project_asset_id++});
	any_array_push(project_assets, asset);
	if (context_raw->texture == NULL) {
		context_raw->texture = asset;
	}
	any_array_push(project_asset_names, name);
	any_imap_set(project_asset_map, asset->id, image);
	ui_base_hwnds->buffer[TAB_AREA_STATUS]->redraws = 2;
	console_info(string("%s %s", tr("Texture imported:"), name));

	// Set as envmap
	if (hdr_as_envmap && ends_with(to_lower_case(path), ".hdr")) {
		import_texture_data_t *itd = GC_ALLOC_INIT(import_texture_data_t, {.path = path, .image = image});
		sys_notify_on_next_frame(&import_texture_run_on_next_frame, itd); // Make sure file browser process did finish
	}
}

gpu_texture_t *import_texture_default_importer(char *path) {
	return data_get_image(path);
}
