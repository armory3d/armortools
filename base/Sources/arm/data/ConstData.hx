package arm.data;

import kha.Image.TextureFormat;
import kha.Image;
import iron.data.Data;
import iron.system.ArmPack;

class ConstData {
	public static var ltcMatTex: Image = null;
	public static var ltcMagTex: Image = null;
	public static function initLTC() {
		// Real-Time Polygonal-Light Shading with Linearly Transformed Cosines
		// https://eheitzresearch.wordpress.com/415-2/
		Data.getBlob("ltc_mat.arm", function(ltc_mat: js.lib.ArrayBuffer) {
			Data.getBlob("ltc_mag.arm", function(ltc_mag: js.lib.ArrayBuffer) {
				ltcMatTex = Image.fromBytes(ArmPack.decode(ltc_mat), 64, 64, TextureFormat.RGBA128);
				ltcMagTex = Image.fromBytes(ArmPack.decode(ltc_mag), 64, 64, TextureFormat.R32);
			});
		});
	}
}
