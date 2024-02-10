
class SlotFontRaw {
	image: image_t = null; // 200px
	previewReady = false;
	id = 0;
	font: g2_font_t;
	name: string;
	file: string;
}

class SlotFont {

	static create(name: string, font: g2_font_t, file = ""): SlotFontRaw {
		let raw = new SlotFontRaw();
		for (let slot of Project.fonts) if (slot.id >= raw.id) raw.id = slot.id + 1;
		raw.name = name;
		raw.font = font;
		raw.file = file;
		return raw;
	}
}
