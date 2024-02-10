
class TabConsole {

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;

		let title = Console.messageTimer > 0 ? Console.message + "        " : tr("Console");
		let color = Console.messageTimer > 0 ? Console.messageColor : -1;

		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (zui_tab(htab, title, false, color) && statush > UIStatus.defaultStatusH * zui_SCALE(ui)) {

			zui_begin_sticky();
			///if (krom_windows || krom_linux || krom_darwin) // Copy
			if (Config.raw.touch_ui) {
				zui_row([1 / 4, 1 / 4, 1 / 4]);
			}
			else {
				zui_row([1 / 14, 1 / 14, 1 / 14]);
			}
			///else
			if (Config.raw.touch_ui) {
				zui_row([1 / 4, 1 / 4]);
			}
			else {
				zui_row([1 / 14, 1 / 14]);
			}
			///end

			if (zui_button(tr("Clear"))) {
				Console.lastTraces = [];
			}
			if (zui_button(tr("Export"))) {
				let str = Console.lastTraces.join("\n");
				UIFiles.show("txt", true, false, (path: string) => {
					let f = UIFiles.filename;
					if (f == "") f = tr("untitled");
					path = path + Path.sep + f;
					if (!path.endsWith(".txt")) path += ".txt";
					krom_file_save_bytes(path, sys_string_to_buffer(str));
				});
			}
			///if (krom_windows || krom_linux || krom_darwin)
			if (zui_button(tr("Copy"))) {
				let str = Console.lastTraces.join("\n");
				krom_copy_to_clipboard(str);
			}
			///end

			zui_end_sticky();

			let _font = ui.font;
			let _fontSize = ui.font_size;
			data_get_font("font_mono.ttf", (f: g2_font_t) => { zui_set_font(ui, f); }); // Sync
			ui.font_size = Math.floor(15 * zui_SCALE(ui));
			for (let t of Console.lastTraces) {
				zui_text(t);
			}
			zui_set_font(ui, _font);
			ui.font_size = _fontSize;
		}
	}
}
