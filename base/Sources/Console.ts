
class Console {

	static message = "";
	static messageTimer = 0.0;
	static messageColor = 0x00000000;
	static lastTraces: string[] = [""];
	static progressText: string = null;

	static drawToast = (s: string, g: Graphics2Raw) => {
		g.color = 0x55000000;
		Graphics2.fillRect(0, 0, System.width, System.height);
		let scale = Base.getUIs()[0].SCALE();
		let x = System.width / 2;
		let y = System.height - 200 * scale;
		Graphics2.fillRect(x - 200 * scale, y, 400 * scale, 80 * scale);
		g.font = Base.font;
		g.fontSize = Math.floor(22 * scale);
		g.color = 0xffffffff;
		Graphics2.drawString(s, x - Font.width(g.font, g.fontSize, s) / 2, y + 40 * scale - Font.height(g.font, g.fontSize) / 2);
	}

	static toast = (s: string, g2: Graphics2Raw = null) => {
		// Show a popup message
		let _render = (g: Graphics2Raw) => {
			Console.drawToast(s, g);
			if (g2 == null) {
				Base.notifyOnNextFrame(() => {
					App.removeRender2D(_render);
				});
			}
		}
		g2 != null ? _render(g2) : App.notifyOnRender2D(_render);
		Console.consoleTrace(s);
	}

	static drawProgress = (g: Graphics2Raw) => {
		Console.drawToast(Console.progressText, g);
	}

	static progress = (s: string) => {
		// Keep popup message displayed until s == null
		if (s == null) {
			App.removeRender2D(Console.drawProgress);
		}
		else if (Console.progressText == null) {
			App.notifyOnRender2D(Console.drawProgress);
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
