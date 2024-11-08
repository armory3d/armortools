
let console_message: string = "";
let console_message_timer: f32 = 0.0;
let console_message_color: i32 = 0x00000000;
let console_last_traces: string[] = [""];
let console_progress_text: string = null;

function console_draw_toast(s: string) {
	g2_set_color(0x55000000);
	g2_fill_rect(0, 0, sys_width(), sys_height());
	let scale: f32 = ui_SCALE(base_get_uis()[0]);
	let x: f32 = sys_width() / 2;
	let y: f32 = sys_height() - 200 * scale;
	g2_fill_rect(x - 200 * scale, y, 400 * scale, 80 * scale);
	g2_set_font(base_font);
	g2_set_font_size(math_floor(22 * scale));
	g2_set_color(0xffffffff);
	g2_draw_string(s, x - g2_font_width(_g2_font, _g2_font_size, s) / 2, y + 40 * scale - g2_font_height(_g2_font, _g2_font_size) / 2);
}

function _console_toast_render(s: string) {
	console_draw_toast(s);
	iron_g4_swap_buffers();
	app_remove_render_2d(_console_toast_render);
}

function console_toast(s: string) {
	// Show a popup message
	app_notify_on_render_2d(_console_toast_render, s);
	console_trace(s);
	iron_g4_swap_buffers();
}

function console_draw_progress() {
	console_draw_toast(console_progress_text);
}

function console_progress(s: string) {
	// Keep popup message displayed until s == null
	if (s == null) {
		app_remove_render_2d(console_draw_progress);
	}
	else if (console_progress_text == null) {
		app_notify_on_render_2d(console_draw_progress);
	}
	if (s != null) {
		console_trace(s);
	}
	console_progress_text = s;

	// Pass one frame to immediately show the message
	ui_end_input();
	g2_end();
	app_render();
	g2_begin();
	iron_g4_swap_buffers();
}

function console_info(s: string) {
	console_message_timer = 5.0;
	console_message = s;
	console_message_color = 0x00000000;
	base_redraw_status();
	console_trace(s);
}

function console_error(s: string) {
	console_message_timer = 8.0;
	console_message = s;
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
