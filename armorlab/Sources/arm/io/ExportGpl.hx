package arm.io;

import haxe.io.BytesOutput;
import arm.ProjectFormat;

class ExportGpl {

	public static function run(path: String, name: String, swatches: Array<TSwatchColor>) {
		var o = new BytesOutput();
		o.bigEndian = false;
		o.writeString("GIMP Palette\n");
		o.writeString("Name: " + name + "\n");
		o.writeString("# armorpaint.org\n");
		o.writeString("#\n");

		for (swatch in swatches) {
			o.writeString(Std.string(swatch.base.Rb) + " " + Std.string(swatch.base.Gb) + " " + Std.string(swatch.base.Bb) + "\n");
		}

		Krom.fileSaveBytes(path, o.getBytes().getData(), o.getBytes().length);
	}
}
