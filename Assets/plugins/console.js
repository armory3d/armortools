
let plugin = new arm.Plugin();

let h1 = new zui.Handle();
let h2 = new zui.Handle();

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "Console")) {
		ui.indent();
		ui.row([8/10, 2/10]);
		var t = ui.textInput(h2);
		if (ui.button("Run")) {
			try { arm.Log.trace("> " + t); eval(t); }
			catch(e) { arm.Log.trace(e); }
		}
		for (const t of arm.Log.lastTraces) ui.text(t);
		ui.unindent();
	}
}
