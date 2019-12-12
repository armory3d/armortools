
let plugin = new arm.Plugin();
let hpanel = new zui.Handle();
let hlist = new zui.Handle();

function stringToBytes(str) {
	let ab = new ArrayBuffer(str.length);
	let u8 = new Uint8Array(ab);
	for (let i = 0; i < str.length; ++i) {
		u8[i] = str.charCodeAt(i);
	}
	return ab;
}

plugin.drawUI = function(ui) {
	if (ui.panel(hpanel, "Theme Editor")) {
		ui.row([1 / 4]);
		if (ui.button("Export")) {
			arm.UIFiles.show("json", true, function(path) {
				path += arm.Path.sep + arm.UIFiles.filename;
				if (!path.endsWith(".json")) path += ".json";
				Krom.fileSaveBytes(path, stringToBytes(JSON.stringify(arm.App.theme)));
			});
		}

		let i = 0;
		let theme = arm.App.theme;
		for (let key in theme) {
			let h = hlist.nest(i);
			let val = theme[key];
			let isHex = key.endsWith("_COL");
			if (isHex && val < 0) val += 4294967295;

			if (isHex) {
				ui.row([1 / 8, 7 / 8]);
				ui.text("", 0, val);
				if (ui.isHovered && ui.inputReleased) {
					arm.UIMenu.draw(function(ui) {
						ui.fill(0, 0, ui._w / ui.ops.scaleFactor, ui.t.ELEMENT_H * 6, ui.t.SEPARATOR_COL);
						ui.changed = false;
						h.color = theme[key];
						theme[key] = zui.Ext.colorWheel(ui, h, false, null, false, false);
						if (ui.changed) arm.UIMenu.keepOpen = true;
					});
				}
			}

			h.text = isHex ? val.toString(16) : val.toString();
			let res = ui.textInput(h, key);
			if (res === "true") theme[key] = true;
			else if (res === "false") theme[key] = false;
			else if (isHex) theme[key] = parseInt(h.text, 16);
			else theme[key] = parseInt(h.text);
			i++;
		}
	}
}
