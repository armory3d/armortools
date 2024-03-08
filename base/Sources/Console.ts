
class Console {

	static message: string = "";
	static message_timer: f32 = 0.0;
	static message_color: i32 = 0x00000000;
	static last_traces: string[] = [""];
	static progress_text: string = null;

	static draw_toast = (s: string) => {
		g2_set_color(0x55000000);
		g2_fill_rect(0, 0, sys_width(), sys_height());
		let scale: f32 = zui_SCALE(base_get_uis()[0]);
		let x: f32 = sys_width() / 2;
		let y: f32 = sys_height() - 200 * scale;
		g2_fill_rect(x - 200 * scale, y, 400 * scale, 80 * scale);
		g2_set_font(base_font);
		g2_set_font_size(Math.floor(22 * scale));
		g2_set_color(0xffffffff);
		g2_draw_string(s, x - g2_font_width(_g2_font, _g2_font_size, s) / 2, y + 40 * scale - g2_font_height(_g2_font, _g2_font_size) / 2);
	}

	static toast = (s: string) => {
		// Show a popup message
		let _render = () => {
			Console.draw_toast(s);
			base_notify_on_next_frame(() => {
				app_remove_render_2d(_render);
			});
		}
		app_notify_on_render_2d(_render);
		Console.console_trace(s);
	}

	static draw_progress = () => {
		Console.draw_toast(Console.progress_text);
	}

	static progress = (s: string) => {
		// Keep popup message displayed until s == null
		if (s == null) {
			app_remove_render_2d(Console.draw_progress);
		}
		else if (Console.progress_text == null) {
			app_notify_on_render_2d(Console.draw_progress);
		}
		if (s != null) Console.console_trace(s);
		Console.progress_text = s;
	}

	static info = (s: string) => {
		Console.message_timer = 5.0;
		Console.message = s;
		Console.message_color = 0x00000000;
		base_redraw_status();
		Console.console_trace(s);
	}

	static error = (s: string) => {
		Console.message_timer = 8.0;
		Console.message = s;
		Console.message_color = 0xffaa0000;
		base_redraw_status();
		Console.console_trace(s);
	}

	static log = (s: string) => {
		Console.console_trace(s);
	}

	static console_trace = (v: any) => {
		base_redraw_console();
		Console.last_traces.unshift(String(v));
		if (Console.last_traces.length > 100) Console.last_traces.pop();
	}
}
