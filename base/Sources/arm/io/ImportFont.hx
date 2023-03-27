package arm.io;

import kha.Font;
import iron.data.Data;
import arm.sys.Path;
import arm.ui.UIBase;
import arm.data.FontSlot;
import arm.util.RenderUtil;

class ImportFont {

	public static function run(path: String) {
		for (f in Project.fonts) {
			if (f.file == path) {
				Console.info(Strings.info0());
				return;
			}
		}
		Data.getFont(path, function(font: Font) {
			var count = Krom.g2_font_count(font.font_);
			var fontSlots = new Array<FontSlot>();
			for (i in 0...count) {
				var ar = path.split(Path.sep);
				var name = ar[ar.length - 1];
				var f = font.clone();
				f.setFontIndex(i);
				var fontSlot = new FontSlot(name, f, path);
				fontSlots.push(fontSlot);
			}

			function _init() {
				for (f in fontSlots) {
					Context.raw.font = f;
					Project.fonts.push(f);
					RenderUtil.makeFontPreview();
				}
			}
			iron.App.notifyOnInit(_init);

			UIBase.inst.hwnds[TabStatus].redraws = 2;
		});
	}
}
