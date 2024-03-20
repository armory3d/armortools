
class slot_font_t {
	image: image_t = null; // 200px
	preview_ready: bool = false;
	id: i32 = 0;
	font: g2_font_t;
	name: string;
	file: string;
}

function slot_font_create(name: string, font: g2_font_t, file = ""): slot_font_t {
	let raw: slot_font_t = new slot_font_t();
	for (let slot of project_fonts) {
		if (slot.id >= raw.id) {
			raw.id = slot.id + 1;
		}
	}
	raw.name = name;
	raw.font = font;
	raw.file = file;
	return raw;
}
