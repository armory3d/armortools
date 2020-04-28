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
			var ar = path.split(Path.sep);
			var name = ar[ar.length - 1];
			Context.font = new FontSlot(name, font);
			Project.fonts.push(Context.font);

			function makeFontPreview(_) {
				RenderUtil.makeFontPreview();
				iron.App.removeRender(makeFontPreview);
			}
			iron.App.notifyOnRender(makeFontPreview);

			UISidebar.inst.hwnd2.redraws = 2;
		});
	}
}
