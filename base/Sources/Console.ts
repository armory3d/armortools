
class Console {

	static message = "";
	static messageTimer = 0.0;
	static messageColor = 0x00000000;
	static lastTraces: string[] = [""];
	static progressText: string = null;

	static drawToast = (s: string) => {
		g2_set_color(0x55000000);
		g2_fill_rect(0, 0, sys_width(), sys_height());
		let scale = zui_SCALE(Base.getUIs()[0]);
		let x = sys_width() / 2;
		let y = sys_height() - 200 * scale;
		g2_fill_rect(x - 200 * scale, y, 400 * scale, 80 * scale);
		g2_set_font(Base.font);
		g2_set_font_size(Math.floor(22 * scale));
		g2_set_color(0xffffffff);
		g2_draw_string(s, x - g2_font_width(_g2_font, _g2_font_size, s) / 2, y + 40 * scale - g2_font_height(_g2_font, _g2_font_size) / 2);
	}

	static toast = (s: string) => {
		// Show a popup message
		let _render = () => {
			Console.drawToast(s);
			Base.notifyOnNextFrame(() => {
				app_remove_render_2d(_render);
			});
		}
		app_notify_on_render_2d(_render);
		Console.consoleTrace(s);
	}

	static drawProgress = () => {
		Console.drawToast(Console.progressText);
	}

	static progress = (s: string) => {
		// Keep popup message displayed until s == null
		if (s == null) {
			app_remove_render_2d(Console.drawProgress);
		}
		else if (Console.progressText == null) {
			app_notify_on_render_2d(Console.drawProgress);
		}
		if (s != null) Console.consoleTrace(s);
		Console.progressText = s;
	}

	static info = (s: string) => {
		Console.messageTimer = 5.0;
		Console.message = s;
		Console.messageColor = 0x00000000;
		Base.redrawStatus();
		Console.consoleTrace(s);
	}

	static error = (s: string) => {
		Console.messageTimer = 8.0;
		Console.message = s;
		Console.messageColor = 0xffaa0000;
		Base.redrawStatus();
		Console.consoleTrace(s);
	}

	static log = (s: string) => {
		Console.consoleTrace(s);
	}

	static consoleTrace = (v: any) => {
		Base.redrawConsole();
		Console.lastTraces.unshift(String(v));
		if (Console.lastTraces.length > 100) Console.lastTraces.pop();
	}
}
