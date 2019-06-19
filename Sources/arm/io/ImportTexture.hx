package arm.io;

import zui.Canvas;
import arm.ui.UITrait;
import arm.util.Path;

class ImportTexture {

	public static function run(path:String) {
		if (!Path.checkTextureFormat(path)) {
			UITrait.inst.showError(Strings.error1);
			return;
		}

		for (a in UITrait.inst.assets) if (a.file == path) { UITrait.inst.showMessage(Strings.info0); return; }
		
		iron.data.Data.getImage(path, function(image:kha.Image) {
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
			if (StringTools.endsWith(path.toLowerCase(), ".hdr") &&
				(image.width == 1024 || image.width == 2048 || image.width == 4096)) {
				arm.io.ImportEnvmap.run(path, image);
			}
		});
	}
}
