package arm;

import iron.System;
import arm.ProjectFormat;

class ExportGpl {

	public static function run(path: String, name: String, swatches: Array<TSwatchColor>) {
		var o = "";
		o += "GIMP Palette\n";
		o += "Name: " + name + "\n";
		o += "# armorpaint.org\n";
		o += "#\n";

		for (swatch in swatches) {
			o += Std.string(swatch.base.Rb) + " " + Std.string(swatch.base.Gb) + " " + Std.string(swatch.base.Bb) + "\n";
		}

		Krom.fileSaveBytes(path, System.stringToBuffer(o), o.length);
	}
}
