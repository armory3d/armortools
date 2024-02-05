
class TabScript {

	static hscript = Handle.create();
	static textColoring: TTextColoring = null;

	static draw = (htab: HandleRaw) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (Zui.tab(htab, tr("Script")) && statush > UIStatus.defaultStatusH * Zui.SCALE(ui)) {

			Zui.beginSticky();
			if (Config.raw.touch_ui) {
				Zui.row([1 / 4, 1 / 4, 1 / 4, 1 / 4]);
			}
			else {
				Zui.row([1 / 14, 1 / 14, 1 / 14, 1 / 14]);
			}
			if (Zui.button(tr("Run"))) {
				try {
					eval(TabScript.hscript.text);
				}
				catch(e: any) {
					Console.log(e);
				}
			}
			if (Zui.button(tr("Clear"))) {
				TabScript.hscript.text = "";
			}
			if (Zui.button(tr("Import"))) {
				UIFiles.show("js", false, false, (path: string) => {
					Data.getBlob(path, (b: ArrayBuffer) => {
						TabScript.hscript.text = sys_buffer_to_string(b);
						Data.deleteBlob(path);
					});
				});
			}
			if (Zui.button(tr("Export"))) {
				let str = TabScript.hscript.text;
				UIFiles.show("js", true, false, (path: string) => {
					let f = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".js")) path += ".js";
					Krom.fileSaveBytes(path, sys_string_to_buffer(str));
				});
			}
			Zui.endSticky();

			let _font = ui.font;
			let _fontSize = ui.fontSize;
			Data.getFont("font_mono.ttf", (f: font_t) => { Zui.setFont(ui, f); }); // Sync
			ui.fontSize = Math.floor(15 * Zui.SCALE(ui));
			Zui.textAreaLineNumbers = true;
			Zui.textAreaScrollPastEnd = true;
			Zui.textAreaColoring = TabScript.getTextColoring();
			Zui.textArea(TabScript.hscript);
			Zui.textAreaLineNumbers = false;
			Zui.textAreaScrollPastEnd = false;
			Zui.textAreaColoring = null;
			Zui.setFont(ui, _font);
			ui.fontSize = _fontSize;
		}
	}

	static getTextColoring = (): TTextColoring => {
		if (TabScript.textColoring == null) {
			Data.getBlob("text_coloring.json", (blob: ArrayBuffer) => {
				TabScript.textColoring = JSON.parse(sys_buffer_to_string(blob));
				TabScript.textColoring.default_color = Math.floor(TabScript.textColoring.default_color);
				for (let coloring of TabScript.textColoring.colorings) coloring.color = Math.floor(coloring.color);
			});
		}
		return TabScript.textColoring;
	}
}
