
class SlotFont {
	image: Image = null; // 200px
	previewReady = false;
	id = 0;
	font: Font;
	name: string;
	file: string;

	constructor(name: string, font: Font, file = "") {
		for (let slot of Project.fonts) if (slot.id >= this.id) this.id = slot.id + 1;
		this.name = name;
		this.font = font;
		this.file = file;
	}
}
