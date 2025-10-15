
let console_message: string       = "";
let console_message_timer: f32    = 0.0;
let console_message_color: i32    = 0x00000000;
let console_last_traces: string[] = [ "" ];
let console_progress_text: string = null;

function console_draw_toast(s: string) {
	draw_begin();
	draw_set_color(0x55000000);
	draw_filled_rect(0, 0, iron_window_width(), iron_window_height());
	let scale: f32 = UI_SCALE();
	let x: f32     = iron_window_width() / 2;
	let y: f32     = iron_window_height() - 200 * scale;
	draw_filled_rect(x - 200 * scale, y, 400 * scale, 80 * scale);
	draw_set_font(base_font, math_floor(22 * scale));
	draw_set_color(0xffffffff);
	draw_string(s, x - draw_string_width(draw_font, draw_font_size, s) / 2, y + 40 * scale - draw_font_height(draw_font, draw_font_size) / 2);
	draw_end();
}

function console_toast(s: string) {
	// Show a popup message
	let current: gpu_texture_t = _draw_current;
	if (current != null)
		draw_end();
	console_trace(s);
	console_draw_toast(s);
	if (current != null)
		draw_begin(current);
}

function console_draw_progress() {
	console_draw_toast(console_progress_text);
}

function console_progress(s: string) {
	// Keep popup message displayed until s == null
	if (s == null) {
		sys_remove_render(console_draw_progress);
	}
	else if (console_progress_text == null) {
		sys_notify_on_render(console_draw_progress);
	}
	if (s != null) {
		console_trace(s);
	}
	console_progress_text = s;

	// Pass one frame to immediately show the message
	draw_end();
	sys_render();
	draw_begin();
	gpu_present();
	ui_end_input();
}

function console_info(s: string) {
	console_message_timer = 5.0;
	console_message       = s;
	console_message_color = 0x00000000;
	base_redraw_status();
	console_trace(s);
}

function console_error(s: string) {
	console_message_timer = 8.0;
	console_message       = s;
	console_message_color = 0xffaa0000;
	base_redraw_status();
	console_trace(s);
}

function console_log(s: string) {
	console_trace(s);
}

function console_trace(s: string) {
	iron_log(s);
	base_redraw_console();
	array_insert(console_last_traces, 0, s);
	if (console_last_traces.length > 100) {
		array_pop(console_last_traces);
	}
}
