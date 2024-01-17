
class TabScript {

	static hscript = new Handle();
	static textColoring: TTextColoring = null;

	static draw = (htab: Handle) => {
		let ui = UIBase.inst.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (ui.tab(htab, tr("Script")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			ui.beginSticky();
			if (Config.raw.touch_ui) {
				ui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
			}
			else {
				ui.row([1 / 14, 1 / 14, 1 / 14, 1 / 14]);
			}
			if (ui.button(tr("Run"))) {
				try {
					eval(TabScript.hscript.text);
				}
				catch(e: any) {
					Console.log(e);
				}
			}
			if (ui.button(tr("Clear"))) {
				TabScript.hscript.text = "";
			}
			if (ui.button(tr("Import"))) {
				UIFiles.show("js", false, false, (path: string) => {
					Data.getBlob(path, (b: ArrayBuffer) => {
						TabScript.hscript.text = System.bufferToString(b);
						Data.deleteBlob(path);
					});
				});
			}
			if (ui.button(tr("Export"))) {
				let str = TabScript.hscript.text;
				UIFiles.show("js", true, false, (path: string) => {
					let f = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".js")) path += ".js";
					Krom.fileSaveBytes(path, System.stringToBuffer(str));
				});
			}
			ui.endSticky();

			let _font = ui.font;
			let _fontSize = ui.fontSize;
			Data.getFont("font_mono.ttf", (f: Font) => { ui.setFont(f); }); // Sync
			ui.fontSize = Math.floor(15 * ui.SCALE());
			Zui.textAreaLineNumbers = true;
			Zui.textAreaScrollPastEnd = true;
			Zui.textAreaColoring = TabScript.getTextColoring();
			ui.textArea(TabScript.hscript);
			Zui.textAreaLineNumbers = false;
			Zui.textAreaScrollPastEnd = false;
			Zui.textAreaColoring = null;
			ui.setFont(_font);
			ui.fontSize = _fontSize;
		}
	}

	static getTextColoring = (): TTextColoring => {
		if (TabScript.textColoring == null) {
			Data.getBlob("text_coloring.json", (blob: ArrayBuffer) => {
				TabScript.textColoring = JSON.parse(System.bufferToString(blob));
				TabScript.textColoring.default_color = Math.floor(TabScript.textColoring.default_color);
				for (let coloring of TabScript.textColoring.colorings) coloring.color = Math.floor(coloring.color);
			});
		}
		return TabScript.textColoring;
	}
}
