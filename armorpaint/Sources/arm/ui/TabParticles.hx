package arm.ui;

import zui.Zui;

class TabParticles {

	public static function draw(htab: Handle) {
		var ui = UIBase.inst.ui;
		if (ui.tab(htab, tr("Particles"))) {
			ui.beginSticky();
			ui.row([1 / 4, 1 / 4, 1 / 4]);
			if (ui.button(tr("New"))) {}
			if (ui.button(tr("Import"))) {}
			if (ui.button(tr("Nodes"))) {}
			ui.endSticky();
		}
	}
}
