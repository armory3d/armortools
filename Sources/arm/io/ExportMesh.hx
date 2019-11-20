package arm.io;

import arm.ui.UITrait;

class ExportMesh {

	public static function run(path:String) {
		if (UITrait.inst.exportMeshFormat == 0) ExportObj.run(path);
		else ExportArm.run(path);
	}
}
