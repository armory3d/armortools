
type slot_font_t = {
	image?: image_t; // 200px
	preview_ready?: bool;
	id?: i32;
	font?: g2_font_t;
	name?: string;
	file?: string;
};

function slot_font_create(name: string, font: g2_font_t, file: string = ""): slot_font_t {
	let raw: slot_font_t = {};
	raw.preview_ready = false;
	raw.id = 0;

	for (let i: i32 = 0; i < project_fonts.length; ++i) {
		let slot: slot_font_t = project_fonts[i];
		if (slot.id >= raw.id) {
			raw.id = slot.id + 1;
		}
	}
	raw.name = name;
	raw.font = font;
	raw.file = file;
	return raw;
}
