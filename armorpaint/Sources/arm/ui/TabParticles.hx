package arm.ui;

class TabParticles {

	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab1, tr("Particles"))) {
			ui.beginSticky();
			ui.row([1 / 4, 1 / 4, 1 / 4]);
			if (ui.button(tr("New"))) {}
			if (ui.button(tr("Import"))) {}
			if (ui.button(tr("Nodes"))) {}
			ui.endSticky();
		}
	}
}
