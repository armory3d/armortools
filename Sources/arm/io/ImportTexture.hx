package arm.io;

import kha.Image;
import iron.data.Data;
import arm.ui.UISidebar;
import arm.sys.Path;
import arm.ProjectFormat;

class ImportTexture {

	public static function run(path: String) {
		if (!Path.isTexture(path)) {
			Log.error(Strings.error1());
			return;
		}

		for (a in Project.assets) {
			if (a.file == path) {
				// Set envmap
				if (path.toLowerCase().endsWith(".hdr")) {
					Data.getImage(path, function(image: kha.Image) {
						App.notifyOnNextFrame(function() { // Make sure file browser process did finish
							ImportEnvmap.run(path, image);
						});
					});
				}
				Log.info(Strings.info0());
				return;
			}
		}

		var ext = path.substr(path.lastIndexOf(".") + 1);
		var importer = Path.textureImporters.get(ext);
		if (importer == null) importer = defaultImporter;

		importer(path, function(image: Image) {
			Data.cachedImages.set(path, image);
			var ar = path.split(Path.sep);
			var name = ar[ar.length - 1];
			var asset: TAsset = {name: name, file: path, id: Project.assetId++};
			Project.assets.push(asset);
			if (Context.texture == null) Context.texture = asset;
			Project.assetNames.push(name);
			Project.assetMap.set(asset.id, image);
			UISidebar.inst.hwnd2.redraws = 2;

			// Set envmap
			if (path.toLowerCase().endsWith(".hdr")) {
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
