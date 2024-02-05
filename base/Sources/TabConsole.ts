
class TabConsole {

	static draw = (htab: HandleRaw) => {
		let ui = UIBase.ui;

		let title = Console.messageTimer > 0 ? Console.message + "        " : tr("Console");
		let color = Console.messageTimer > 0 ? Console.messageColor : -1;

		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (Zui.tab(htab, title, false, color) && statush > UIStatus.defaultStatusH * Zui.SCALE(ui)) {

			Zui.beginSticky();
			///if (krom_windows || krom_linux || krom_darwin) // Copy
			if (Config.raw.touch_ui) {
				Zui.row([1 / 4, 1 / 4, 1 / 4]);
			}
			else {
				Zui.row([1 / 14, 1 / 14, 1 / 14]);
			}
			///else
			if (Config.raw.touch_ui) {
				Zui.row([1 / 4, 1 / 4]);
			}
			else {
				Zui.row([1 / 14, 1 / 14]);
			}
			///end

			if (Zui.button(tr("Clear"))) {
				Console.lastTraces = [];
			}
			if (Zui.button(tr("Export"))) {
				let str = Console.lastTraces.join("\n");
				UIFiles.show("txt", true, false, (path: string) => {
					let f = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".txt")) path += ".txt";
					Krom.fileSaveBytes(path, sys_string_to_buffer(str));
				});
			}
			///if (krom_windows || krom_linux || krom_darwin)
			if (Zui.button(tr("Copy"))) {
				let str = Console.lastTraces.join("\n");
				Krom.copyToClipboard(str);
			}
			///end

			Zui.endSticky();

			let _font = ui.font;
			let _fontSize = ui.fontSize;
			Data.getFont("font_mono.ttf", (f: font_t) => { Zui.setFont(ui, f); }); // Sync
			ui.fontSize = Math.floor(15 * Zui.SCALE(ui));
			for (let t of Console.lastTraces) {
				Zui.text(t);
			}
			Zui.setFont(ui, _font);
			ui.fontSize = _fontSize;
		}
	}
}
