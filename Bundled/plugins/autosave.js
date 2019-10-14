
let plugin = new arm.Plugin();

let h1 = new zui.Handle();
let h2 = new zui.Handle({value: 5});
let timer = 0.0;

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
