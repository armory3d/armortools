
var plugin = new arm.Plugin();

var h1 = plugin.handle();
var h2 = plugin.handle({value:1});

var timer = 0.0;

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "Auto Save")) {
		ui.slider(h2, "min", 1, 15, false, 1);
	}
}

plugin.update = function() {
	if (arm.Project.filepath == "") return;
	timer += 1 / 60;
	if (timer >= h2.value * 60) {
		timer = 0.0;
		arm.Project.projectSave();
	}
}
