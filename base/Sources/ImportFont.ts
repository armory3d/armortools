
class ImportFont {

	static run = (path: string) => {
		for (let f of Project.fonts) {
			if (f.file == path) {
				Console.info(Strings.info0());
				return;
			}
		}
		Data.getFont(path, (font: Font) => {
			font.init(); // Make sure font_ is ready
			let count = Krom.g2_font_count(font.font_);
			let fontSlots: SlotFontRaw[] = [];
			for (let i = 0; i < count; ++i) {
				let ar = path.split(Path.sep);
				let name = ar[ar.length - 1];
				let f = font.clone();
				f.setFontIndex(i);
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
			App.notifyOnInit(_init);

			UIBase.hwnds[TabArea.TabStatus].redraws = 2;
		});
	}
}
