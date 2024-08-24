
type import_texture_data_t = {
	path: string;
	image: image_t;
};

function import_texture_run(path: string, hdr_as_envmap: bool = true) {
	if (!path_is_texture(path)) {
		if (!context_enable_import_plugin(path)) {
			console_error(strings_error1());
			return;
		}
	}

	for (let i: i32 = 0; i < project_assets.length; ++i) {
		let a: asset_t = project_assets[i];
		// Already imported
		if (a.file == path) {
			// Set as envmap
			if (hdr_as_envmap && ends_with(to_lower_case(path), ".hdr")) {
				let image: image_t = data_get_image(path);
				let itd: import_texture_data_t = { path: path, image: image };
				app_notify_on_next_frame(function (itd: import_texture_data_t) { // Make sure file browser process did finish
					import_envmap_run(itd.path, itd.image);
				}, itd);
			}
			console_info(strings_info0());
			return;
		}
	}

	let ext: string = substring(path, string_last_index_of(path, ".") + 1, path.length);
	let importer: any = map_get(path_texture_importers, ext); // JSValue -> (s: string)=>image_t
	let cached: bool = map_get(data_cached_images, path) != null; // Already loaded or pink texture for missing file
	let image: image_t;
	if (importer == null || cached) {
		image = import_texture_default_importer(path);
	}
	else {
		image = js_pcall_str(importer, path);
	}

	map_set(data_cached_images, path, image);
	let ar: string[] = string_split(path, path_sep);
	let name: string = ar[ar.length - 1];
	let asset: asset_t = {name: name, file: path, id: project_asset_id++};
	array_push(project_assets, asset);
	if (context_raw.texture == null) {
		context_raw.texture = asset;
	}
	array_push(project_asset_names, name);
	map_set(project_asset_map, asset.id, image);
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
	console_info(tr("Texture imported:") + " " + name);

	// Set as envmap
	if (hdr_as_envmap && ends_with(to_lower_case(path), ".hdr")) {
		let itd: import_texture_data_t = { path: path, image: image };
		app_notify_on_next_frame(function (itd: import_texture_data_t) { // Make sure file browser process did finish
			import_envmap_run(itd.path, itd.image);
		}, itd);
	}
}

function import_texture_default_importer(path: string): image_t {
	return data_get_image(path);
}
