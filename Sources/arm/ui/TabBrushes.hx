package arm.ui;

class TabBrushes {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab1, "Brushes")) {
			ui.row([1/4,1/4]);
			if (ui.button("New")) {}
			if (ui.button("Nodes")) UITrait.inst.showBrushNodes();
		}
	}
}
