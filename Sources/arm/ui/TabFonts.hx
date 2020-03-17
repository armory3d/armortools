package arm.ui;

import zui.Zui;
import zui.Id;
import arm.io.ImportFont;
import arm.App.tr;

class TabFonts {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, tr("Fonts"))) {
			ui.row([1 / 4]);

			if (ui.button(tr("Import"))) Project.importAsset("ttf");
			if (ui.isHovered) ui.tooltip(tr("Import font file") + ' (${Config.keymap.file_import_assets})');

			for (f in ImportFont.fontList) {
				ui.text(f);
			}
		}
	}
}
