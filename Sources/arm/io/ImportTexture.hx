package arm.io;

import kha.Image;
import zui.Canvas;
import iron.data.Data;
import arm.ui.UITrait;
import arm.util.Path;
using StringTools;

class ImportTexture {

	public static function run(path:String) {
		if (!Path.checkTextureFormat(path)) {
			UITrait.inst.showError(Strings.error1);
			return;
		}

		for (a in UITrait.inst.assets) if (a.file == path) { UITrait.inst.showMessage(Strings.info0); return; }
		
		Data.getImage(path, function(image:Image) {
			var ar = path.split("/");
			ar = ar[ar.length - 1].split("\\");
			var name = ar[ar.length - 1];
			var asset:TAsset = {name: name, file: path, id: UITrait.inst.assetId++};
			UITrait.inst.assets.push(asset);
			if (UITrait.inst.selectedTexture == null) UITrait.inst.selectedTexture = asset;
			UITrait.inst.assetNames.push(name);
			Canvas.assetMap.set(asset.id, image);
			UITrait.inst.hwnd2.redraws = 2;

			// Set envmap
			if (path.toLowerCase().endsWith(".hdr") &&
				(image.width == 1024 || image.width == 2048 || image.width == 4096)) {
				ImportEnvmap.run(path, image);
			}
		});
	}
}
