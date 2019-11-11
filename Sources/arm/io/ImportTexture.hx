package arm.io;

import kha.Image;
import iron.data.Data;
import arm.ui.UITrait;
import arm.util.Path;
import arm.Project;
using StringTools;

class ImportTexture {

	public static function run(path:String) {
		if (!Path.isTexture(path)) {
			Log.showError(Strings.error1);
			return;
		}

		for (a in Project.assets) if (a.file == path) { Log.showMessage(Strings.info0); return; }

		Data.getImage(path, function(image:Image) {
			var ar = path.split("/");
			ar = ar[ar.length - 1].split("\\");
			var name = ar[ar.length - 1];
			var asset:TAsset = {name: name, file: path, id: Project.assetId++};
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
}
