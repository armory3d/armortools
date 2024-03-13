
class SlotFontRaw {
	image: image_t = null; // 200px
	preview_ready: bool = false;
	id: i32 = 0;
	font: g2_font_t;
	name: string;
	file: string;
}

class SlotFont {

	static slot_font_create(name: string, font: g2_font_t, file = ""): SlotFontRaw {
		let raw: SlotFontRaw = new SlotFontRaw();
		for (let slot of project_fonts) if (slot.id >= raw.id) raw.id = slot.id + 1;
		raw.name = name;
		raw.font = font;
		raw.file = file;
		return raw;
	}
}
