package arm.ui;

import zui.Id;
import zui.Ext;

class TabBrowser {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "Browser")) {
			var h = Id.handle();
			h.text = ui.textInput(h, "Path");
			Ext.dataPath = iron.data.Data.dataPath;
			Ext.fileBrowser(ui, h);
		}
	}
}
