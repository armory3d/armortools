package arm.ui;

import zui.Id;

class TabWorld {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "World")) {
			// ui.check(Id.handle({selected: true}), "Sun");
			// ui.check(Id.handle({selected: true}), "Clouds");
			Project.waterPass = ui.check(Id.handle({selected: Project.waterPass}), "Water");
		}
	}
}
