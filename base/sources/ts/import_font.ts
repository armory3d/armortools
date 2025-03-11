
function import_font_run(path: string) {
	for (let i: i32 = 0; i < project_fonts.length; ++i) {
		let f: slot_font_t = project_fonts[i];
		if (f.file == path) {
			console_info(strings_asset_already_imported());
			return;
		}
	}

	let font: draw_font_t = data_get_font(path);
	draw_font_init(font); // Make sure font_ is ready
	let count: i32 = draw_font_count(font);
	let font_slots: slot_font_t[] = [];

	for (let i: i32 = 0; i < count; ++i) {
		let ar: string[] = string_split(path, path_sep);
		let name: string = ar[ar.length - 1];
		let f: draw_font_t = { buf: font.buf, index: font.index };
		draw_font_init(f);
		if (!draw_set_font(f, util_render_font_preview_size)) {
			console_error(tr("Error: Failed to read font data"));
			continue;
		}

		let font_slot: slot_font_t = slot_font_create(name, f, path);
		array_push(font_slots, font_slot);
	}

	sys_notify_on_init(function (font_slots: slot_font_t[]) {
		for (let i: i32 = 0; i < font_slots.length; ++i) {
			let f: slot_font_t = font_slots[i];
			context_raw.font = f;
			array_push(project_fonts, f);
			util_render_make_font_preview();
		}
	}, font_slots);

	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
}
