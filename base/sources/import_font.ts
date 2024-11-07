
function import_font_run(path: string) {
	for (let i: i32 = 0; i < project_fonts.length; ++i) {
		let f: slot_font_t = project_fonts[i];
		if (f.file == path) {
			console_info(strings_info0());
			return;
		}
	}
	let font: g2_font_t = data_get_font(path);
	g2_font_init(font); // Make sure font_ is ready
	let count: i32 = iron_g2_font_count(font.font_);
	let font_slots: slot_font_t[] = [];
	for (let i: i32 = 0; i < count; ++i) {
		let ar: string[] = string_split(path, path_sep);
		let name: string = ar[ar.length - 1];
		let f: g2_font_t = g2_font_clone(font);
		g2_font_set_font_index(f, i);
		let font_slot: slot_font_t = slot_font_create(name, f, path);
		array_push(font_slots, font_slot);
	}

	app_notify_on_init(function (font_slots: slot_font_t[]) {
		for (let i: i32 = 0; i < font_slots.length; ++i) {
			let f: slot_font_t = font_slots[i];
			context_raw.font = f;
			array_push(project_fonts, f);
			util_render_make_font_preview();
		}
	}, font_slots);

	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
}
