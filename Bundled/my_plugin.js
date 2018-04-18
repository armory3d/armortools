
var plugin = new arm.Plugin();

var h1 = plugin.handle();
var h2 = plugin.handle();
var h3 = plugin.handle();
var h4 = plugin.handle();
var h5 = plugin.handle();
var h6 = plugin.handle();
var h7 = plugin.handle();

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "My Plugin")) {
		ui.text("Label");
		ui.textInput(h7, "Text Input");
		if (ui.button("Button")) {
			plugin.log("Hello");
		}
		ui.row([1/2, 1/2]);
		ui.button("Button A");
		ui.button("Button B");
		ui.combo(h5, ["Item 1", "Item 2"], "Combo");
		ui.row([1/2, 1/2]);
		ui.slider(h2, "Slider", 0, 1, true);
		ui.slider(h3, "Slider", 0, 1, true);
		ui.check(h4, "Check");
		ui.radio(h6, 0, "Radio 1");
		ui.radio(h6, 1, "Radio 2");
		ui.radio(h6, 2, "Radio 3");
	}
}
