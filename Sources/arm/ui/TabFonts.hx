package arm.ui;

import zui.Zui;
import zui.Id;
import arm.io.ImportFont;

class TabFonts {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab2, tr("Fonts"))) {
			ui.row([1 / 4]);

			if (ui.button(tr("Import"))) Project.importAsset("ttf");
			if (ui.isHovered) ui.tooltip(tr("Import font file") + ' (${Config.keymap.file_import_assets})');

			for (f in ImportFont.fontList) {
				ui.text(f);
			}
		}
	}
}
