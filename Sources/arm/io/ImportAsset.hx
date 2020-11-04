package arm.io;

import arm.sys.Path;
import arm.ui.UINodes;
import arm.ui.UIBox;
import arm.Project;

class ImportAsset {

	public static function run(path: String, dropX = -1.0, dropY = -1.0, showBox = true) {
		if (Path.isMesh(path)) {
			showBox ? Project.importMeshBox(path) : ImportMesh.run(path);
			if (dropX > 0) UIBox.clickToHide = false; // Prevent closing when going back to window after drag and drop
		}
		else if (Path.isTexture(path)) {
			ImportTexture.run(path);
			// Place image node
			var x0 = UINodes.inst.wx;
			var x1 = UINodes.inst.wx + UINodes.inst.ww;
			if (UINodes.inst.show && dropX > x0 && dropX < x1) {
				var assetIndex = 0;
				for (i in 0...Project.assets.length) {
					if (Project.assets[i].file == path) {
						assetIndex = i;
						break;
					}
				}
				UINodes.inst.acceptAssetDrag(assetIndex);
				UINodes.inst.getNodes().nodesDrag = false;
				UINodes.inst.hwnd.redraws = 2;
			}
		}
		else if (Path.isFont(path)) {
			ImportFont.run(path);
		}
		else if (Path.isProject(path)) {
			ImportArm.runProject(path);
		}
		else if (Path.isPlugin(path)) {
			ImportPlugin.run(path);
		}
		else if (Path.isFolder(path)) {
			ImportFolder.run(path);
		}
		else {
			Log.error(Strings.error1());
		}
	}
}
