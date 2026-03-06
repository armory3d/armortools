void console_draw_toast(string_t *s) {
	draw_begin(null, false, 0);
	draw_set_color(0x55000000);
	draw_filled_rect(0, 0, iron_window_width(), iron_window_height());
	f32 scale = UI_SCALE();
	f32 x     = iron_window_width() / (float)2;
	f32 y     = iron_window_height() - 200 * scale;
	draw_filled_rect(x - 200 * scale, y, 400 * scale, 80 * scale);
	draw_set_font(base_font, math_floor(22 * scale));
	draw_set_color(0xffffffff);
	draw_string(s, x - draw_string_width(draw_font, draw_font_size, s) / (float)2, y + 40 * scale - draw_font_height(draw_font, draw_font_size) / (float)2);
	draw_end();
}

void console_toast(string_t *s) {
	// Show a popup message
	gpu_texture_t *current = _draw_current;
	bool           in_use  = gpu_in_use;
	if (in_use)
		draw_end();
	console_trace(s);
	console_draw_toast(s);
	if (in_use)
		draw_begin(current, false, 0);
}

void console_draw_progress(any _) {
	console_draw_toast(console_progress_text);
}

void console_progress(string_t *s) {
	// Keep popup message displayed until s == null
	if (s == null) {
		sys_remove_render(console_draw_progress);
	}
	else if (console_progress_text == null) {
		sys_notify_on_render(console_draw_progress, null);
	}
	if (s != null) {
		console_trace(s);
	}
	gc_unroot(console_progress_text);
	console_progress_text = string_copy(s);
	gc_root(console_progress_text);

	// Pass one frame to immediately show the message
	draw_end();
	sys_render();
	draw_begin(null, false, 0);
	gpu_present();
	ui_end_input();
}

void console_info(string_t *s) {
	console_message_timer = 5.0;
	gc_unroot(console_message);
	console_message = string_copy(s);
	gc_root(console_message);
	console_message_color = 0x00000000;
	base_redraw_status();
	console_trace(s);
}

void console_error(string_t *s) {
	console_message_timer = 8.0;
	gc_unroot(console_message);
	console_message = string_copy(s);
	gc_root(console_message);
	console_message_color = 0xffaa0000;
	base_redraw_status();
	console_trace(s);
}

void console_log(string_t *s) {
	console_trace(s);
}

void console_trace(string_t *s) {
	iron_log(s);
	base_redraw_console();
	array_insert(console_last_traces, 0, s);
	if (console_last_traces->length > 100) {
		array_pop(console_last_traces);
	}
}
