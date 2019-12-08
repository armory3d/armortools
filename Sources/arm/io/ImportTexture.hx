package arm.io;

import kha.Image;
import iron.data.Data;
import arm.ui.UITrait;
import arm.sys.Path;
import arm.Project;
using StringTools;

class ImportTexture {

	public static function run(path: String) {
		if (!Path.isTexture(path)) {
			Log.error(Strings.error1);
			return;
		}

		for (a in Project.assets) if (a.file == path) { Log.info(Strings.info0); return; }

		var ext = path.substr(path.lastIndexOf(".") + 1);
		var importer = Path.textureImporters.get(ext);
		if (importer == null) importer = defaultImporter;

		importer(path, function(image: Image) {
			Data.cachedImages.set(path, image);
			var ar = path.split("/");
			ar = ar[ar.length - 1].split("\\");
			var name = ar[ar.length - 1];
			var asset: TAsset = {name: name, file: path, id: Project.assetId++};
			Project.assets.push(asset);
			if (Context.texture == null) Context.texture = asset;
			Project.assetNames.push(name);
			Project.assetMap.set(asset.id, image);
			UITrait.inst.hwnd2.redraws = 2;

			// Set envmap
			if (path.toLowerCase().endsWith(".hdr") &&
				(image.width == 1024 || image.width == 2048 || image.width == 4096)) {
				ImportEnvmap.run(path, image);
			}
		});
	}

	static function defaultImporter(path: String, done: Image->Void) {
		Data.getImage(path, done);
	}
}
