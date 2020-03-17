package arm.ui;

import arm.App.tr;

class TabParticles {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab1, tr("Particles"))) {
			ui.row([1 / 4, 1 / 4, 1 / 4]);
			if (ui.button(tr("New"))) {}
			if (ui.button(tr("Import"))) {}
			if (ui.button(tr("Nodes"))) {}
		}
	}
}
