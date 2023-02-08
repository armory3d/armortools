package arm.io;

import kha.Image;
import iron.data.Data;
import arm.ui.UIStatus;
import arm.sys.Path;
import arm.ProjectFormat;

class ImportTexture {

	public static function run(path: String, hdrAsEnvmap = true) {
		if (!Path.isTexture(path)) {
			if (!Context.enableImportPlugin(path)) {
				Console.error(Strings.error1());
				return;
			}
		}

		for (a in Project.assets) {
			// Already imported
			if (a.file == path) {
				// Set as envmap
				if (hdrAsEnvmap && path.toLowerCase().endsWith(".hdr")) {
					Data.getImage(path, function(image: kha.Image) {
						App.notifyOnNextFrame(function() { // Make sure file browser process did finish
							ImportEnvmap.run(path, image);
						});
					});
				}
				Console.info(Strings.info0());
				return;
			}
		}

		var ext = path.substr(path.lastIndexOf(".") + 1);
		var importer = Path.textureImporters.get(ext);
		var cached = Data.cachedImages.get(path) != null; // Already loaded or pink texture for missing file
		if (importer == null || cached) importer = defaultImporter;

		importer(path, function(image: Image) {
			Data.cachedImages.set(path, image);
			var ar = path.split(Path.sep);
			var name = ar[ar.length - 1];
			var asset: TAsset = {name: name, file: path, id: Project.assetId++};
			Project.assets.push(asset);
			if (Context.texture == null) Context.texture = asset;
			Project.assetNames.push(name);
			Project.assetMap.set(asset.id, image);
			UIStatus.inst.statusHandle.redraws = 2;
			Console.info(tr("Texture imported:") + " " + name);

			// Set as envmap
			if (hdrAsEnvmap && path.toLowerCase().endsWith(".hdr")) {
				App.notifyOnNextFrame(function() { // Make sure file browser process did finish
					ImportEnvmap.run(path, image);
				});
			}
		});
	}

	static function defaultImporter(path: String, done: Image->Void) {
		Data.getImage(path, done);
	}
}
