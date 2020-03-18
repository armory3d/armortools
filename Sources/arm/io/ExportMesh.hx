package arm.io;

import arm.ui.UISidebar;

class ExportMesh {

	public static function run(path: String, applyDisplacement = false) {
		if (UISidebar.inst.exportMeshFormat == FormatObj) ExportObj.run(path, applyDisplacement);
		else ExportArm.run(path);
	}
}
