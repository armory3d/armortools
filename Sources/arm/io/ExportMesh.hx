package arm.io;

import arm.ui.UITrait;

class ExportMesh {

	public static function run(path: String) {
		if (UITrait.inst.exportMeshFormat == FormatObj) ExportObj.run(path);
		else ExportArm.run(path);
	}
}
