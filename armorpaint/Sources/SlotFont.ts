
class SlotFontRaw {
	image: Image = null; // 200px
	previewReady = false;
	id = 0;
	font: Font;
	name: string;
	file: string;
}

class SlotFont {

	static create(name: string, font: Font, file = ""): SlotFontRaw {
		let raw = new SlotFontRaw();
		for (let slot of Project.fonts) if (slot.id >= raw.id) raw.id = slot.id + 1;
		raw.name = name;
		raw.font = font;
		raw.file = file;
		return raw;
	}
}
