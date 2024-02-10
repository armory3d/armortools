
///if (is_paint || is_sculpt)

class ImportFont {

	static run = (path: string) => {
		for (let f of Project.fonts) {
			if (f.file == path) {
				Console.info(Strings.info0());
				return;
			}
		}
		data_get_font(path, (font: g2_font_t) => {
			g2_font_init(font); // Make sure font_ is ready
			let count = krom_g2_font_count(font.font_);
			let fontSlots: SlotFontRaw[] = [];
			for (let i = 0; i < count; ++i) {
				let ar = path.split(Path.sep);
				let name = ar[ar.length - 1];
				let f = g2_font_clone(font);
				g2_font_set_font_index(f, i);
				let fontSlot = SlotFont.create(name, f, path);
				fontSlots.push(fontSlot);
			}

			let _init = () => {
				for (let f of fontSlots) {
					Context.raw.font = f;
					Project.fonts.push(f);
					UtilRender.makeFontPreview();
				}
			}
			app_notify_on_init(_init);

			UIBase.hwnds[TabArea.TabStatus].redraws = 2;
		});
	}
}

///end
