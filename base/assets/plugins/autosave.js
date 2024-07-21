
let plugin = plugin_create();

let h1 = zui_handle_create();
let h2 = zui_handle_create({value: 5});
let timer = 0.0;

plugin.draw_ui = function(ui) {
	if (zui_panel(h1, "Auto Save")) {
		zui_slider(h2, "min", 1, 15, false, 1);
	}
}

plugin.update = function() {
	if (project_filepath == "") {
		return;
	}
	timer += 1 / 60;
	if (timer >= h2.value * 60) {
		timer = 0.0;
		project_save();
	}
}
