
class ImportTexture {

	static run = (path: string, hdrAsEnvmap = true) => {
		if (!Path.isTexture(path)) {
			if (!Context.enableImportPlugin(path)) {
				Console.error(Strings.error1());
				return;
			}
		}

		for (let a of Project.assets) {
			// Already imported
			if (a.file == path) {
				// Set as envmap
				if (hdrAsEnvmap && path.toLowerCase().endsWith(".hdr")) {
					data_get_image(path, (image: image_t) => {
						Base.notifyOnNextFrame(() => { // Make sure file browser process did finish
							ImportEnvmap.run(path, image);
						});
					});
				}
				Console.info(Strings.info0());
				return;
			}
		}

		let ext = path.substr(path.lastIndexOf(".") + 1);
		let importer = Path.textureImporters.get(ext);
		let cached = data_cached_images.get(path) != null; // Already loaded or pink texture for missing file
		if (importer == null || cached) importer = ImportTexture.defaultImporter;

		importer(path, (image: image_t) => {
			data_cached_images.set(path, image);
			let ar = path.split(Path.sep);
			let name = ar[ar.length - 1];
			let asset: TAsset = {name: name, file: path, id: Project.assetId++};
			Project.assets.push(asset);
			if (Context.raw.texture == null) Context.raw.texture = asset;
			Project.assetNames.push(name);
			Project.assetMap.set(asset.id, image);
			UIBase.hwnds[TabArea.TabStatus].redraws = 2;
			Console.info(tr("Texture imported:") + " " + name);

			// Set as envmap
			if (hdrAsEnvmap && path.toLowerCase().endsWith(".hdr")) {
				Base.notifyOnNextFrame(() => { // Make sure file browser process did finish
					ImportEnvmap.run(path, image);
				});
			}
		});
	}

	static defaultImporter = (path: string, done: (img: image_t)=>void) => {
		data_get_image(path, done);
	}
}
