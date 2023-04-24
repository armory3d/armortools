package arm.ui;

import zui.Zui;

class TabPlugins {

	public static function draw(htab: Handle) {
		var ui = UIBase.inst.ui;
		if (ui.tab(htab, tr("Plugins"))) {

			ui.beginSticky();

			#if (is_paint || is_sculpt)
			ui.row([1 / 4]);
			#end
			#if is_lab
			ui.row([1 / 14]);
			#end

			if (ui.button(tr("Manager"))) {
				BoxPreferences.htab.position = 6; // Plugins
				BoxPreferences.show();
			}
			ui.endSticky();

			// Draw plugins
			for (p in Plugin.plugins) if (p.drawUI != null) p.drawUI(ui);
		}
	}
}
