
let plugin = new arm.Plugin();

let h1 = new zui.Handle();
let h2 = new zui.Handle();
let h3 = new zui.Handle();
let h4 = new zui.Handle();
let h5 = new zui.Handle();
let h6 = new zui.Handle();
let h7 = new zui.Handle();

plugin.drawUI = function(ui) {
	if (Zui.panel(h1, "My Plugin")) {
		Zui.text("Label");
		Zui.textInput(h7, "Text Input");
		if (Zui.button("Button")) {
			console.error("Hello");
		}
		Zui.row([1/2, 1/2]);
		Zui.button("Button A");
		Zui.button("Button B");
		Zui.combo(h5, ["Item 1", "Item 2"], "Combo");
		Zui.row([1/2, 1/2]);
		Zui.slider(h2, "Slider", 0, 1, true);
		Zui.slider(h3, "Slider", 0, 1, true);
		Zui.check(h4, "Check");
		Zui.radio(h6, 0, "Radio 1");
		Zui.radio(h6, 1, "Radio 2");
		Zui.radio(h6, 2, "Radio 3");
	}
}
