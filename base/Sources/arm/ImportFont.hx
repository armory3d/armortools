package arm;

import iron.App;
import iron.System;
import iron.Data;

class ImportFont {

	public static function run(path: String) {
		for (f in Project.fonts) {
			if (f.file == path) {
				Console.info(Strings.info0());
				return;
			}
		}
		Data.getFont(path, function(font: Font) {
			font.init(); // Make sure font_ is ready
			var count = Krom.g2_font_count(font.font_);
			var fontSlots = new Array<SlotFont>();
			for (i in 0...count) {
				var ar = path.split(Path.sep);
				var name = ar[ar.length - 1];
				var f = font.clone();
				f.setFontIndex(i);
				var fontSlot = new SlotFont(name, f, path);
				fontSlots.push(fontSlot);
			}

			function _init() {
				for (f in fontSlots) {
					Context.raw.font = f;
					Project.fonts.push(f);
					UtilRender.makeFontPreview();
				}
			}
			App.notifyOnInit(_init);

			UIBase.inst.hwnds[TabStatus].redraws = 2;
		});
	}
}
