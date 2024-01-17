
class Operator {

	static ops = new Map<string, any>();

	static register = (name: string, call: any) => {
		Operator.ops.set(name, call);
	}

	static run = (name: string) => {
		if (Operator.ops.get(name) != null) Operator.ops.get(name)();
	}

	static update = () => {
		if (Input.getMouse().startedAny() || Input.getKeyboard().startedAny()) {
			for (let op in Config.keymap) {
				if (Operator.shortcut(Config.keymap[op])) Operator.run(op);
			}
		}
	}

	static shortcut = (s: string, type = ShortcutType.ShortcutStarted): bool => {
		if (s == "") return false;
		let mouse = Input.getMouse();
		let kb = Input.getKeyboard();
		let shift = s.indexOf("shift") >= 0;
		let ctrl = s.indexOf("ctrl") >= 0;
		let alt = s.indexOf("alt") >= 0;
		let flag = shift == kb.down("shift") &&
				   ctrl == kb.down("control") &&
				   alt == kb.down("alt");
		if (s.indexOf("+") > 0) {
			s = s.substr(s.lastIndexOf("+") + 1);
			if (s == "number") return flag;
		}
		else if (shift || ctrl || alt) return flag;
		let key = (s == "left" || s == "right" || s == "middle") ?
			// Mouse
			(type == ShortcutType.ShortcutDown ? mouse.down(s) : mouse.started(s)) :
			// Keyboard
			(type == ShortcutType.ShortcutRepeat ? kb.repeat(s) : type == ShortcutType.ShortcutDown ? kb.down(s) : type == ShortcutType.ShortcutReleased ? kb.released(s) : kb.started(s));
		return flag && key;
	}
}

enum ShortcutType {
	ShortcutStarted,
	ShortcutRepeat,
	ShortcutDown,
	ShortcutReleased,
}
