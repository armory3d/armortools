
let plugin = plugin_create();

let h0 = ui_handle_create();
let h1 = ui_handle_create();
let h2 = ui_handle_create();
let h3 = ui_handle_create();
let h4 = ui_handle_create();
let h5 = ui_handle_create();
let h6 = ui_handle_create();

plugin_notify_on_ui(plugin, function() {
	if (ui_panel(h0, "My Plugin")) {
		ui_text("Label");
		ui_text_input(h1, "Text Input");
		if (ui_button("Button")) {
			console_info("Hello");
		}
		ui_row([1 / 2, 1 / 2]);
		ui_button("Button A");
		ui_button("Button B");
		ui_combo(h2, ["Item 1", "Item 2"], "Combo");
		ui_row([1 / 2, 1 / 2]);
		ui_slider(h3, "Slider", 0, 1, true);
		ui_slider(h4, "Slider", 0, 1, true);
		ui_check(h5, "Check");
		ui_radio(h6, 0, "Radio 1");
		ui_radio(h6, 1, "Radio 2");
		ui_radio(h6, 2, "Radio 3");
	}
});
