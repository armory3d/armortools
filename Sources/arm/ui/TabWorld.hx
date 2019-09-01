package arm.ui;

import zui.Id;

class TabWorld {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "World")) {
			// ui.check(Id.handle({selected: true}), "Sun");
			// ui.check(Id.handle({selected: true}), "Clouds");
			Project.waterPass = ui.check(Id.handle({selected: Project.waterPass}), "Water");

			// var world = iron.Scene.active.world;
			// var light = iron.Scene.active.lights[0];
			// // Sync sun direction
			// var v = light.look();
			// world.raw.sun_direction[0] = v.x;
			// world.raw.sun_direction[1] = v.y;
			// world.raw.sun_direction[2] = v.z;
		}
	}
}
