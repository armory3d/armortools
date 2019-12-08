package arm.ui;

class TabParticles {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab1, "Particles")) {
			ui.row([1 / 4, 1 / 4]);
			if (ui.button("New")) {}
			if (ui.button("Nodes")) {}
		}
	}
}
