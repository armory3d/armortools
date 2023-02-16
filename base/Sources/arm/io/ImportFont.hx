package arm.io;

import kha.Font;
import iron.data.Data;
import arm.sys.Path;
import arm.ui.UIStatus;
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
			var fn = font.getFontNames();
			var fontSlots = new Array<FontSlot>();
			for (i in 0...fn.length) {
				var ar = path.split(Path.sep);
				var name = fn[i] != null ? fn[i] : ar[ar.length - 1];
				var f = font.clone();
				f.setFontIndex(i);
				var fontSlot = new FontSlot(name, f, path);
				fontSlots.push(fontSlot);
			}

			function _init() {
				for (f in fontSlots) {
					Context.font = f;
					Project.fonts.push(f);
					RenderUtil.makeFontPreview();
				}
			}
			iron.App.notifyOnInit(_init);

			UIStatus.inst.statusHandle.redraws = 2;
		});
	}
}
