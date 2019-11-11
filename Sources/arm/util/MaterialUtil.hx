package arm.util;

import iron.data.Data;

class MaterialUtil {

	public static function removeMaterialCache() {
		Data.cachedMaterials.remove("SceneMaterial2");
		Data.cachedShaders.remove("Material2_data");
		Data.cachedSceneRaws.remove("Material2_data");
		// Data.cachedBlobs.remove("Material2_data.arm");
	}
}
