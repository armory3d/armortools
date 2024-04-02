
let plugin = plugin_create();

let h1 = zui_handle_create();
let h2 = zui_handle_create();
let h3 = zui_handle_create();
let h4 = zui_handle_create();
let h5 = zui_handle_create();
let h6 = zui_handle_create();
let h7 = zui_handle_create();

plugin.draw_ui = function(ui) {
	if (zui_panel(h1, "My Plugin")) {
		zui_text("Label");
		zui_text_input(h7, "Text Input");
		if (zui_button("Button")) {
			console.error("Hello");
		}
		zui_row([1/2, 1/2]);
		zui_button("Button A");
		zui_button("Button B");
		zui_combo(h5, ["Item 1", "Item 2"], "Combo");
		zui_row([1/2, 1/2]);
		zui_slider(h2, "Slider", 0, 1, true);
		zui_slider(h3, "Slider", 0, 1, true);
		zui_check(h4, "Check");
		zui_radio(h6, 0, "Radio 1");
		zui_radio(h6, 1, "Radio 2");
		zui_radio(h6, 2, "Radio 3");
	}
}
