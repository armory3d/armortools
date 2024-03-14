
///if (is_paint || is_sculpt)

function import_font_run(path: string) {
	for (let f of project_fonts) {
		if (f.file == path) {
			console_info(strings_info0());
			return;
		}
	}
	let font: g2_font_t = data_get_font(path);
	g2_font_init(font); // Make sure font_ is ready
	let count: i32 = krom_g2_font_count(font.font_);
	let font_slots: SlotFontRaw[] = [];
	for (let i: i32 = 0; i < count; ++i) {
		let ar: string[] = path.split(path_sep);
		let name: string = ar[ar.length - 1];
		let f: g2_font_t = g2_font_clone(font);
		g2_font_set_font_index(f, i);
		let font_slot: SlotFontRaw = slot_font_create(name, f, path);
		font_slots.push(font_slot);
	}

	let _init = function () {
		for (let f of font_slots) {
			context_raw.font = f;
			project_fonts.push(f);
			util_render_make_font_preview();
		}
	}
	app_notify_on_init(_init);

	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
}

///end
