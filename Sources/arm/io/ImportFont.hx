package arm.io;

import kha.Font;
import iron.data.Data;
import arm.sys.Path;
import arm.ui.UISidebar;
import arm.data.FontSlot;
import arm.util.RenderUtil;

class ImportFont {

	public static function run(path: String) {
		Data.getFont(path, function(font: Font) {
			var fn = font.getFontNames();
			var fontSlots = new Array<FontSlot>();
			for (i in 0...fn.length) {
				var ar = path.split(Path.sep);
				var name = fn[i] != null ? fn[i] : ar[ar.length - 1];
				var f = font.clone();
				f.setFontIndex(i);
				var fontSlot = new FontSlot(name, f);
				fontSlots.push(fontSlot);
			}
			font.unload();

			function makeFontPreview(_) {
				for (f in fontSlots) {
					Context.font = f;
					Project.fonts.push(f);
					RenderUtil.makeFontPreview();
				}
				iron.App.removeRender(makeFontPreview);
			}
			iron.App.notifyOnRender(makeFontPreview);

			UISidebar.inst.hwnd2.redraws = 2;
		});
	}
}
