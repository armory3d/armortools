package arm.io;

import kha.Font;
import kha.arrays.Int16Array;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.data.Data;
import iron.math.Vec4;
import iron.Scene;
import arm.util.MeshUtil;
import arm.util.UVUtil;
import arm.util.ViewportUtil;
import arm.sys.Path;
import arm.ui.UITrait;
import arm.ui.UIView2D;
import arm.ui.UINodes;
import arm.ui.UIBox;
import arm.Project;
import arm.Tool;
using StringTools;

class ImportAsset {

	public static function run(path:String, dropX = -1.0, dropY = -1.0, showBox = true) {
		// Mesh
		if (Path.isMesh(path)) {
			showBox ? Project.importMeshBox(path) : ImportMesh.run(path);
			if (dropX > 0) UIBox.clickToHide = false; // Prevent closing when going back to window after drag and drop
		}
		// Image
		else if (Path.isTexture(path)) {
			ImportTexture.run(path);
			// Place image node
			var x0 = UINodes.inst.wx;
			var x1 = UINodes.inst.wx + UINodes.inst.ww;
			if (UINodes.inst.show && dropX > x0 && dropX < x1) {
				UINodes.inst.acceptAssetDrag(Project.assets.length - 1);
				UINodes.inst.getNodes().nodesDrag = false;
				UINodes.inst.hwnd.redraws = 2;
			}
		}
		// Font
		else if (Path.isFont(path)) {
			ImportFont.run(path);
		}
		// Project
		else if (Path.isProject(path)) {
			ImportArm.runProject(path);
		}
		// Plugin
		else if (Path.isPlugin(path)) {
			ImportPlugin.run(path);
		}
		// Folder
		else if (Path.isFolder(path)) {
			ImportFolder.run(path);
		}
		else {
			Log.error(Strings.error1);
		}
	}
}
