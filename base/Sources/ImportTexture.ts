
class ImportTexture {

	static run = (path: string, hdrAsEnvmap: bool = true) => {
		if (!Path.is_texture(path)) {
			if (!Context.enable_import_plugin(path)) {
				Console.error(Strings.error1());
				return;
			}
		}

		for (let a of Project.assets) {
			// Already imported
			if (a.file == path) {
				// Set as envmap
				if (hdrAsEnvmap && path.toLowerCase().endsWith(".hdr")) {
					let image: image_t = data_get_image(path);
					Base.notify_on_next_frame(() => { // Make sure file browser process did finish
						ImportEnvmap.run(path, image);
					});
				}
				Console.info(Strings.info0());
				return;
			}
		}

		let ext: string = path.substr(path.lastIndexOf(".") + 1);
		let importer: (s: string, f: (img: image_t)=>void)=>void = Path.texture_importers.get(ext);
		let cached: bool = data_cached_images.get(path) != null; // Already loaded or pink texture for missing file
		if (importer == null || cached) importer = ImportTexture.default_importer;

		importer(path, (image: image_t) => {
			data_cached_images.set(path, image);
			let ar: string[] = path.split(Path.sep);
			let name: string = ar[ar.length - 1];
			let asset: asset_t = {name: name, file: path, id: Project.asset_id++};
			Project.assets.push(asset);
			if (Context.raw.texture == null) Context.raw.texture = asset;
			Project.asset_names.push(name);
			Project.asset_map.set(asset.id, image);
			UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
			Console.info(tr("Texture imported:") + " " + name);

			// Set as envmap
			if (hdrAsEnvmap && path.toLowerCase().endsWith(".hdr")) {
				Base.notify_on_next_frame(() => { // Make sure file browser process did finish
					ImportEnvmap.run(path, image);
				});
			}
		});
	}

	static default_importer = (path: string, done: (img: image_t)=>void) => {
		let img: image_t = data_get_image(path);
		done(img);
	}
}
