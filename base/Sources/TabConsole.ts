
class TabConsole {

	static draw = (htab: Handle) => {
		let ui = UIBase.ui;

		let title = Console.messageTimer > 0 ? Console.message + "        " : tr("Console");
		let color = Console.messageTimer > 0 ? Console.messageColor : -1;

		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (ui.tab(htab, title, false, color) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			ui.beginSticky();
			///if (krom_windows || krom_linux || krom_darwin) // Copy
			if (Config.raw.touch_ui) {
				ui.row([1 / 4, 1 / 4, 1 / 4]);
			}
			else {
				ui.row([1 / 14, 1 / 14, 1 / 14]);
			}
			///else
			if (Config.raw.touch_ui) {
				ui.row([1 / 4, 1 / 4]);
			}
			else {
				ui.row([1 / 14, 1 / 14]);
			}
			///end

			if (ui.button(tr("Clear"))) {
				Console.lastTraces = [];
			}
			if (ui.button(tr("Export"))) {
				let str = Console.lastTraces.join("\n");
				UIFiles.show("txt", true, false, (path: string) => {
					let f = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".txt")) path += ".txt";
					Krom.fileSaveBytes(path, System.stringToBuffer(str));
				});
			}
			///if (krom_windows || krom_linux || krom_darwin)
			if (ui.button(tr("Copy"))) {
				let str = Console.lastTraces.join("\n");
				Krom.copyToClipboard(str);
			}
			///end

			ui.endSticky();

			let _font = ui.font;
			let _fontSize = ui.fontSize;
			Data.getFont("font_mono.ttf", (f: Font) => { ui.setFont(f); }); // Sync
			ui.fontSize = Math.floor(15 * ui.SCALE());
			for (let t of Console.lastTraces) {
				ui.text(t);
			}
			ui.setFont(_font);
			ui.fontSize = _fontSize;
		}
	}
}
