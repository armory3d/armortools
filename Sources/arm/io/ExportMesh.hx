package arm.io;

import arm.ui.UITrait;

class ExportMesh {

	public static function run(path: String, applyDisplacement = false) {
		if (UITrait.inst.exportMeshFormat == FormatObj) ExportObj.run(path, applyDisplacement);
		else ExportArm.run(path);
	}
}
