package arm.io;

import iron.data.SceneFormat;
import arm.ui.UITrait;

class ExportArm {

	public static function run(path:String) {
		var raw:TSceneFormat = { mesh_datas: [ UITrait.inst.paintObject.data.raw ] };
		var b = iron.system.ArmPack.encode(raw);
		if (!StringTools.endsWith(path, ".arm")) path += ".arm";
		#if kha_krom
		Krom.fileSaveBytes(path, b.getData());
		#end
	}
}
