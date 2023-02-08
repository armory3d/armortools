
let plugin = new arm.Plugin();

let h1 = new zui.Handle();
let h2 = new zui.Handle();
let h3 = new zui.Handle();
let h4 = new zui.Handle();
let h5 = new zui.Handle();
let h6 = new zui.Handle();
let h7 = new zui.Handle();

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "My Plugin")) {
		ui.text("Label");
		ui.textInput(h7, "Text Input");
		if (ui.button("Button")) {
			console.error("Hello");
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
